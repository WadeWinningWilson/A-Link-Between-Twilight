#!/usr/bin/env python3
# ============================================================================
# ERA WARNING (Foundry kit-family sweep, 2026-08-21 - user order: kits may
# still carry mount/bake/legs-era recipes; History saw remnants and this
# sweep CONFIRMED them family-wide).
#
# THIS KIT'S OWN HEADER NAMES ITS BAKE-ERA CHAIN ('merge_event's
# bake-era chain stays in the skeleton dir') and MOD points at
# model_replacements. Its disc-first donor reads are era-correct; the
# bake/skeleton paths are the fork-era remnant.
#
# Era doctrine of record: actor_kit.py's banner + its era-split checklist
# ([FORK] vs [PLUGIN] columns) and the trace method at
# docs/state/ww-plugin-outset.md #How-ports-land. PLUGIN delivery never
# routes through dExtNpcMount / f_pc edits / files.cmake / model_replacements
# - those are FORK-era mechanisms a stock-dusklight user never receives.
# This banner marks the era; a DEEP retool of this kit is a named plate item,
# not implied by the banner's presence.
# ============================================================================
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
#
# Usage:
#   space_kit.py inventory <DonorStageDirName>   Pass-1 over a DONOR stage
#   space_kit.py regress                         Pass-3 battery over the LIVE host
#
#   Inputs   : inventory — DISC-FIRST (backend.extraIsoPath / WW_ISO; game-id
#              checked) with the Ex WW tree as LOUD fallback; SOURCE line printed
#              regress   — <mod>/files/res/Stage/R_DL01/STG_00.arc
#                          <mod>/files/res/Stage/R_DL01/R00_00.arc
#   Outputs  : NONE on disk. inventory prints the Pass-4 manifest as JSON on
#              stdout; regress prints the assert battery. Redirect to capture.
#   Exit     : regress returns 1 if ANY assert fails, 0 if all pass — so it is
#              usable as a recipe GATE, not merely a report. inventory returns
#              its own status from mode_inventory.
#   Idempotent: trivially — read-only in both modes, no game data is written.
#   Order    : regress runs AFTER the R_DL01 host arcs are built and baked; it
#              asserts against the FINISHED tale, so running it early reports
#              failures that only mean "not built yet". inventory has no order at
#              all — it reads the donor tree and never touches the mod.
#   R1 note  : this is a VALIDATOR, not a conversion step. It produces no output
#              file, so the runner's checksum short-circuit has nothing to match
#              and would either skip it forever or re-run it blindly — a recipe
#              must mark it ALWAYS-RUN and place it as a terminal gate (or after
#              each bake), never as a producing step.
# ============================================================================
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
# Consolidation (tale §850, user-ordered): the kit now lives in tools/foundry;
# merge_event's bake-era chain stays in the skeleton dir — path kept for it.
sys.path.insert(0, str(Path(__file__).parent.parent / "ww_crew_restoration_skeleton"))
from jstudio_stb import yaz0_dec, rarc_members
from merge_event import EventFile, arc_member, ESZ, SSZ, CSZ, DSZ

DONOR_ROOT = Path("D:/XXXXXXX/Ex WW/files/res/Stage")

# ============================================================================
# NATIVE-DISC INPUT (user routing via tale-bus WHOSE-TURN: "kit right now works
# for mainly mounts/legs, not native disc"). Donor reads are DISC-FIRST:
# the WW image named by `backend.extraIsoPath` (the row-14 generic field the
# plugin itself consumes) or env WW_ISO; the extracted tree is the FALLBACK,
# and every inventory prints its SOURCE line so which one served can never be
# assumed (source-attribution honesty).
# ============================================================================
import json as _json
import os as _os
sys.path.insert(0, str(Path(__file__).parent.parent / "foundry"))
import ww_disc as _wd

_DISC = None  # (file, boot, label) | False once probed and absent


def _ww_iso():
    global _DISC
    if _DISC is not None:
        return _DISC or None
    path = _os.environ.get("WW_ISO", "")
    if not path:
        cfg = _os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight\config.json")
        try:
            path = _json.load(open(cfg, encoding="utf-8")).get("backend.extraIsoPath", "")
        except OSError:
            path = ""
    if path and Path(path).is_file():
        try:
            f, boot = _wd.iso_open(path, _wd.WW_IDS)
            _DISC = (f, boot, "disc %s (%s)" % (boot[:6].decode("ascii", "replace"), path))
            return _DISC
        except (OSError, ValueError) as e:
            print("SOURCE: disc configured but unusable (%s) -- falling back to tree" % e)
    _DISC = False
    return None


