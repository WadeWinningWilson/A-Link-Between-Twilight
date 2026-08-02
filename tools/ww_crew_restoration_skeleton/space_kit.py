#!/usr/bin/env python3
# ============================================================================
# space_kit.py — the Space Kit driver (build order §3.1 + Pass-3 battery)
#
# Implements, per docs/WW Linked/space-kit-design.md (§323-refined, §325-trimmed):
#   * §0  teardown-archetype classifier (Pass-1 first act):
#         A (exit-ful)  = any staff in the event's closure carries Stage+StartCode
#         B (exit-less) = none does
#         (bus §323.1; decider is mechanical, from staff DATA)
#   * Pass-1 field decoders:
#         PLYR 0x20/entry: byte 0x08 = REVT event index (0xff = none — the native
#           cutscene auto-trigger), byte 0x1D = spawn id (low byte of angle.z),
#           floats +0x0C = pos, s16 +0x18/1A/1C = angles (plyr_append.py layout)
#         REVT 0x1C/entry: type byte +0x0, map_id +0x4, exit bytes +0x7/+0x9,
#           name +0xD, switch +0x1B (revt_bake.py layout + §321 exit semantics)
#         STB DATA_ID channels: paragraph type 0x81, content u32[0] = channel;
#           9 = BLACK FADE / 10 = WHITE FADE (§322 corrected semantics)
#   * Pass-3 regression asserts (the §323 failure catalog, mechanized):
#         fade-pairing, REVT-0xFF, B-closure(WAIT), teardown-spawn, StartCode
#         resolution (incl. the §322b id-alias 0→0xCB)
#   * Pass-4 manifest (JSON) — what the space carries + code deps it relies on.
#
# MODES:
#   inventory <StageDirName>   Pass-1 on a DONOR stage (reads Ex WW tree)
#   regress                    Pass-3 battery on the LIVE R_DL01 host — the
#                              pilot: the tale is CLOSED end-to-end on these
#                              arcs, so every assert must PASS; a failure is a
#                              kit bug (or a real latent gap — report either).
#
# Read-only in both modes. Never writes game data. №31-clean.
# ============================================================================
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from jstudio_stb import yaz0_dec, rarc_members
from merge_event import EventFile, arc_member, ESZ, SSZ, CSZ, DSZ

DONOR_ROOT = Path("D:/XXXXXXX/Ex WW/files/res/Stage")
MOD = Path("C:/Users/xxxxx/AppData/Roaming/TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration")
HOST_STG = MOD / "files/res/Stage/R_DL01/STG_00.arc"
HOST_ROOM0 = MOD / "files/res/Stage/R_DL01/R00_00.arc"
HOST_DEMO_ARC = MOD / "arcs/Demo01.arc"

# §322b alias (port-side code the kit VERIFIES, never bakes — §325 trim):
# donor StartCode -> host spawn id, per stage alias in d_event_data.cpp
STARTCODE_ALIAS = {"LinkRM": {0: 0xCB}}

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]
bef32 = lambda d, o: struct.unpack_from(">f", d, o)[0]


def get_member(arc_path, suffix):
    raw = Path(arc_path).read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    for nm, blob in rarc_members(raw):
        if nm.lower().endswith(suffix.lower()):
            if blob[:4] == b"Yaz0":
                blob = yaz0_dec(blob)
            return nm, blob
    return None, None


def member_names(arc_path):
    raw = Path(arc_path).read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    return [nm for nm, _ in rarc_members(raw)]


def dz_chunks(dz):
    n = be32(dz, 0)
    out = {}
    for i in range(n):
        tag = dz[4 + i * 12:8 + i * 12].decode("ascii", "replace")
        cnt, off = struct.unpack_from(">II", dz, 8 + i * 12)
        out.setdefault(tag, []).append((cnt, off))
    return out


# ---------------------------------------------------------------------------
# event_list closure + §0 archetype classifier
# ---------------------------------------------------------------------------
def rec_name(raw, top, idx, size):
    n = 8 if size == SSZ else 32
    return raw[top + idx * size:top + idx * size + n].split(b"\0")[0].decode(
        "ascii", "replace")


