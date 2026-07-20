#!/usr/bin/env python3
"""№113: convert the source game's lighting tables into this engine's format.

The two engines store environment lighting as the same four-table chain, but the
record layouts diverged between them:

    source          this engine     meaning
    EnvR  0x08      Env0  0x41      per-room -> palette-set ids
    Colo  0x0C      Col0  0x0C      palette-set -> 6 palettes (IDENTICAL layout)
    Pale  0x2C      PAL0  0x34      the actual ambient/fog colours
    Virt  0x24      VRB0  0x18      sky / cloud / haze colours

Two findings drive this converter:

  1. The structures line up almost 1:1. A native outdoor stage (F_SP103) carries
     18 palettes in 3 groups of 6 with bloom ids cycling 4..9 - which is exactly
     the shape the donor room's chain has. So this is a field-mapping job, not a
     redesign.

  2. This engine's PAL0 has SIX trailing fields the donor format has no source
     for (0x2C..0x33). Left zeroed they are not neutral - `unk_0x2f` is 200 and
     `bloom_tbl_id` is 4..9 on every native outdoor stage, and a zero there is a
     different bloom table entirely. That is the blown-out sky: the geometry was
     right, the exposure fields were empty. Defaults below are sampled from
     native outdoor stages, not invented.

Everything is written mod-side only. No donor bytes enter the repo.
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

import build_fdl_host_stg as bf
import grow_rdl01_stg as g
from adapt_bdl_arcs import be32, yaz0_dec

MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm"
    / "Dusklight"
    / "model_replacements"
    / "WW-Crew-Restoration"
)
SRC_STAGE = Path("D:/XXXXXXX/Ex WW/files/res/Stage")

# --- native-sampled defaults for the fields the donor format cannot supply ----
# F_SP00 / F_SP102 / F_SP103 / F_SP108 / F_SP121 all agree on these outdoors.
BG_LIGHT_INFLUENCE = 100
CLOUD_SHADOW_DENSITY = 45
UNK_2F_EXTERIOR = 200   # native outdoor stages
UNK_2F_INTERIOR = 0     # native indoor stage (R_SP01) uses 0
BG_AMB_ALPHA = (255, 255, 255)


def rarc_members(d: bytearray):
    data_abs = 0x20 + be32(d, 0x0C)
    info = 0x20
    n = be32(d, info + 8)
    ent = info + be32(d, info + 0x0C)
    strs = info + be32(d, info + 0x14)
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", d, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", d, e + 6)[0]
        name = bytes(d[strs + no : d.index(b"\0", strs + no)]).decode("ascii", "replace")
        off, size = be32(d, e + 8), be32(d, e + 12)
        yield name, bytes(d[data_abs + off : data_abs + off + size])


def load_src_dzs(stage: str) -> bytes:
    path = SRC_STAGE / stage / "Stage.arc"
    if not path.is_file():
        raise SystemExit(f"missing donor stage: {path}")
    d = bytearray(path.read_bytes())
    if bytes(d[:4]) == b"Yaz0":
        d = yaz0_dec(d)
    for name, member in rarc_members(d):
        if name.lower().endswith(".dzs"):
            return member
    raise SystemExit(f"{stage}: no .dzs member")


def dzs_tables(dzs: bytes) -> dict[str, tuple[int, int]]:
    out: dict[str, tuple[int, int]] = {}
    for i in range(be32(dzs, 0)):
        o = 4 + i * 12
        out[bytes(dzs[o : o + 4]).decode("ascii", "replace")] = (
            be32(dzs, o + 4),
            be32(dzs, o + 8),
        )
    return out


class SrcLighting:
    """One donor stage's four lighting tables."""

    def __init__(self, stage: str):
        self.stage = stage
        self.dzs = load_src_dzs(stage)
        self.t = dzs_tables(self.dzs)
        for tag in ("EnvR", "Colo", "Pale", "Virt"):
            if tag not in self.t:
                raise SystemExit(f"{stage}: no {tag} table (cannot convert)")

    def _rec(self, tag: str, i: int, size: int) -> bytes:
        cnt, off = self.t[tag]
        if i >= cnt:
            raise SystemExit(f"{self.stage}: {tag}[{i}] out of range (n={cnt})")
        return self.dzs[off + i * size : off + (i + 1) * size]

    def envr(self, room: int) -> list[int]:
        return list(self._rec("EnvR", room, 0x8))

    def colo(self, i: int) -> tuple[list[int], float]:
        r = self._rec("Colo", i, 0xC)
        return list(r[:8]), struct.unpack_from(">f", r, 8)[0]

    def pale(self, i: int) -> bytes:
        return self._rec("Pale", i, 0x2C)

    def virt(self, i: int) -> bytes:
        return self._rec("Virt", i, 0x24)


