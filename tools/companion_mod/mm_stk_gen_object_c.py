#!/usr/bin/env python3
# ============================================================================
# MM object_stk -> ZAPD-style C (for Fast64 custom skeleton import)
# ============================================================================
# Replaces the broken custom F3DEX2 parser path (see
# docs/MM-SkullKid-SideBySide-HANDOFF.md). Three root causes fixed here:
#   1. WRONG FILE: old extractor read gObjectTable at a raw ROM offset that
#      landed inside dmadata -> pulled VROM 0x1820000 (the file AFTER
#      object_stk). The real object_stk is found by structural fingerprint
#      (limb table at 0x131D8 must have small joint positions and segment-6
#      or NULL DL pointers).
#   2. BAD YAZ0: old yaz0_decompress ignored the extended-length encoding
#      (length nibble == 0 -> third byte, length = b + 0x12), desyncing the
#      output stream.
#   3. WRONG MICROCODE: hand-rolled parser used legacy F3D field layouts on
#      F3DEX2 data. Here libgfxd (pygfxd) does ALL Gfx decoding.
#
# Output: a single self-contained object_stk_gen.c containing u64 texture
# arrays, Vtx arrays, Gfx display lists, StandardLimb structs, the limb
# table, and the FlexSkeletonHeader -- formatted to match the regexes in
# tools/vendor/fast64/fast64_internal/z64/skeleton/utility.py and
# fast64_internal/f3d/f3d_parser.py (parseVertexData / parseTextureData).
#
# Usage:
#   python tools/companion_mod/mm_stk_gen_object_c.py
#   python tools/companion_mod/mm_stk_gen_object_c.py --no-head-hack
# ============================================================================
from __future__ import annotations

import argparse
import struct
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

from pygfxd import (
    GfxdEndian,
    gfxd_buffer_to_string,
    gfxd_dl_callback,
    gfxd_endian,
    gfxd_execute,
    gfxd_f3dex2,
    gfxd_input_buffer,
    gfxd_macro_dflt,
    gfxd_macro_fn,
    gfxd_output_buffer,
    gfxd_printf,
    gfxd_puts,
    gfxd_target,
    gfxd_timg_callback,
    gfxd_vtx_callback,
)

# ============================================================================
# Paths / constants
# ============================================================================
ROM = Path(
    r"D:\Extractuibs\Extractions 6.5\MM64 shiz\Zelda64Recompiled-v1.2.2-Windows"
    r"\roms\Legend of Zelda, The - Majora's Mask (USA).z64"
)
WORK = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk"
)
XML = WORK / "object_stk.xml"
OUT_DIR = WORK / "gen_c"

DMADATA_OFF = 0x001A500  # MM US 1.0
LIMB_TABLE_PROBE = 0x131D8  # gSkullKidRootLimb offset (fingerprint anchor)
LIMB_PROBE_COUNT = 17

# ============================================================================
# Visual limb-DL overrides (disable all with --faithful)
# ============================================================================
# - Head/eyes are drawn by DmStk_PostLimbDraw in code, not by the head limb
#   DL; wire them onto the head limb via a synthesized wrapper DL.
# - The left hand limb binds gSkullKidLeftHandAndFluteDL in ROM; swap in the
#   flute-less open hand for the reskin side-by-side.
HEAD_LIMB_NAME = "gSkullKidHeadLimb"
HEAD_DL = "gSkullKidNormalHeadDL"
EYES_DL = "gSkullKidNormalEyesDL"
HEAD_WRAPPER = "gSkullKidHeadWithEyesDL"
VISUAL_LIMB_DL_OVERRIDES = {
    HEAD_LIMB_NAME: HEAD_WRAPPER,
    "gSkullKidLeftHandLimb": "gSkullKidOpenLeftHandDL",
}


# ============================================================================
# ROM extraction (correct Yaz0 + fingerprint file identification)
# ============================================================================
def yaz0_decompress(data: bytes) -> bytes:
    if data[:4] != b"Yaz0":
        return data
    dec_size = struct.unpack(">I", data[4:8])[0]
    out = bytearray(dec_size)
    src, dst = 16, 0
    code, bits = 0, 0
    while dst < dec_size:
        if bits == 0:
            code = data[src]
            src += 1
            bits = 8
        if code & 0x80:
            out[dst] = data[src]
            dst += 1
            src += 1
        else:
            b1, b2 = data[src], data[src + 1]
            src += 2
            dist = ((b1 & 0xF) << 8) | b2
            n = b1 >> 4
            if n == 0:
                length = data[src] + 0x12
                src += 1
            else:
                length = n + 2
            offset = dst - dist - 1
            for i in range(length):
                if dst >= dec_size:
                    break
                out[dst] = out[offset + i]
                dst += 1
        code <<= 1
        bits -= 1
    return bytes(out)