def donor_file(stage_name, member):
    """(bytes, source-label) for res/Stage/<stage>/<member> -- disc first."""
    d = _ww_iso()
    if d:
        try:
            data = _wd.iso_read_file(d[0], d[1], "res/Stage/%s/%s" % (stage_name, member))
            return data, d[2]
        except FileNotFoundError:
            pass
    p = DONOR_ROOT / stage_name / member
    if p.is_file():
        return p.read_bytes(), "tree FALLBACK %s" % p
    raise FileNotFoundError("%s/%s on neither disc nor tree" % (stage_name, member))


def donor_rooms(stage_name):
    """Room*.arc member names for the stage -- FST-listed on disc, glob on tree."""
    d = _ww_iso()
    if d:
        pre = "res/Stage/%s/Room" % stage_name
        names = [pth.rsplit("/", 1)[1] for pth, _o, _l in _wd.fst_walk(d[0], d[1])
                 if pth.startswith(pre) and pth.endswith(".arc")]
        if names:
            return sorted(names)
    return sorted(q.name for q in (DONOR_ROOT / stage_name).glob("Room*.arc"))

MOD = Path("C:/Users/xxxxx/AppData/Roaming/TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration")
HOST_STG = MOD / "files/res/Stage/R_DL01/STG_00.arc"
HOST_ROOM0 = MOD / "files/res/Stage/R_DL01/R00_00.arc"
# Row-21 retirement made arcs/ a moving target: the live overlay first, the
# retired REFERENCE copy second (a validator asserting the CLOSED tale may
# read reference bytes; it must say which it read — the source line prints).
# §742 MEASURED: our Demo01.arc vs disc res/Object/Demo01.arc — member sets
# EQUAL, stb/bck/brk BYTE-IDENTICAL; only ba.bdl/fuku.bdl/blackfadebox.bdl
# differ (DN-3 model adaptation). TRANSFORMED bucket, so mount-sourced is
# legitimate-for-now — WITH ITS EXIT (§723 rule): this input flips to the
# disc path the day BDL adaptation happens at consumption (L1 layer-2) instead
# of at bake; until then the three adapted models ARE the residue.
_DEMO_CANDIDATES = [MOD / "arcs/Demo01.arc", MOD / "arcs_retired_row21/Demo01.arc"]
HOST_DEMO_ARC = next((c for c in _DEMO_CANDIDATES if c.is_file()), _DEMO_CANDIDATES[0])

# §322b alias (port-side code the kit VERIFIES, never bakes — §325 trim):
# donor StartCode -> host spawn id, per stage alias in d_event_data.cpp
STARTCODE_ALIAS = {"LinkRM": {0: 0xCB}}

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]
bef32 = lambda d, o: struct.unpack_from(">f", d, o)[0]


def get_member(arc_path, suffix):
    raw = arc_path if isinstance(arc_path, (bytes, bytearray)) else Path(arc_path).read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    for nm, blob in rarc_members(raw):
        if nm.lower().endswith(suffix.lower()):
            if blob[:4] == b"Yaz0":
                blob = yaz0_dec(blob)
            return nm, blob
    return None, None


def member_names(arc_path):
    raw = arc_path if isinstance(arc_path, (bytes, bytearray)) else Path(arc_path).read_bytes()
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

# ============================================================================
# INTERIOR-COMPLETENESS (interior-entrance-transitions.md §3, routed): an
# interior is INCOMPLETE while any of its TRANSITION ACTORS is unported —
# the donor dispatch chain is the complete set, nothing invented. Placement
# names map to actor classes via the donor's own l_objectName rows; the
# ported/unported roster is DECLARED with the doc as authority and shrinks as
# History ports (update BOTH the doc table and this dict in the same change).
# ============================================================================
TRANSITION_CLASS = {
    # l_objectName spelling -> donor actor class
    "door10": "DOOR10", "door11": "DOOR10", "door20": "DOOR10", "door21": "DOOR10",
    "Zenshut": "DOOR10", "keyshut": "DOOR10", "K_Zshut": "DOOR10",
    "door12": "DOOR12", "door12M": "DOOR12", "door12B": "DOOR12",
    "door13": "DOOR12", "door13M": "DOOR12", "door13B": "DOOR12",
    "keyS12": "DOOR12", "ZenS12": "DOOR12",
    "doorKD": "KDDOOR", "ZenshKD": "KDDOOR", "K_ZshKD": "KDDOOR",
    "doorSH": "KDDOOR", "doorSZ": "KDDOOR",
    "KNOB00": "KNOB00", "KNOB01": "KNOB00", "KNOB02": "KNOB00", "KNOB03": "KNOB00",
    "KNOB00D": "KNOB00", "KNOB01D": "KNOB00", "KNOB02D": "KNOB00", "KNOB03D": "KNOB00",
    "ATdoor": "ATDOOR", "Mori1": "MDOOR",
    "Ashut": "SHUTTER", "Ashut2": "SHUTTER2",
    "htetu": "OBJ_HOLE", "Hhyu1": "OBJ_HOLE",
    "ladder": "OBJ_LADDER", "Mhsg4h": "LSTAIR", "Mhsg6": "LSTAIR", "Mhsg9": "LSTAIR",
}
PORTED_CLASSES = {"KNOB00", "OBJ_LADDER"}  # doc table; the other 13 are unported


