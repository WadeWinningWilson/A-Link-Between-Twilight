#!/usr/bin/env python3
"""§267: bake donor-verbatim connectivity/population chunks into the NATIVE
room arcs — the quest-substrate nativization step (Foundry, greenlit pre-quest).

WHY (the tale lesson, generalized)
----------------------------------
Quests run on event bits -> LAYER switches -> per-layer ACTR sets. The bridge
machinery (npc/*.ini region transports + population CSVs) would have to
reimplement layer semantics; native ACTR/SCLS/DOOR chunks in the room arcs the
TP engine ALREADY loads get donor behavior from engine code that already
exists. Every quest built on the bridge is a future migration (§263-bis tale
precedent) — so the substrate goes native BEFORE quest buildout.

WHAT IT DOES
------------
Reads the donor room's .dzr, extracts SCLS / DOOR / TGDR / ACTR(+layers) /
SCOB(+layers) / TGOB / TGSC entries, and merges them into the native room
arc's room.dzr:

  - ACTR-family entries are FILTERED against the receiver's l_objectName
    registry (src/d/d_stage.cpp OBJNAME rows): only names the port resolves
    natively are baked; the rest are DEFERRED (stay CSV-side until their
    actor ports) and reported. One-way ratchet: rerun after each actor port.
  - SCLS dest stages are TRANSLATED donor->host via the npc/*.ini
    arc=/host_stage= pairs (+ the exterior seed sea->F_DL01). Unmapped dests
    are SKIPPED and reported — never guessed.
  - Entries merge byte-verbatim with per-entry dedup: rerunning is a no-op
    for already-baked rows (idempotent).
  - DN-1 fence: this tool touches placement/exit chunks ONLY. It never
    writes BG/collision data.

DEFAULT IS DRY-RUN. --write packs the arc (one-time .pre-bake-bak backup).
Do not --write while another lane has the arcs mid-edit.

Usage:
  bake_room_chunks.py <donor-room-arc> <native-room-arc> [--tags T1,T2]
                      [--write] [--report out.md]
e.g.
  bake_room_chunks.py "D:/XXXXXXX/Ex WW/files/res/Stage/sea/Room44.arc" \
      <MOD>/files/res/Stage/F_DL01/R44_00.arc
"""
from __future__ import annotations
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
import re
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from adapt_bdl_arcs import be32, yaz0_dec

RECEIVER = Path(r"C:\Users\xxxxx\Documents\dusklight")
MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")

BASE_TAGS = ["SCLS", "DOOR", "TGDR", "ACTR", "SCOB", "TGOB", "TGSC"]
LAYER_SUFFIX = "0123456789ab"
NAMED_TAGS = {"DOOR", "TGDR", "ACTR", "SCOB", "TGOB", "TGSC"}  # filterable
# entry sizes for every tag we bake (WW layout, shared with the port). The
# gap-derivation cross-checks these; known size LEADS because the last chunk's
# gap includes trailing padding (derived >= known * count is fine, < is not).
KNOWN_SIZE = {"SCLS": 0xC, "ACTR": 0x20, "SCOB": 0x24, "DOOR": 0x24,
              "TGDR": 0x24, "TGOB": 0x24, "TGSC": 0x24}


def tag_size(tag: str) -> int | None:
    if tag.startswith("ACT"):
        return KNOWN_SIZE["ACTR"]
    if tag.startswith("SCO"):
        return KNOWN_SIZE["SCOB"]
    return KNOWN_SIZE.get(tag)


def layered(tag: str) -> list[str]:
    if tag in ("ACTR", "SCOB"):
        return [tag] + [tag[:3] + c for c in LAYER_SUFFIX]
    return [tag]


def read_dzr(arc_path: Path) -> tuple[list[tuple[str, bytes]], bytes]:
    raw = bytearray(arc_path.read_bytes())
    if bytes(raw[:4]) == b"Yaz0":
        raw = yaz0_dec(raw)
    members = dict(g.list_rarc_files(bytes(raw)))
    dzr = next((b for n, b in members.items() if n.lower().endswith(".dzr")),
               None)
    if dzr is None:
        raise SystemExit(f"no .dzr member in {arc_path}")
    return list(members.items()), dzr