def parse_dmadata(rom: bytes, table_off: int) -> list[dict]:
    entries = []
    off = table_off
    while off + 16 <= len(rom):
        v0, v1, p0, p1 = struct.unpack_from(">IIII", rom, off)
        if v0 == 0 and v1 == 0 and p0 == 0 and p1 == 0:
            break
        entries.append({"v0": v0, "v1": v1, "p0": p0, "p1": p1})
        off += 16
    return entries


def limb_fingerprint_score(blob: bytes, base: int, count: int) -> int:
    if len(blob) < base + count * 12:
        return -1
    score = 0
    for i in range(count):
        raw = blob[base + i * 12 : base + i * 12 + 12]
        x, y, z, child, sib = struct.unpack_from(">hhhBB", raw)
        dl = struct.unpack_from(">I", raw, 8)[0]
        score += all(abs(v) < 5000 for v in (x, y, z))
        score += (child < 30 or child == 255) and (sib < 30 or sib == 255)
        score += dl == 0 or (dl >> 24) == 6
    return score


def extract_object_stk(rom: bytes) -> tuple[bytes, int]:
    best_score, best_blob, best_vrom = -1, b"", 0
    for e in parse_dmadata(rom, DMADATA_OFF):
        if e["p0"] == 0xFFFFFFFF:
            continue
        size = e["v1"] - e["v0"]
        if abs(size - 0x1D530) > 0x4000:
            continue
        end = e["p1"] if e["p1"] else e["p0"] + size
        try:
            blob = yaz0_decompress(rom[e["p0"] : end])
        except (IndexError, struct.error):
            continue
        s = limb_fingerprint_score(blob, LIMB_TABLE_PROBE, LIMB_PROBE_COUNT)
        if s > best_score:
            best_score, best_blob, best_vrom = s, blob, e["v0"]
    if best_score < LIMB_PROBE_COUNT * 3:
        raise SystemExit(
            f"object_stk fingerprint failed (best {best_score}/{LIMB_PROBE_COUNT * 3} "
            f"@ vrom {best_vrom:#x}) -- wrong ROM?"
        )
    print(f"object_stk: vrom {best_vrom:#x}, {len(best_blob):#x} bytes, "
          f"fingerprint {best_score}/{LIMB_PROBE_COUNT * 3}")
    return best_blob, best_vrom


# ============================================================================
# XML symbol tables
# ============================================================================
class Symbols:
    def __init__(self, xml_path: Path):
        root = ET.parse(xml_path).getroot()
        file_node = root.find("File")
        self.dlists: dict[int, str] = {}
        self.textures: dict[int, dict] = {}
        self.limbs: dict[int, str] = {}
        self.animations: dict[int, str] = {}
        self.skeleton_off: int | None = None
        self.skeleton_name = ""
        for node in file_node:
            off_attr = node.get("Offset")
            if off_attr is None:
                continue
            off = int(off_attr, 16)
            if node.tag == "DList":
                self.dlists[off] = node.get("Name")
            elif node.tag == "Texture":
                w, h = int(node.get("Width")), int(node.get("Height"))
                fmt = node.get("Format")
                bpp = {"rgba32": 4, "rgba16": 2, "ia16": 2,
                       "ia8": 1, "i8": 1, "ci8": 1}.get(fmt)
                if bpp is None:
                    raise SystemExit(f"unsupported texture format {fmt} @ {off:#x}")
                self.textures[off] = {"name": node.get("Name"), "w": w, "h": h,
                                      "size": w * h * bpp}
            elif node.tag == "Limb":
                self.limbs[off] = node.get("Name")
            elif node.tag == "Animation":
                self.animations[off] = node.get("Name")
            elif node.tag == "Skeleton":
                self.skeleton_off = off
                self.skeleton_name = node.get("Name")

    def texture_for_addr(self, off: int) -> str | None:
        for t_off, t in self.textures.items():
            if t_off <= off < t_off + t["size"]:
                if off != t_off:
                    return None  # mid-texture reference: unexpected
                return t["name"]
        return None


