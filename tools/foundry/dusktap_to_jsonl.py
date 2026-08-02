"""Foundry P2 schema v0: DuskTap/Dolphin log -> probe-event JSONL.

Event: {"t": "MM:SS:mmm", "t_s": float_seconds, "site": str, "addr": hex,
        "regs": [10 hex r3..r12], "lr": hex, "derefs": [{"reg","addr","words"}],
        "key": [site-specific projection]}
The `key` is the semantic identity the differ aligns on (id for emitters/SE,
proc for actor creates, ascii name for placements); regs/derefs stay for depth.

Usage: python dusktap_to_jsonl.py <dolphin.log> <out.jsonl> [--sites a,b,...]
"""
import sys, re, json

SITES = {
    0x8007DBC4: ("setSimple",         lambda r, d: ["id", r[1]]),
    0x8026F47C: ("js_actor_cmd",      lambda r, d: []),
    0x8026F9C4: ("js_camera_cmd",     lambda r, d: []),
    0x802700F4: ("js_message_cmd",    lambda r, d: []),
    0x802750DC: ("js_reserved_cmd",   lambda r, d: []),
    0x8002451C: ("fopAcM_create_s16", lambda r, d: ["proc", r[0]]),
    0x80024598: ("fopAcM_create_chr", lambda r, d: ["name", _deref_ascii(d)]),
    0x8025F0E4: ("emitter_create",    lambda r, d: ["id", r[2]]),
    0x802A6720: ("se_start",          lambda r, d: ["id", r[1]]),
    0x80041628: ("stage_placement",   lambda r, d: ["name", _deref_ascii(d)]),
    0x80281258: ("bgm_note_on",       lambda r, d: ["note", r[1], r[2]]),
}

def _deref_ascii(derefs):
    """First 8 bytes of the first deref, as ascii actor name (dot-padded)."""
    if not derefs:
        return None
    words = derefs[0]["words"][:2]
    raw = b"".join(int(w, 16).to_bytes(4, "big") for w in words)
    return "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in raw).rstrip(".")

# placate linters: real SITES built after helper exists
SITES = {a: (n, f) for a, (n, f) in SITES.items()}

BP_RE = re.compile(
    r"^(\d+):(\d+):(\d+) .*?: BP ([0-9a-fA-F]{8})\s+--- \(([0-9a-fA-F ]+)\) LR=([0-9a-fA-F]+)")
DT_RE = re.compile(
    r"^\d+:\d+:\d+ .*?: DuskTap \[(r\d+)\]@([0-9a-fA-F]+)=([0-9a-fA-F ]+)")
# Receiver-side (dusklight) tap lines — self-timestamped (ms since first tap):
#   [DuskLog] §P2 emitter t=12345 id=0031 pos=(x,y,z)
DL_EMIT_RE = re.compile(
    r"\[DuskLog\] §?P2 emitter t=(\d+) id=([0-9a-fA-F]+) pos=\(([^)]*)\)")
# Receiver ExtSeq note stream — mirrors donor bgm_note_on @80281258. The port's
# Ja1Track::noteOn emits (voice DECIMAL, key/vel/gate HEX) behind DUSK_EXTSEQ_NOTE_TAP=1:
#   [ExtSeq] §P2 noteTap note=  0 p=(0018,005f,0d08)
# Donor key = ["note", noteid, note] (voice, pitch) — vel+gate kept aside, as the
# donor does (velocity is regs[3], gate regs[4], neither in the align key).
DL_NOTE_RE = re.compile(
    r"§?P2 noteTap note=\s*(\d+)\s+p=\(([0-9a-fA-F]+),([0-9a-fA-F]+),([0-9a-fA-F]+)\)")
# Receiver census spawn — legacy partial mirror (omits grass/bulk paths):
#   [Spawn] src=census:Aj1@(-202676,745,317655) proc=NPC_AJ
DL_SPAWN_RE = re.compile(
    r"\[Spawn\] src=census:([A-Za-z0-9_]+)@\(([^)]*)\) proc=(NPC_[A-Za-z0-9_]+)")
# §P2 placement-parity tap — the COMPLETE, uncapped placement mirror (every
# census spawn incl. NPC_EXTVEG grass), emitted by dExtNpcPop_placeTapLog behind
# DUSK_PLACE_TAP=1. Carries t=ms (fixes the one-shot rate math) + layer chunk.
# Donor key = ["name", ascii]:
#   [DuskLog] §P2 placement t=1234 name=kusax7 proc=NPC_EXTVEG chunk=ACTR pos=(x,y,z)
DL_PLACE_RE = re.compile(
    r"§?P2 placement t=(\d+) name=(\S+) proc=(\S+) chunk=(\S+) pos=\(([^)]*)\)")
# Receiver action-state tap (§224, include/dusk/state_tap.hpp):
#   [DuskLog] §P2 state t=123 tag=pig act=1 mode=50
DL_STATE_RE = re.compile(
    r"\[DuskLog\] §?P2 state t=(\d+) tag=(\S+) act=(-?\d+) mode=(-?\d+)")