def event_closure(ef, ev_idx):
    """Names + records of every staff/cut/data in the event's closure."""
    ev = ef.rec(ef.etop, ev_idx, ESZ)
    nstaff = struct.unpack_from(">i", ev, 0x7C)[0]
    staffs, cuts, datas = [], [], []
    for k in range(nstaff):
        s = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        if not (0 <= s < ef.snum):
            continue
        staffs.append(s)
        st = ef.rec(ef.stop, s, SSZ)
        c = struct.unpack_from(">i", st, 0x30)[0]
        guard = 0
        while 0 <= c < ef.cnum and guard < 512:
            guard += 1
            cuts.append(c)
            cut = ef.rec(ef.ctop, c, CSZ)
            d = struct.unpack_from(">i", cut, 0x38)[0]
            dg = 0
            while 0 <= d < ef.dnum and dg < 512:
                dg += 1
                datas.append(d)
                d = struct.unpack_from(">i", ef.rec(ef.dtop, d, DSZ), 0x30)[0]
            c = struct.unpack_from(">i", cut, 0x3C)[0]
    return staffs, cuts, datas


def data_value(ef, d_idx):
    """Best-effort payload read: (name, type, values)."""
    r = ef.rec(ef.dtop, d_idx, DSZ)
    nm = bytes(r[:32]).split(b"\0")[0].decode("ascii", "replace")
    ty, di, num, _ = struct.unpack_from(">iiii", r, 0x24)
    # donor dEvDtData_c::DataType (d_event_data.h:26):
    # 0=FLOAT, 1=VEC, 3=INT, 4=STRING
    vals = None
    try:
        if ty == 0:
            vals = list(struct.unpack_from(">%df" % max(num, 1), ef.raw,
                                           ef.ftop + di * 4))
        elif ty == 1:
            vals = list(struct.unpack_from(">%df" % (3 * max(num, 1)), ef.raw,
                                           ef.ftop + di * 4))
        elif ty == 3:
            vals = list(struct.unpack_from(">%di" % max(num, 1), ef.raw,
                                           ef.itop + di * 4))
        elif ty == 4:
            end = ef.raw.index(b"\0", ef.sdtop + di)
            vals = ef.raw[ef.sdtop + di:end].decode("ascii", "replace")
    except Exception:
        vals = "<decode-miss raw=%s>" % r[0x24:0x34].hex()
    return nm, ty, vals


def classify_events(ef):
    """§0: archetype per event, from staff data. Returns per-event dict."""
    out = {}
    for i in range(ef.enum):
        name = rec_name(ef.raw, ef.etop, i, ESZ)
        staffs, cuts, datas = event_closure(ef, i)
        dnames = {}
        for d in datas:
            nm, ty, vals = data_value(ef, d)
            dnames.setdefault(nm, []).append(vals)
        has_stage = "Stage" in dnames
        has_start = "StartCode" in dnames
        arche = "A(exit-ful)" if (has_stage and has_start) else "B(exit-less)"
        out[name] = {
            "index": i,
            "archetype": arche,
            "staff_names": [rec_name(ef.raw, ef.stop, s, SSZ) for s in staffs],
            "cut_names": sorted({rec_name(ef.raw, ef.ctop, c, CSZ) for c in cuts}),
            "Stage": dnames.get("Stage"),
            "StartCode": dnames.get("StartCode"),
        }
    return out


# ---------------------------------------------------------------------------
# PLYR / REVT / STB decoders
# ---------------------------------------------------------------------------
def decode_plyr(dzr):
    pts = []
    for cnt, off in dz_chunks(dzr).get("PLYR", []):
        for k in range(cnt):
            e = off + k * 0x20
            pts.append({
                "name": dzr[e:e + 8].split(b"\0")[0].decode("ascii", "replace"),
                "event_byte": dzr[e + 0x08],
                "pos": [round(bef32(dzr, e + 0x0C + 4 * i), 1) for i in range(3)],
                "angle_y": struct.unpack_from(">h", dzr, e + 0x1A)[0],
                "spawn_id": dzr[e + 0x1D],
            })
    return pts


def decode_revt(dzs):
    evs = []
    for cnt, off in dz_chunks(dzs).get("REVT", []):
        for k in range(cnt):
            e = off + k * 0x1C
            evs.append({
                "type": dzs[e],
                "map_id": dzs[e + 4],
                "exit_finish": dzs[e + 7],
                "exit_skip": dzs[e + 9],
                "name": dzs[e + 0xD:e + 0xD + 13].split(b"\0")[0].decode(
                    "ascii", "replace"),
                "switch": dzs[e + 0x1B],
            })
    return evs