# ============================================================================
# DL scanning helpers
# ============================================================================
def dl_slice(blob: bytes, start: int) -> bytes:
    """Slice one display list: from start through its ENDDL (0xDF) or a
    tail-branch gsSPBranchList (0xDE with flag byte 1)."""
    off = start
    while off + 8 <= len(blob):
        op = blob[off]
        if op == 0xDF:
            return blob[start : off + 8]
        if op == 0xDE and blob[off + 1] == 1:
            return blob[start : off + 8]
        off += 8
    raise ValueError(f"no DL terminator after {start:#x}")


class Collector:
    """Pass 1: record vtx load ranges and segment-6 branch targets."""

    def __init__(self):
        self.vtx_ranges: list[tuple[int, int]] = []  # (start, end)
        self.branches: set[int] = set()
        self.timgs: set[int] = set()

    def vtx_cb(self, vtx: int, num: int) -> int:
        if vtx >> 24 == 6:
            start = vtx & 0xFFFFFF
            self.vtx_ranges.append((start, start + num * 16))
        return 0

    def timg_cb(self, timg, fmt, size, width, height, pal) -> int:
        if timg >> 24 == 6:
            self.timgs.add(timg & 0xFFFFFF)
        return 0

    def dl_cb(self, dl: int) -> int:
        if dl >> 24 == 6:
            self.branches.add(dl & 0xFFFFFF)
        return 0