def _placement_names(dz):
    """8-byte actor names from every placement chunk (ACTR/SCOB/DOOR/TGDR/TGSC
    + story layers), at each tag's record stride."""
    names = set()
    for tag, entries in dz_chunks(dz).items():
        base = tag[:3]
        if tag in ("ACTR", "TGOB") or base == "ACT":
            stride = 0x20
        elif tag in ("SCOB", "TGSC", "DOOR", "TGDR", "Door") or base == "SCO":
            stride = 0x24
        else:
            continue
        for cnt, off in entries:
            # dz_chunks stores (count, OFFSET into dz) -- slice the dz itself
            for i in range(cnt):
                rec = dz[off + i * stride: off + i * stride + 8]
                nm = rec.split(b"\x00")[0]
                if nm:
                    names.add(nm.decode("ascii", "replace"))
    return names


def transition_report(all_names):
    found = {}
    for nm in sorted(all_names):
        cls = TRANSITION_CLASS.get(nm)
        if cls:
            found.setdefault(cls, []).append(nm)
    unported = sorted(c for c in found if c not in PORTED_CLASSES)
    return {
        "transition_actors_found": {c: found[c] for c in sorted(found)},
        "unported_transition_classes": unported,
        "interior_status": ("INCOMPLETE — %d transition class(es) unported: %s"
                            % (len(unported), ", ".join(unported)))
                           if unported else "transition-complete",
    }



# ============================================================================
# LAYER-AWARE WARP EMITTER (interconnected §805, user-directed: move porting
# load off AI instances and onto the kits). Reads each stage's DZR layer
# chunks off the DISC and emits dBootStage_add("<stage>,<room>,<layer>") rows
# for every layer that actually CONTAINS placements — labeled by the donor's
# own actor names (Ob1, Ko1 — never hypothesis labels). The engine grammar
# already exists (§638); covenant-clean by construction: runtime strings, no
# compiled stage literals. Layer index = the chunk suffix (ACT0..ACT9,a,b =
# 0..11), the same number setRoomLayerOverride consumes.
# ============================================================================
_LAYER_IDX = {c: i for i, c in enumerate("0123456789ab")}


def _layer_names(dz):
    """{layer_index: sorted actor names} for layers WITH content; -1 = base."""
    out = {}
    for tag, entries in dz_chunks(dz).items():
        base3 = tag[:3]
        if tag in ("ACTR", "TGOB"):
            layer, stride = -1, 0x20
        elif tag in ("SCOB", "TGSC", "DOOR", "TGDR", "Door"):
            layer, stride = -1, 0x24
        elif base3 == "ACT" and tag[3] in _LAYER_IDX:
            layer, stride = _LAYER_IDX[tag[3]], 0x20
        elif base3 == "SCO" and tag[3] in _LAYER_IDX:
            layer, stride = _LAYER_IDX[tag[3]], 0x24
        else:
            continue
        for cnt, off in entries:
            for i in range(cnt):
                nm = dz[off + i * stride: off + i * stride + 8].split(b"\x00")[0]
                if nm:
                    out.setdefault(layer, set()).add(nm.decode("ascii", "replace"))
    return {k: sorted(v) for k, v in out.items()}