def parse_chunks(dzr: bytes) -> dict[str, list[bytes]]:
    """tag -> [entry bytes] with entry size derived from offset gaps."""
    n = be32(dzr, 0)
    hdrs = []
    for i in range(n):
        tag = dzr[4 + i * 0xC: 8 + i * 0xC].decode("ascii", "replace")
        cnt = be32(dzr, 8 + i * 0xC)
        off = be32(dzr, 12 + i * 0xC)
        hdrs.append((tag, cnt, off))
    ends = sorted({off for _, _, off in hdrs} | {len(dzr)})
    out = {}
    for tag, cnt, off in hdrs:
        if cnt == 0:
            out.setdefault(tag, [])
            continue
        end = next((e for e in ends if e > off), len(dzr))
        size = tag_size(tag)
        if size is None:
            size = (end - off) // cnt      # derive for tags we don't bake
        elif end - off < size * cnt:
            # the chunk is SMALLER than the known layout demands — refuse
            raise SystemExit(f"{tag}: {cnt} entries need {size*cnt:#x} bytes "
                             f"but chunk spans {end-off:#x} — layout drift, "
                             "refusing to bake")
        out.setdefault(tag, [])
        for k in range(cnt):
            out[tag].append(bytes(dzr[off + k * size: off + (k + 1) * size]))
    return out


def build_dzr(chunks: dict[str, list[bytes]]) -> bytes:
    tags = [t for t, es in chunks.items() if es]
    out = bytearray(struct.pack(">I", len(tags)))
    hdr_at = {}
    for t in tags:
        hdr_at[t] = len(out)
        out += t.encode("ascii").ljust(4)[:4]
        out += struct.pack(">II", len(chunks[t]), 0)
    while len(out) % 4:
        out.append(0)
    for t in tags:
        struct.pack_into(">I", out, hdr_at[t] + 8, len(out))
        for e in chunks[t]:
            out += e
        while len(out) % 4:
            out.append(0)
    return bytes(out)


def receiver_objnames() -> set[str]:
    t = (RECEIVER / "src" / "d" / "d_stage.cpp").read_text(
        encoding="utf-8", errors="replace")
    return {m.group(1) for m in re.finditer(r'OBJNAME\("([^"]+)"', t)}


def host_map() -> dict[str, str]:
    """donor stage name -> native host stage, from the npc/*.ini pairs."""
    out = {"sea": "F_DL01"}  # exterior seed (Outset content hosted there)
    for ini in (MOD / "npc").glob("*.ini"):
        txt = ini.read_text(encoding="utf-8", errors="replace")
        arc = re.search(r"^arc=(\w+)", txt, re.M)
        host = re.search(r"^host_stage=(\w+)", txt, re.M)
        pop = re.search(r"^population_stage=(\w+)", txt, re.M)
        if host and (pop or arc):
            out[(pop or arc).group(1)] = host.group(1)
    return out