def merge_ranges(ranges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    merged: list[list[int]] = []
    for s, e in sorted(ranges):
        if merged and s <= merged[-1][1]:
            merged[-1][1] = max(merged[-1][1], e)
        else:
            merged.append([s, e])
    return [(s, e) for s, e in merged]


# ============================================================================
# C emission
# ============================================================================
def emit_texture(name: str, blob: bytes, off: int, size: int) -> str:
    words = struct.unpack_from(f">{size // 8}Q", blob, off)
    lines = [f"u64 {name}[] = {{"]
    for i in range(0, len(words), 4):
        row = ", ".join(f"0x{w:016X}" for w in words[i : i + 4])
        lines.append(f"    {row},")
    lines.append("};\n")
    return "\n".join(lines)


def emit_vtx_array(name: str, blob: bytes, start: int, end: int) -> str:
    lines = [f"Vtx {name}[] = {{"]
    for off in range(start, end, 16):
        x, y, z, flag, s, t = struct.unpack_from(">hhhHhh", blob, off)
        r, g, b, a = struct.unpack_from(">BBBB", blob, off + 12)
        lines.append(
            f"    {{{{ {{ {x}, {y}, {z} }}, {flag}, {{ {s}, {t} }}, "
            f"{{ {r}, {g}, {b}, {a} }} }}}},"
        )
    lines.append("};\n")
    return "\n".join(lines)


def emit_animation(name: str, blob: bytes, off: int, limb_count: int) -> str:
    """AnimationHeader { { frameCount }, frameData, jointIndices, staticIndexMax }.
    frameData / jointIndices sizes are derived from the joint index table:
    static tracks read frameData[idx], dynamic tracks read frameData[idx+frame]."""
    frame_count = struct.unpack_from(">h", blob, off)[0]
    frame_ptr, joint_ptr = struct.unpack_from(">II", blob, off + 4)
    static_max = struct.unpack_from(">H", blob, off + 12)[0]
    if frame_ptr >> 24 != 6 or joint_ptr >> 24 != 6:
        raise SystemExit(f"anim {name}: non-segment-6 pointers")
    frame_off = frame_ptr & 0xFFFFFF
    joint_off = joint_ptr & 0xFFFFFF

    n_joints = limb_count + 1  # root translation + one rotation per limb
    joints = [
        struct.unpack_from(">HHH", blob, joint_off + i * 6) for i in range(n_joints)
    ]
    n_frames_data = 0
    for jx in joints:
        for idx in jx:
            need = idx + 1 if idx < static_max else idx + frame_count
            n_frames_data = max(n_frames_data, need)
    frames = struct.unpack_from(f">{n_frames_data}h", blob, frame_off)

    lines = [f"s16 {name}FrameData[] = {{"]
    for i in range(0, len(frames), 12):
        row = ", ".join(f"0x{v & 0xFFFF:04X}" for v in frames[i : i + 12])
        lines.append(f"    {row},")
    lines.append("};\n")
    lines.append(f"JointIndex {name}JointIndices[] = {{")
    for jx in joints:
        lines.append(f"    {{ {jx[0]}, {jx[1]}, {jx[2]} }},")
    lines.append("};\n")
    lines.append(
        f"AnimationHeader {name} = {{\n"
        f"    {{ {frame_count} }}, {name}FrameData, {name}JointIndices, {static_max}\n"
        f"}};\n"
    )
    return "\n".join(lines)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--rom", type=Path, default=ROM)
    ap.add_argument("--out", type=Path, default=OUT_DIR)
    ap.add_argument("--faithful", action="store_true",
                    help="No visual overrides: head limb NULL (drawn in code), "
                         "left hand keeps the flute DL")
    args = ap.parse_args()

    rom = args.rom.read_bytes()
    blob, vrom = extract_object_stk(rom)
    syms = Symbols(XML)

    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / "object_stk.zobj").write_bytes(blob)

    # ------------------------------------------------------------------
    # Pass 1: collect vtx ranges / branch targets / texture refs per DL,
    # growing the DL set until closed under gsSPDisplayList branches.
    # ------------------------------------------------------------------
    dl_names: dict[int, str] = dict(syms.dlists)
    collector = Collector()
    gfxd_target(gfxd_f3dex2)
    gfxd_endian(GfxdEndian.big, 4)
    gfxd_vtx_callback(collector.vtx_cb)
    gfxd_timg_callback(collector.timg_cb)
    gfxd_dl_callback(collector.dl_cb)
    gfxd_macro_fn(gfxd_macro_dflt)

    scanned: set[int] = set()
    queue = sorted(dl_names)
    while queue:
        off = queue.pop()
        if off in scanned:
            continue
        scanned.add(off)
        data = dl_slice(blob, off)
        gfxd_input_buffer(data)
        gfxd_output_buffer(bytes(len(data) * 96))
        if gfxd_execute() != 0:
            raise SystemExit(f"gfxd pass1 failed on DL @ {off:#x} ({dl_names.get(off)})")
        for b in sorted(collector.branches):
            if b not in dl_names:
                dl_names[b] = f"gSkullKidDL_{b:06X}"
            if b not in scanned:
                queue.append(b)
        collector.branches.clear()

    # Unknown texture refs are a hard error -- no hand-wavy hint maps.
    unknown_timgs = [t for t in collector.timgs if syms.texture_for_addr(t) is None]
    if unknown_timgs:
        raise SystemExit("SETTIMG targets not in XML: "
                         + ", ".join(hex(t) for t in sorted(unknown_timgs)))

    vtx_arrays = merge_ranges(collector.vtx_ranges)

    def vtx_symbol(addr: int) -> str | None:
        off = addr & 0xFFFFFF
        for s, e in vtx_arrays:
            if s <= off < e:
                name = f"gSkullKidVtx_{s:06X}"
                idx = (off - s) // 16
                return f"&{name}[{idx}]" if idx else name
        return None

    # ------------------------------------------------------------------
    # Pass 2: emit each DL with symbol substitution.
    # ------------------------------------------------------------------
    def vtx_cb2(vtx: int, num: int) -> int:
        sym = vtx_symbol(vtx)
        if sym and vtx >> 24 == 6:
            gfxd_printf(sym)
            return 1
        return 0

    def timg_cb2(timg, fmt, size, width, height, pal) -> int:
        if timg >> 24 == 6:
            name = syms.texture_for_addr(timg & 0xFFFFFF)
            if name:
                gfxd_printf(name)
                return 1
        return 0

    def dl_cb2(dl: int) -> int:
        if dl >> 24 == 6:
            gfxd_printf(dl_names[dl & 0xFFFFFF])
            return 1
        return 0

    def macro_fn2() -> int:
        gfxd_puts("    ")
        gfxd_macro_dflt()
        gfxd_puts(",\n")
        return 0

    gfxd_vtx_callback(vtx_cb2)
    gfxd_timg_callback(timg_cb2)
    gfxd_dl_callback(dl_cb2)
    gfxd_macro_fn(macro_fn2)

    dl_texts: dict[int, str] = {}
    for off in sorted(dl_names):
        data = dl_slice(blob, off)
        gfxd_input_buffer(data)
        outb = gfxd_output_buffer(bytes(len(data) * 96))
        if gfxd_execute() != 0:
            raise SystemExit(f"gfxd pass2 failed on DL @ {off:#x} ({dl_names[off]})")
        body = gfxd_buffer_to_string(outb)
        dl_texts[off] = f"Gfx {dl_names[off]}[] = {{\n{body}}};\n"

    # ------------------------------------------------------------------
    # Skeleton: FlexSkeletonHeader { { limbTable, limbCount }, dListCount }
    # ------------------------------------------------------------------
    tbl_ptr, limb_count, dlist_count = struct.unpack_from(
        ">IB3xB", blob, syms.skeleton_off
    )
    if tbl_ptr >> 24 != 6:
        raise SystemExit(f"skeleton limb table ptr not segment 6: {tbl_ptr:#x}")
    tbl_off = tbl_ptr & 0xFFFFFF

    limb_entries = []
    for i in range(limb_count):
        ptr = struct.unpack_from(">I", blob, tbl_off + i * 4)[0]
        l_off = ptr & 0xFFFFFF
        if l_off not in syms.limbs:
            raise SystemExit(f"limb table entry {i} -> {ptr:#x} not in XML")
        limb_entries.append(syms.limbs[l_off])

    limb_structs = []
    for l_off, l_name in sorted(syms.limbs.items()):
        x, y, z, child, sib = struct.unpack_from(">hhhBB", blob, l_off)
        dl = struct.unpack_from(">I", blob, l_off + 8)[0]
        if dl == 0:
            dl_sym = "NULL"
        else:
            dl_sym = dl_names.get(dl & 0xFFFFFF)
            if dl_sym is None:
                raise SystemExit(f"limb {l_name} DL {dl:#x} not disassembled")
        if not args.faithful and l_name in VISUAL_LIMB_DL_OVERRIDES:
            dl_sym = VISUAL_LIMB_DL_OVERRIDES[l_name]
        limb_structs.append(
            f"StandardLimb {l_name} = {{\n"
            f"    {{ {x}, {y}, {z} }}, 0x{child:02X}, 0x{sib:02X},\n"
            f"    {dl_sym}\n"
            f"}};\n"
        )

    # ------------------------------------------------------------------
    # Assemble the C file
    # ------------------------------------------------------------------
    parts: list[str] = [
        "// ============================================================================\n"
        f"// object_stk (MM US, VROM {vrom:#x}) -- generated by mm_stk_gen_object_c.py\n"
        "// libgfxd (pygfxd) disassembly; layout matches Fast64 z64 import regexes.\n"
        "// ============================================================================\n"
    ]
    for t_off, t in sorted(syms.textures.items()):
        parts.append(emit_texture(t["name"], blob, t_off, t["size"]))
    for s, e in vtx_arrays:
        parts.append(emit_vtx_array(f"gSkullKidVtx_{s:06X}", blob, s, e))
    for off in sorted(dl_texts):
        parts.append(dl_texts[off])

    if not args.faithful:
        parts.append(
            "// Head + eyes are drawn by DmStk_PostLimbDraw in code; wire them to\n"
            "// the head limb for Blender import (side-by-side step 1 only).\n"
            f"Gfx {HEAD_WRAPPER}[] = {{\n"
            f"    gsSPDisplayList({EYES_DL}),\n"
            f"    gsSPBranchList({HEAD_DL}),\n"
            "};\n"
        )

    for a_off, a_name in sorted(syms.animations.items()):
        parts.append(emit_animation(a_name, blob, a_off, limb_count))

    parts.extend(limb_structs)
    limb_refs = ",\n    ".join(f"&{n}" for n in limb_entries)
    parts.append(
        f"void* gSkullKidSkelLimbs[{limb_count}] = {{\n    {limb_refs},\n}};\n"
    )
    parts.append(
        f"FlexSkeletonHeader {syms.skeleton_name} = {{\n"
        f"    {{ gSkullKidSkelLimbs, {limb_count} }}, {dlist_count}\n"
        f"}};\n"
    )

    out_c = args.out / "object_stk_gen.c"
    out_c.write_text("\n".join(parts), encoding="utf-8", newline="\n")

    print(f"limbs: {limb_count} (dlist count {dlist_count})")
    print(f"DLs: {len(dl_texts)} ({len(dl_names) - len(syms.dlists)} auto-named)")
    print(f"Vtx arrays: {len(vtx_arrays)} "
          f"({sum((e - s) // 16 for s, e in vtx_arrays)} verts)")
    print(f"textures: {len(syms.textures)}")
    print(f"wrote {out_c}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