def mode_warps(specs):
    """specs: ['Stage' or 'Stage,room' ...] -> dBootStage_add rows + artifact."""
    lines = []
    for spec in specs:
        stage, _, room_s = spec.partition(",")
        want_room = int(room_s) if room_s else None
        for rname in donor_rooms(stage):
            room_no = int("".join(ch for ch in rname if ch.isdigit()) or 0)
            if want_room is not None and room_no != want_room:
                continue
            rbytes, _src = donor_file(stage, rname)
            _, dzr = get_member(rbytes, ".dzr")
            if dzr is None:
                continue
            per = _layer_names(dzr)
            for layer in sorted(k for k in per if k >= 0):
                lines.append('dBootStage_add("%s,%d,%d");  // %s'
                             % (stage, room_no, layer, " ".join(per[layer])))
            if -1 in per and want_room is not None:
                lines.append("// %s,%d base (layer -1): %s"
                             % (stage, room_no, " ".join(per[-1][:10])))
    for ln in lines:
        print(ln)
    out = Path(__file__).resolve().parents[2] / "docs" / "state" / "ww-staging" / "warp_rows.inc"
    with open(out, "w", encoding="utf-8", newline="\r\n") as f:
        f.write("// GENERATED by space_kit warps (interconnected §805) — the\n"
                "// donor's own layer map as dBootStage_add rows. Labels are the\n"
                "// donor's placement names. Engine wires; regenerate any time.\n")
        for ln in lines:
            f.write(ln + "\n")
    print("\n%d row(s) -> %s" % (len(lines), out))
    return 0


def mode_inventory(stage_name):
    stage_bytes, src = donor_file(stage_name, "Stage.arc")
    print("SOURCE: %s" % src)
    _, dzs = get_member(stage_bytes, ".dzs")
    man = {"stage": stage_name, "pass": "inventory(P1)", "source": src}
    ch = dz_chunks(dzs)
    man["stage_chunks"] = {t: sum(c for c, _ in v) for t, v in ch.items()}
    man["hosting_shape"] = ("DEDICATED" if man["stage_chunks"].get("RTBL", 1) > 1
                            else "PACKED")  # §302 hybrid rule (MULT>1 check below)

    # merge_event.arc_member is Path-only; the suffix reader handles bytes and
    # event_list.dat is a unique member name, so exact-suffix match is exact.
    _, ev_raw = get_member(stage_bytes, "event_list.dat")
    if ev_raw:
        man["events"] = classify_events(EventFile(ev_raw))
    man["rooms"] = {}
    for rname in donor_rooms(stage_name):
        rbytes, _rsrc = donor_file(stage_name, rname)
        r = Path(rname)
        _, dzr = get_member(rbytes, ".dzr")
        if dzr is None:
            man["rooms"][rname] = {"note": "no .dzr member (model-only arc)"}
            continue
        rch = dz_chunks(dzr)
        layers = sorted(t for t in rch if t[:3] in ("ACT", "SCO") and t not in
                        ("ACTR", "SCOB"))
        man["rooms"][r.name] = {
            "chunks": {t: sum(c for c, _ in v) for t, v in rch.items()},
            "story_layers_present": layers,
            "plyr": decode_plyr(dzr),
        }
    all_names = set()
    _, dzs2 = get_member(stage_bytes, ".dzs")
    all_names |= _placement_names(dzs2)
    for rname in donor_rooms(stage_name):
        rb, _ = donor_file(stage_name, rname)
        _, rdzr = get_member(rb, ".dzr")
        if rdzr:
            all_names |= _placement_names(rdzr)
    man.update(transition_report(all_names))
    print(json.dumps(man, indent=1))
    return 0


# ---------------------------------------------------------------------------
# MODE: regress (Pass 3 battery on the live R_DL01 — the pilot)
# ---------------------------------------------------------------------------
def check(results, name, ok, detail):
    results.append((name, bool(ok), detail))
    print(("PASS " if ok else "FAIL ") + name + " — " + detail)


def mode_regress():
    print("SOURCE demo arc: %s%s" % (HOST_DEMO_ARC,
          "  [RETIRED REFERENCE]" if "retired" in str(HOST_DEMO_ARC) else ""))
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
    if len(sys.argv) >= 3 and sys.argv[1] == "warps":
        sys.exit(mode_warps(sys.argv[2:]))
    if len(sys.argv) >= 3 and sys.argv[1] == "inventory":
        sys.exit(mode_inventory(sys.argv[2]))
    elif len(sys.argv) >= 2 and sys.argv[1] == "regress":
        sys.exit(mode_regress())
    else:
        sys.exit("usage: space_kit.py inventory <DonorStageDir> | warps <Stage[,room]>... | regress")
