"""Adapt WW-era BDL4 arcs for the TP port's battle-tested BMD3 loader.

The port's loadBinaryDisplayList (BDL path) is untested decomp code (TP content
never exercises it) and crashes on WW models. The Beta Link precedent ("adapted
remount, NOT raw extracts") applies: convert offline, in the MOD FOLDER only.

Conversion per .bdl member, done IN-PLACE inside the RARC (member slots keep
their offsets; only the entry's dataSize shrinks, tail slack zeroed):
  - remove the MDL3 section (precompiled GX display lists, GC-only)
  - magic 'J3D2bdl4' -> 'J3D2bmd3', sectionCount -= 1, fileSize adjusted
BCK/BTP/BTI members are format-shared with TP and pass through untouched.

Usage: python adapt_bdl_arcs.py <src.arc|src_dir> <dst.arc|dst_dir>
"""
# ============================================================================
# TIER-3 QUARANTINE (kit audit + OUTPUT LAW wiring). This tool writes donor-
# format bytes and belongs to the era the project is leaving. It must NOT run
# in the disc-native porting wave. Kept RE-RUNNABLE per the No116 ceremony:
# pass --tier3-override; the run is then LEDGERED so no donor-format write
# happens silently. IMPORT-SAFE: gates only under __main__, so Tier-1 kits
# that import helpers from this file are unaffected.
# ============================================================================
import sys as _q_sys
if __name__ == "__main__":
    if "--tier3-override" not in _q_sys.argv:
        _q_sys.stderr.write(
            "TIER-3 QUARANTINED (kit audit): retired from the disc-native wave. "
            "Deliberate rerun: --tier3-override (ledgered).\n")
        raise SystemExit(3)
    _q_sys.argv.remove("--tier3-override")
    import io as _q_io
    from pathlib import Path as _q_P
    _q_led = _q_P(__file__).resolve().parents[2] / "docs" / "state" / "ww-staging" / "OUTPUT-LEDGER.md"
    with _q_io.open(_q_led, "a", encoding="utf-8", newline="\r\n") as _q_f:
        _q_f.write("| TIER3-OVERRIDE RUN: %s | deliberate rerun | quarantine stands |\n"
                   % _q_P(__file__).name)
import os
import struct
import sys


def be32(d, o):
    return struct.unpack_from(">I", d, o)[0]


def yaz0_dec(d):
    size = be32(d, 4)
    out = bytearray(size)
    si, di = 16, 0
    code, bits = 0, 0
    while di < size:
        if bits == 0:
            code = d[si]; si += 1; bits = 8
        if code & 0x80:
            out[di] = d[si]; si += 1; di += 1
        else:
            b1, b2 = d[si], d[si + 1]; si += 2
            dist = ((b1 & 0xF) << 8) | b2
            src = di - (dist + 1)
            n = b1 >> 4
            if n == 0:
                n = d[si] + 0x12
                si += 1
            else:
                n += 2
            for _ in range(n):
                out[di] = out[src]; di += 1; src += 1
                if di >= size:
                    break
        code <<= 1; bits -= 1
    return out


def normalize_litmask(buf):
    """WW lit channels use litMask 0x03 (light slots 0+1); TP's actor light path
    only reliably populates slot 0 (proven by the working WW-boots overlay, mask
    0x01) — slot 1 garbage blacks the channel. Clamp every ENABLED channel's mask
    to 0x01. Unlit channels (eyes etc.) untouched."""
    off = 0x20
    seccnt = be32(buf, 12)
    for _ in range(seccnt):
        tag = bytes(buf[off:off + 4])
        ssize = be32(buf, off + 4)
        if tag == b"MAT3":
            base = off
            offs = [struct.unpack_from(">i", buf, base + 0x0C + i * 4)[0] for i in range(30)]
            pool = offs[7]
            nxt = min((o for o in offs if o > pool), default=ssize)
            patched = 0
            for e in range(pool, nxt - 7, 8):
                enable, litmask = buf[base + e], buf[base + e + 2]
                if enable == 1 and litmask not in (0x00, 0x01):
                    buf[base + e + 2] = 0x01
                    patched += 1
            return patched
        off += ssize
    return 0