def conv_vrb0(v: bytes) -> bytes:
    """Virt(0x24) -> VRB0(0x18).

    Donor packs cloud colours as GXColor and the three horizon bands as RGB.
    This engine wants sky/kumo_top/kumo_bottom as RGB then three GXColors.
    There is no donor source for kumo_shadow, so it is derived from the cloud
    colour at 60% - a shadow that is a darker version of its own cloud is the
    only defensible reading, and it keeps the alpha the donor authored.
    """
    kumo = v[0x10:0x14]          # GXColor
    kumo_center = v[0x14:0x18]   # GXColor
    sky = v[0x18:0x1B]
    uso_umi = v[0x1B:0x1E]
    kasumi_mae = v[0x1E:0x21]

    shadow = bytes([(c * 3) // 5 for c in kumo[:3]]) + bytes([kumo[3]])
    out = bytearray()
    out += sky                                   # 0x00 sky_col
    out += kumo[:3]                              # 0x03 kumo_top_col
    out += kumo_center[:3]                       # 0x06 kumo_bottom_col
    out += shadow                                # 0x09 kumo_shadow_col
    out += uso_umi + b"\xff"                     # 0x0D kasumi_outer_col
    out += kasumi_mae + b"\xff"                  # 0x11 kasumi_inner_col
    out += b"\x00" * 3                           # 0x15 pad to 0x18
    assert len(out) == 0x18
    return bytes(out)


def conv_pal0(p: bytes, vrbcol_id: int, band: int, exterior: bool) -> bytes:
    """Pale(0x2C) -> PAL0(0x34)."""
    actor_c0 = p[0x00:0x03]
    actor_k0 = p[0x03:0x06]
    bg0_c0 = p[0x06:0x09]
    bg1_c0 = p[0x0C:0x0F]
    bg2_c0 = p[0x12:0x15]
    bg3_c0 = p[0x18:0x1B]
    fog = p[0x1E:0x21]
    fog_z = p[0x24:0x2C]   # start/end floats sit at the same offset in both

    out = bytearray()
    out += actor_c0                              # 0x00 actor_amb_col
    out += bg0_c0 + bg1_c0 + bg2_c0 + bg3_c0     # 0x03 bg_amb_col[4]
    # No donor source for the 6 point-light colours. The donor's actor konstant
    # is the scene key-light tint, so it is the tonally correct neutral: it can
    # never blow out, and it only applies if a LGHT entry exists at all.
    out += actor_k0 * 6                          # 0x0F plight_col[6]
    out += fog                                   # 0x21 fog_col
    out += fog_z                                 # 0x24 fog_start_z / fog_end_z
    out.append(vrbcol_id)                        # 0x2C vrboxcol_id
    out.append(BG_LIGHT_INFLUENCE)               # 0x2D
    out.append(CLOUD_SHADOW_DENSITY)             # 0x2E
    out.append(UNK_2F_EXTERIOR if exterior else UNK_2F_INTERIOR)
    out.append(4 + (band % 6))                   # 0x30 bloom_tbl_id (native idiom)
    out += bytes(BG_AMB_ALPHA)                   # 0x31 BG1/2/3_amb_alpha
    assert len(out) == 0x34
    return bytes(out)


def build_tables(room_map: list[tuple[int, str, int]], exterior: bool):
    """room_map: [(our_room, donor_stage, donor_room)] -> (env0, col0, pal0, vrb0)."""
    stages: dict[str, SrcLighting] = {}
    col0_ids: dict[tuple[str, int], int] = {}
    pal0_ids: dict[tuple[str, int], int] = {}
    vrb0_ids: dict[tuple[str, int], int] = {}
    col0_recs: list[bytes] = []
    pal0_recs: list[bytes] = []
    vrb0_recs: list[bytes] = []
    env0_by_room: dict[int, bytes] = {}
    report: list[str] = []

    def get(stage: str) -> SrcLighting:
        if stage not in stages:
            stages[stage] = SrcLighting(stage)
        return stages[stage]

    def intern_virt(stage: str, vid: int) -> int:
        key = (stage, vid)
        if key not in vrb0_ids:
            vrb0_ids[key] = len(vrb0_recs)
            vrb0_recs.append(conv_vrb0(get(stage).virt(vid)))
        return vrb0_ids[key]

    def intern_pale(stage: str, pid: int, band: int) -> int:
        key = (stage, pid)
        if key not in pal0_ids:
            p = get(stage).pale(pid)
            vid = intern_virt(stage, p[0x21])
            pal0_ids[key] = len(pal0_recs)
            pal0_recs.append(conv_pal0(p, vid, band, exterior))
        return pal0_ids[key]

    def intern_colo(stage: str, cid: int) -> int:
        key = (stage, cid)
        if key not in col0_ids:
            pal_ids, rate = get(stage).colo(cid)
            new = [intern_pale(stage, p, band) for band, p in enumerate(pal_ids[:6])]
            # Native Col0 parks the two unused slots at 255, not 0.
            rec = bytes(new) + bytes([255, 255]) + struct.pack(">f", rate)
            assert len(rec) == 0xC
            col0_ids[key] = len(col0_recs)
            col0_recs.append(rec)
        return col0_ids[key]

    for our_room, stage, donor_room in room_map:
        src_envr = get(stage).envr(donor_room)
        mapped = [intern_colo(stage, c) for c in src_envr]
        # This engine's Env0 is 65 bytes. Only indices 0..7 have a known meaning
        # (pat0..pat7 in the kankyo colour path). The remaining slots are filled
        # with the donor's DEFAULT set rather than zero, so that whatever index
        # the engine reaches it lands on an authored, in-range palette set.
        default = mapped[0]
        env0 = bytes(mapped[:8]) + bytes([default]) * (0x41 - 8)
        assert len(env0) == 0x41
        env0_by_room[our_room] = env0
        report.append(
            f"  room {our_room:<3} <- {stage}[{donor_room}]  sets={src_envr} -> {mapped}"
        )

    max_room = max(r for r, _s, _d in room_map)
    env0_blob = b"".join(
        env0_by_room.get(i, env0_by_room[room_map[0][0]]) for i in range(max_room + 1)
    )
    return (
        (max_room + 1, env0_blob),
        (len(col0_recs), b"".join(col0_recs)),
        (len(pal0_recs), b"".join(pal0_recs)),
        (len(vrb0_recs), b"".join(vrb0_recs)),
        report,
    )


def read_rtbl_active(dzs: bytes) -> set[int]:
    """Recover which rooms an existing RTBL marks live (ChkBg bit 0x80)."""
    t = dzs_tables(dzs)
    if "RTBL" not in t:
        return set()
    cnt, off = t["RTBL"]
    active: set[int] = set()
    for i in range(cnt):
        ptr = be32(dzs, off + i * 4)
        if ptr + 8 > len(dzs):
            continue
        rooms_off = be32(dzs, ptr + 4)
        if rooms_off < len(dzs) and dzs[rooms_off] & 0x80:
            active.add(i)
    return active


def inject(stage_path: Path, tables: dict[str, tuple[int, bytes]]) -> None:
    """Replace/insert the four lighting tables in a stage.dzs, keep the rest."""
    # Always convert from the pristine pre-conversion stage, never from our own
    # output: re-running otherwise reads back an already-rewritten (and possibly
    # broken) RTBL. This makes the pass idempotent.
    bak = stage_path.with_suffix(stage_path.suffix + ".pre113-bak")
    raw = bak.read_bytes() if bak.is_file() else stage_path.read_bytes()
    members = dict(g.list_rarc_files(raw))
    if "stage.dzs" not in members:
        raise SystemExit(f"{stage_path}: no stage.dzs")
    chunks = g.parse_dzs_chunks(members["stage.dzs"])

    kept = [(tag, ent, payload) for tag, ent, payload in chunks if tag not in tables]
    # Drop the legacy vrbox table too: it shares VRB0's role and a stale copy
    # would describe a sky we are replacing.
    kept = [c for c in kept if c[0] != "Virt"]
    # Drop layer-variant lighting tables inherited from the template stage.
    # The env layer is chosen by Elst (m_layerTable[storyLayer] -> env digit);
    # with no Elst chunk the decoder pins layer 0, so these are unreachable now -
    # but they still hold the template's own colours, and they would silently
    # take over the moment an Elst is added. Remove rather than leave armed.
    dropped = [c[0] for c in kept if c[0][:3] in ("Env", "Col", "PAL", "VRB")]
    kept = [c for c in kept if c[0][:3] not in ("Env", "Col", "PAL", "VRB")]
    if dropped:
        print(f"  dropped stale layer tables: {dropped}")
    new_chunks = kept + [(tag, cnt, blob) for tag, (cnt, blob) in tables.items()]

    data_start = 4 + len(new_chunks) * 12
    cursor = data_start
    meta = []
    for tag, ent, payload in new_chunks:
        while cursor % 4:
            cursor += 1
        meta.append((tag, ent, cursor, payload))
        cursor += len(payload)

    # RTBL stores ABSOLUTE offsets into the dzs. Adding/removing any chunk
    # resizes the header and slides RTBL, which silently invalidates every
    # pointer inside it - the table still parses, it just points at the old
    # layout. Regenerate it for wherever it landed.
    rtbl_active = read_rtbl_active(members["stage.dzs"])
    out = bytearray()
    out += g.be32(len(meta))
    for tag, ent, off, _p in meta:
        out += tag.encode("ascii") + g.be32(ent) + g.be32(off)
    for tag, ent, off, payload in meta:
        while len(out) < off:
            out.append(0)
        if tag == "RTBL":
            payload = bf.build_rtbl_sparse(ent, off, rtbl_active)
        out += payload
    if any(tag == "RTBL" for tag, _e, _o, _p in meta):
        rtbl_n = next(e for tag, e, _o, _p in meta if tag == "RTBL")
        g.assert_rtbl_pointers(bytes(out), rtbl_n)
        print(f"  RTBL rebuilt for new offset, active rooms={sorted(rtbl_active)}")

    files = [(n, b) for n, b in g.list_rarc_files(raw) if n != "stage.dzs"]
    files.append(("stage.dzs", bytes(out)))
    packed = g.pack_rarc(files)

    if not bak.is_file():
        bak.write_bytes(raw)
        print(f"  backup -> {bak.name}")
    stage_path.write_bytes(packed)


def do_stage(name: str, room_map, exterior: bool) -> None:
    stage_path = MOD / "files" / "res" / "Stage" / name / "STG_00.arc"
    if not stage_path.is_file():
        print(f"SKIP {name}: no STG_00.arc yet")
        return
    env0, col0, pal0, vrb0, report = build_tables(room_map, exterior)
    print(f"{name}  ({'exterior' if exterior else 'interior'})")
    for line in report:
        print(line)
    print(
        f"  Env0 n={env0[0]}  Col0 n={col0[0]}  PAL0 n={pal0[0]}  VRB0 n={vrb0[0]}"
    )
    inject(
        stage_path,
        {"Env0": env0, "Col0": col0, "PAL0": pal0, "VRB0": vrb0},
    )
    print(f"  wrote {stage_path}")


def main() -> int:
    # Outset exterior: donor sea sector, room 44.
    do_stage("F_DL01", [(44, "sea", 44)], exterior=True)
    # Forest: its own donor stage, room 0.
    do_stage("F_DL02", [(0, "A_mori", 0)], exterior=True)
    # Interiors: each donor house is its own stage; our rooms 0..5 (№105 ids).
    do_stage(
        "R_DL01",
        [
            (0, "LinkRM", 0),
            (1, "LinkUG", 0),
            (2, "Ojhous2", 1),
            (3, "Ojhous2", 0),
            (4, "Omasao", 0),
            (5, "Onobuta", 0),
        ],
        exterior=False,
    )
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