def entry_name(e: bytes) -> str:
    return e[:8].split(b"\0")[0].decode("ascii", "replace")


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    donor_arc, native_arc = Path(args[0]), Path(args[1])
    write = "--write" in sys.argv
    tags = BASE_TAGS
    if "--tags" in sys.argv:
        tags = sys.argv[sys.argv.index("--tags") + 1].split(",")
    report_to = (sys.argv[sys.argv.index("--report") + 1]
                 if "--report" in sys.argv else None)

    _, donor_dzr = read_dzr(donor_arc)
    members, native_dzr = read_dzr(native_arc)
    donor = parse_chunks(donor_dzr)
    native = parse_chunks(native_dzr)
    names_ok = receiver_objnames()
    hosts = host_map()

    rep = [f"# Room bake report — {donor_arc.name} -> {native_arc} (§267)",
           "", f"mode: {'WRITE' if write else 'DRY-RUN'}", ""]
    baked = deferred = skipped = deduped = 0
    added: dict[str, list[bytes]] = {}
    for base in tags:
        for tag in layered(base):
            if tag not in donor:
                continue
            for e in donor[tag]:
                nm = entry_name(e)
                if base == "SCLS":
                    dest = nm  # SCLS name field IS the dest stage
                    if dest in hosts:
                        e2 = bytearray(e)
                        e2[:8] = hosts[dest].encode("ascii").ljust(8, b"\0")
                        spawn, room = e[8], e[9]
                        # §267 open point: when several donor stages share one
                        # host, the DONOR room number is not the HOST room
                        # number — the room-within-host map (History's room
                        # fixes define it) must retarget e2[9] before --write.
                        shared = [d for d, h in hosts.items()
                                  if h == hosts[dest]]
                        warn = (" **[ROOM-MAP NEEDED: host hosts "
                                f"{len(shared)} donor stages — donor room "
                                f"{room} must be retargeted]**"
                                if len(shared) > 1 else "")
                        rep.append(f"- SCLS `{dest}` -> `{hosts[dest]}` "
                                   f"(spawn={spawn} room={room}) [{tag}]"
                                   + warn)
                        e = bytes(e2)
                    elif dest in hosts.values():
                        rep.append(f"- SCLS `{dest}` already host-native")
                    else:
                        skipped += 1
                        rep.append(f"- SCLS `{dest}` **SKIPPED — no host "
                                   f"mapping (add to npc ini or hosts seed)**")
                        continue
                elif base in NAMED_TAGS:
                    if nm not in names_ok:
                        deferred += 1
                        rep.append(f"- {tag} `{nm}` DEFERRED (proc not in "
                                   "receiver l_objectName — stays CSV-side)")
                        continue
                if e in native.get(tag, []) or e in added.get(tag, []):
                    deduped += 1
                    continue
                added.setdefault(tag, []).append(e)
                baked += 1
                if base != "SCLS":
                    rep.append(f"- {tag} `{nm}` BAKED")
    rep += ["", f"**{baked} baked · {deduped} already present (dedup) · "
            f"{deferred} deferred (unported procs) · {skipped} skipped "
            f"(unmapped SCLS dests).**",
            "", "> DN-1 fence: placement/exit chunks only — no BG/collision "
            "written. Rerun after each actor port to shrink the deferred "
            "list (idempotent)."]
    text = "\n".join(rep) + "\n"
    print(text)
    if report_to:
        Path(report_to).parent.mkdir(parents=True, exist_ok=True)
        Path(report_to).write_text(text, encoding="utf-8")

    if not write:
        print("dry-run — nothing written (use --write AFTER in-flight room "
              "edits land)")
        return 0
    if baked == 0:
        print("nothing new to bake — arc untouched")
        return 0
    # ========================================================================
    # §3.2 REFIT (P1 law): offset-stable extend/add per tag — NEVER a wholesale
    # re-layout (build_dzr retired from the write path; kept only for tests).
    # Every operation byte-diff-verifies that nothing outside its claimed
    # regions moved (dzx_offset_stable._verify_untouched), so RPAT/RPPN-class
    # absolute pointers can never silently break.
    # ========================================================================
    import dzx_offset_stable as ost
    new_dzr = bytes(native_dzr)
    for t, es in added.items():
        sz = tag_size(t)  # handles ACT*/SCO* layer names itself
        assert sz, f"no entry size for {t}"
        if any(c[0] == t for c in ost.chunk_table(new_dzr)):
            new_dzr, _ = ost.extend_chunk(new_dzr, t, es, sz)
        else:
            new_dzr, _ = ost.add_chunk(new_dzr, t, es, sz)
    chk = parse_chunks(new_dzr)  # verify with the same reader
    for t in set(list(native) + list(added)):
        want = len(native.get(t, [])) + len(added.get(t, []))
        assert len(chk.get(t, [])) == want, f"refit lost {t}"
    files = [(n, b) for n, b in members if not n.lower().endswith(".dzr")]
    dzr_name = next(n for n, _ in members if n.lower().endswith(".dzr"))
    files.append((dzr_name, new_dzr))
    bak = native_arc.with_suffix(native_arc.suffix + ".pre-bake-bak")
    if not bak.is_file():
        bak.write_bytes(native_arc.read_bytes())
        print(f"backup -> {bak.name}")
    native_arc.write_bytes(g.pack_rarc(files))
    print(f"wrote {native_arc}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