# §238 NPC string-state variant (function-pointer states — method names):
DL_STATEN_RE = re.compile(
    r"\[DuskLog\] §?P2 staten t=(\d+) tag=(\S+) fn=(\S+)")

def parse(path):
    events, cur = [], None
    wrap_s, last_raw = 0.0, None
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            m = BP_RE.match(line)
            if m:
                mm, ss, ms = int(m.group(1)), int(m.group(2)), int(m.group(3))
                raw_s = mm * 60 + ss + ms / 1000.0
                # Timestamps carry no hour and the log is append-only, so time
                # must be monotonic: any backward step is one or more hour wraps.
                if last_raw is not None:
                    while raw_s + wrap_s < last_raw - 1.0:
                        wrap_s += 3600.0
                last_raw = raw_s + wrap_s
                addr = int(m.group(4), 16)
                regs = m.group(5).split()
                cur = {"t": f"{mm:02d}:{ss:02d}:{ms:03d}", "t_s": raw_s + wrap_s,
                       "addr": f"{addr:08x}", "regs": regs, "lr": m.group(6),
                       "derefs": []}
                events.append(cur)
                continue
            m = DT_RE.match(line)
            if m and cur is not None:
                cur["derefs"].append({"reg": m.group(1), "addr": m.group(2),
                                      "words": m.group(3).split()})
                continue
            m = DL_EMIT_RE.search(line)
            if m:
                ms = int(m.group(1))
                rid = int(m.group(2), 16)
                events.append({"t": f"{ms/1000:.3f}s", "t_s": ms / 1000.0,
                               "addr": "dusklog", "regs": [], "lr": "",
                               "derefs": [], "site": "emitter_create",
                               "key": ["id", f"{rid:08x}"],
                               "pos": m.group(3)})
                continue
            m = DL_NOTE_RE.search(line)
            if m:
                voice = int(m.group(1))          # decimal in the port line
                key = int(m.group(2), 16)
                vel = int(m.group(3), 16)
                gate = int(m.group(4), 16)
                events.append({"t": "", "t_s": 0.0, "addr": "dusklog",
                               "regs": [], "lr": "", "derefs": [],
                               "site": "bgm_note_on",
                               "key": ["note", f"{voice:08x}", f"{key:08x}"],
                               "vel": f"{vel:08x}", "gate": f"{gate:08x}"})
                continue
            m = DL_PLACE_RE.search(line)
            if m:
                ms = int(m.group(1))
                events.append({"t": f"{ms/1000:.3f}s", "t_s": ms / 1000.0,
                               "addr": "dusklog", "regs": [], "lr": "",
                               "derefs": [], "site": "stage_placement",
                               "key": ["name", m.group(2)], "proc": m.group(3),
                               "chunk": m.group(4), "pos": m.group(5)})
                continue
            m = DL_SPAWN_RE.search(line)
            if m:
                events.append({"t": "", "t_s": 0.0, "addr": "dusklog",
                               "regs": [], "lr": "", "derefs": [],
                               "site": "stage_placement",
                               "key": ["name", m.group(1)],
                               "proc": m.group(3), "pos": m.group(2)})
                continue
            m = DL_STATE_RE.search(line)
            if m:
                ms = int(m.group(1))
                events.append({"t": f"{ms/1000:.3f}s", "t_s": ms / 1000.0,
                               "addr": "dusklog", "regs": [], "lr": "",
                               "derefs": [], "site": "actor_state",
                               "key": ["st", m.group(2), m.group(3), m.group(4)]})
                continue
            m = DL_STATEN_RE.search(line)
            if m:
                ms = int(m.group(1))
                events.append({"t": f"{ms/1000:.3f}s", "t_s": ms / 1000.0,
                               "addr": "dusklog", "regs": [], "lr": "",
                               "derefs": [], "site": "actor_state_npc",
                               "key": ["stn", m.group(2), m.group(3)]})
                continue
    # resolve sites + keys after derefs are attached
    for ev in events:
        if "site" in ev:  # receiver-side events arrive pre-resolved
            continue
        addr = int(ev["addr"], 16)
        name, keyfn = SITES.get(addr, (ev["addr"], lambda r, d: []))
        ev["site"] = name
        try:
            ev["key"] = keyfn(ev["regs"], ev["derefs"])
        except Exception:
            ev["key"] = []
    return events

def main():
    src, dst = sys.argv[1], sys.argv[2]
    only = None
    if len(sys.argv) > 4 and sys.argv[3] == "--sites":
        only = set(sys.argv[4].split(","))
    events = parse(src)
    n = 0
    with open(dst, "w", encoding="utf-8") as f:
        for ev in events:
            if only and ev["site"] not in only:
                continue
            f.write(json.dumps(ev, separators=(",", ":")) + "\n")
            n += 1
    sites = {}
    for ev in events:
        sites[ev["site"]] = sites.get(ev["site"], 0) + 1
    print(f"{n} events written ({len(events)} parsed) from {src}")
    for s, c in sorted(sites.items(), key=lambda kv: -kv[1]):
        print(f"  {s}: {c}")

if __name__ == "__main__":
    main()