def stb_fade_channels(stb):
    """Scan every object block's paragraphs for DATA_ID (type 0x81) channels."""
    channels = set()
    nblocks = be32(stb, 0x0C)
    off = 0x20
    for _ in range(nblocks):
        bsize, btype = struct.unpack_from(">II", stb, off)
        idsize = be16(stb, off + 0xA)
        seq = off + 0xC + ((idsize + 3) & ~3)
        p = seq
        end = off + bsize
        while p < end:
            head = be32(stb, p)
            typ, param = head >> 24, head & 0xFFFFFF
            nxt = p + 4 if typ <= 0x7F else p + 4 + ((param + 3) & ~3)
            if typ == 0:
                break
            if typ == 0x80:
                q, qend = p + 4, p + 4 + param
                while q < qend:
                    h = be16(stb, q)
                    if h & 0x8000:
                        size = ((h & 0x7FFF) << 16) | be16(stb, q + 2)
                        ptype, c = be32(stb, q + 4), q + 8
                    else:
                        size, ptype, c = h, be16(stb, q + 2), q + 4
                    # DATA_ID paragraph content = u32 subtype, u32 CHANNEL id,
                    # then the TParse_TParagraph_data blob (verified against
                    # tale.stb: d_act3 = id 9 + payload dir 0x00/frames 0x14 =
                    # the 20-frame ending fade-OUT; d_act2 = id 4 save-bits)
                    if ptype == 0x81 and size >= 8:
                        channels.add(be32(stb, c + 4))
                    q = c + ((size + 3) & ~3) if size else c
            p = nxt
        off += bsize
    return channels


# ---------------------------------------------------------------------------
# MODE: inventory (Pass 1 on a donor stage)
# ---------------------------------------------------------------------------
def mode_inventory(stage_name):
    sdir = DONOR_ROOT / stage_name
    _, dzs = get_member(sdir / "Stage.arc", ".dzs")
    man = {"stage": stage_name, "pass": "inventory(P1)"}
    ch = dz_chunks(dzs)
    man["stage_chunks"] = {t: sum(c for c, _ in v) for t, v in ch.items()}
    man["hosting_shape"] = ("DEDICATED" if man["stage_chunks"].get("RTBL", 1) > 1
                            else "PACKED")  # §302 hybrid rule (MULT>1 check below)

    ev_raw = arc_member(sdir / "Stage.arc", "event_list.dat")
    if ev_raw:
        man["events"] = classify_events(EventFile(ev_raw))
    rooms = sorted(sdir.glob("Room*.arc"))
    man["rooms"] = {}
    for r in rooms:
        _, dzr = get_member(r, ".dzr")
        rch = dz_chunks(dzr)
        layers = sorted(t for t in rch if t[:3] in ("ACT", "SCO") and t not in
                        ("ACTR", "SCOB"))
        man["rooms"][r.name] = {
            "chunks": {t: sum(c for c, _ in v) for t, v in rch.items()},
            "story_layers_present": layers,
            "plyr": decode_plyr(dzr),
        }
    print(json.dumps(man, indent=1))
    return 0


# ---------------------------------------------------------------------------
# MODE: regress (Pass 3 battery on the live R_DL01 — the pilot)
# ---------------------------------------------------------------------------
def check(results, name, ok, detail):
    results.append((name, bool(ok), detail))
    print(("PASS " if ok else "FAIL ") + name + " — " + detail)