def flatten_toon(buf):
    """WW cel-shading: body TEV multiplies texture by a toon ramp (ZAtoon /
    ZBtoonEX, I4 8x8) sampled by light level — an unlit channel samples the
    ramp's BLACK end, annihilating the texture. Flatten the ramp to white so
    the multiplier is 1.0 (flat-lit WW look) until channel lighting lands."""
    off = 0x20
    seccnt = be32(buf, 12)
    for _ in range(seccnt):
        tag = bytes(buf[off:off + 4])
        ssize = be32(buf, off + 4)
        if tag == b"TEX1":
            base = off
            cnt = struct.unpack_from(">H", buf, base + 8)[0]
            hdrs = base + be32(buf, base + 0xC)
            nametab = base + be32(buf, base + 0x10)
            n = struct.unpack_from(">H", buf, nametab)[0]
            names = []
            for i in range(n):
                so = struct.unpack_from(">H", buf, nametab + 4 + i * 4 + 2)[0]
                end = buf.index(b"\0", nametab + so)
                names.append(buf[nametab + so:end].decode("ascii", "replace"))
            patched = 0
            for i in range(cnt):
                if i >= len(names) or not names[i].lower().startswith(("zatoon", "zbtoon")):
                    continue
                h = hdrs + i * 0x20
                w, ht = struct.unpack_from(">HH", buf, h + 2)
                dataoff = be32(buf, h + 0x1C)
                size = max(32, (w * ht) // 2)  # I4 = 4bpp
                buf[h + dataoff:h + dataoff + size] = b"\xFF" * size
                patched += 1
            return patched
        off += ssize
    return 0


def normalize_tevregs(buf):
    """WW's runtime overwrites material TEV register colors per frame (C-regs =
    live light/shadow colors); the file ships a 50%-gray placeholder
    (128,128,128,255) that our mount never replaces -> everything renders at
    exactly half brightness, immune to ambient. Promote the gray placeholder to
    full white (lit-side multiplier = 1.0)."""
    off = 0x20
    seccnt = be32(buf, 12)
    for _ in range(seccnt):
        tag = bytes(buf[off:off + 4])
        ssize = be32(buf, off + 4)
        if tag == b"MAT3":
            base = off
            offs = [struct.unpack_from(">i", buf, base + 0x0C + i * 4)[0] for i in range(30)]
            o = offs[17]  # tevColor S10 (s16 RGBA)
            if o <= 0:
                return 0
            nxt = min((x for x in offs if x > o), default=ssize)
            patched = 0
            for e in range(base + o, base + nxt - 7, 8):
                r, g, bl, a = struct.unpack_from(">hhhh", buf, e)
                if (r, g, bl) == (128, 128, 128):
                    struct.pack_into(">hhhh", buf, e, 255, 255, 255, a)
                    patched += 1
            return patched
        off += ssize
    return 0


def adapt_bdl(buf, skip_tevregs=False):
    """buf = bytearray of one .bdl file. Returns new size, or None if not bdl4.

    skip_tevregs: leave TEV colour registers alone (sky domes - see adapt_arc).
    """
    if bytes(buf[:8]) != b"J3D2bdl4":
        return None
    seccnt = be32(buf, 12)
    off = 0x20
    mdl_off = mdl_size = None
    for _ in range(seccnt):
        tag = bytes(buf[off:off + 4])
        ssize = be32(buf, off + 4)
        if tag == b"MDL3":
            mdl_off, mdl_size = off, ssize
            break
        off += ssize
    if mdl_off is None:
        # bdl4 without MDL3 — just retag
        buf[0:8] = b"J3D2bmd3"
        return len(buf)
    tail = buf[mdl_off + mdl_size:]
    new_size = len(buf) - mdl_size
    buf[mdl_off:mdl_off + len(tail)] = tail
    buf[new_size:] = b"\x00" * (len(buf) - new_size)
    buf[0:8] = b"J3D2bmd3"
    struct.pack_into(">I", buf, 8, new_size)
    struct.pack_into(">I", buf, 12, seccnt - 1)
    normalize_litmask(buf)
    if not skip_tevregs:
        normalize_tevregs(buf)
    if "--flat-toon" in sys.argv:
        flatten_toon(buf)
    return new_size


def adapt_dzb(buf):
    """№21 verdict: WW dzb poly-property word0 carries 0x7FF across bits 16-26;
    TP reads bits 14-23 as through-flags (obj/cam/LINK/arrow/stick/boomerang/
    rope/bomb) -> every WW triangle is pass-through under TP semantics (P2 miss,
    P3 hit). Clear the TP through cluster; keep material/sound bits."""
    if len(buf) < 48:
        return 0
    w = struct.unpack_from(">12I", buf, 0)
    ti_n, ti_o = w[10], w[11]
    if ti_n <= 0 or ti_o + ti_n * 16 > len(buf):
        return 0
    patched = 0
    for i in range(ti_n):
        e = ti_o + i * 16
        w0 = struct.unpack_from(">I", buf, e)[0]
        nw0 = w0 & ~0x00FFC000
        if nw0 != w0:
            struct.pack_into(">I", buf, e, nw0)
            patched += 1
    return patched


def adapt_arc(src, dst):
    d = bytearray(open(src, "rb").read())
    if bytes(d[:4]) == b"Yaz0":
        d = yaz0_dec(d)  # output is written plain; the mount loader accepts both
    if bytes(d[:4]) != b"RARC":
        sys.exit(f"{src}: not a RARC")
    data_abs = 0x20 + be32(d, 0x0C)
    info = 0x20
    n = be32(d, info + 0x08)
    ent = info + be32(d, info + 0x0C)
    strs = info + be32(d, info + 0x14)
    converted = []
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", d, e + 4)[0] & 0x1100 != 0x1100:
            continue
        name_off = struct.unpack_from(">H", d, e + 6)[0]
        end = d.index(b"\0", strs + name_off)
        name = d[strs + name_off:end].decode("ascii", "replace")
        off, size = be32(d, e + 8), be32(d, e + 12)
        lname = name.lower()
        if lname.endswith(".dzb"):
            member = bytearray(d[data_abs + off:data_abs + off + size])
            n = adapt_dzb(member)
            if n:
                d[data_abs + off:data_abs + off + size] = member
                converted.append(f"{name}: {n} ti through-cluster cleared")
            continue
        if not lname.endswith(".bdl"):
            continue
        member = bytearray(d[data_abs + off:data_abs + off + size])
        # Sky-dome models are the one case where normalize_tevregs is WRONG:
        # for vr_* the TEV register IS the sky colour, not a lighting
        # multiplier, so promoting it to white paints the dome pure white.
        # Their colours are authored by bake_wwsky_colors.py (see №116).
        new_size = adapt_bdl(member, skip_tevregs=lname.startswith("vr_"))
        if new_size is None:
            continue
        d[data_abs + off:data_abs + off + size] = member
        struct.pack_into(">I", d, e + 12, new_size)
        converted.append(f"{name}: {size} -> {new_size} (-{size - new_size})")
    open(dst, "wb").write(d)
    print(f"{os.path.basename(src)} -> {dst}")
    for c in converted:
        print(f"   {c}")
    if not converted:
        print("   (no bdl4 members found)")


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    src, dst = sys.argv[1], sys.argv[2]
    if os.path.isdir(src):
        os.makedirs(dst, exist_ok=True)
        for fn in sorted(os.listdir(src)):
            if fn.lower().endswith(".arc"):
                adapt_arc(os.path.join(src, fn), os.path.join(dst, fn))
    else:
        adapt_arc(src, dst)


if __name__ == "__main__":
    main()