def mode_regress():
    results = []
    _, dzs = get_member(HOST_STG, "stage.dzs")
    _, dzr = get_member(HOST_ROOM0, ".dzr")
    revt = decode_revt(dzs)
    plyr = decode_plyr(dzr)
    ev_raw = arc_member(HOST_STG, "event_list.dat")
    events = classify_events(EventFile(ev_raw)) if ev_raw else {}
    demo_members = member_names(HOST_DEMO_ARC)
    _, tale = get_member(HOST_DEMO_ARC, "tale.stb")
    fades = stb_fade_channels(tale) & {9, 10} if tale else set()

    print("== events ==")
    for nm, e in events.items():
        print(f"  {nm:12s} {e['archetype']:12s} staffs={e['staff_names']}"
              f" Stage={e['Stage']} StartCode={e['StartCode']}")
    print("== REVT ==")
    for r in revt:
        print(" ", r)
    print("== PLYR (room 0) ==")
    for p in plyr:
        print(" ", p)
    print(f"== tale.stb fade channels: {sorted(fades)} ==\n")

    # --- the §323 asserts, against the PROVEN-GOOD stage -------------------
    tale_ev = events.get("TALE_DEMO")
    check(results, "classifier:TALE_DEMO=A", tale_ev
          and tale_ev["archetype"].startswith("A"),
          f"TALE_DEMO archetype={tale_ev['archetype'] if tale_ev else 'MISSING'}"
          " (tale closed via same-stage reload — MUST classify A)")

    bad_exits = [r for r in revt
                 if r["exit_finish"] != 0xFF or r["exit_skip"] != 0xFF]
    check(results, "revt:exit-bytes-0xFF", not bad_exits,
          f"{len(revt)} REVT entries; non-0xFF exits: {bad_exits or 'none'} (§321)")

    ids = {p["spawn_id"]: p for p in plyr}
    entry = ids.get(0xC8)
    check(results, "plyr:entry-0xC8-fires-REVT0", entry
          and entry["event_byte"] == 0x00,
          f"0xC8={entry}")
    landing = ids.get(0xCB)
    ok_landing = (landing and landing["event_byte"] == 0xFF
                  and abs(landing["pos"][1] - 375.0) < 1.0)
    check(results, "plyr:landing-0xCB-teardown", ok_landing,
          f"0xCB={landing} (must be evt 0xff, platform height — the continuity"
          " spawn, NOT the entry point)")

    check(results, "fade-pairing:tale-A", (not fades) or
          (tale_ev and tale_ev["archetype"].startswith("A")),
          f"tale.stb fade channels {sorted(fades)} require archetype A")

    sc_missing = []
    for nm, e in events.items():
        if not e["archetype"].startswith("A") or not e["StartCode"]:
            continue
        for group in e["StartCode"]:
            codes = group if isinstance(group, list) else [group]
            for code in codes:
                if not isinstance(code, int):
                    continue
                mapped = STARTCODE_ALIAS.get("LinkRM", {}).get(code, code)
                if mapped not in ids:
                    sc_missing.append((nm, code, mapped))
    check(results, "startcode:closure", not sc_missing,
          f"unresolved StartCodes (after §322b alias): {sc_missing or 'none'}")

    # B-closure applies to REVT-FIRED events only: a B event fired by stage
    # re-entry has no other close path than its WAIT finish-flag cut. Order
    # events (tale_1/tale_2 pattern) close via their own actor/staff flow —
    # the donor itself authors tale_2 as PACKAGE-only with no WAIT (donor
    # inventory receipt), and tale2 is user-confirmed closed in game.
    revt_names = {r["name"] for r in revt}
    b_bad = [nm for nm, e in events.items()
             if e["archetype"].startswith("B") and nm in revt_names
             and "WAIT" not in e["cut_names"]]
    check(results, "b-closure:WAIT-cut(REVT-fired)", not b_bad,
          f"REVT-fired B events lacking a WAIT cut: {b_bad or 'none'}"
          f" (REVT-fired: {sorted(revt_names) or 'none'})")

    fade_res = [m for m in demo_members
                if "fade" in m.lower() or "blackfadebox" in m.lower()]
    check(results, "demo-arc:fade-box-resident", bool(fade_res),
          f"fade resources in Demo01.arc: {fade_res or 'NONE'}")

    n_fail = sum(1 for _, ok, _ in results if not ok)
    print(f"\n== REGRESSION: {len(results) - n_fail}/{len(results)} passed ==")
    return 1 if n_fail else 0


if __name__ == "__main__":
    if len(sys.argv) >= 3 and sys.argv[1] == "inventory":
        sys.exit(mode_inventory(sys.argv[2]))
    elif len(sys.argv) >= 2 and sys.argv[1] == "regress":
        sys.exit(mode_regress())
    else:
        sys.exit("usage: space_kit.py inventory <DonorStageDir> | regress")
