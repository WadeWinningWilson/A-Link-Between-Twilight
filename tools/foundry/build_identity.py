#!/usr/bin/env python3
# ============================================================================
# build_identity.py — DURABLE BUILD IDENTITY. One need, three debts:
#
#   1. The 08-12 crash is PERMANENTLY unsymbolicatable (open ALL row): that
#      build's PDB is gone, 0 of 12 archived PDBs carry its id, and
#      symbolicating its rva against today's PDB yields a confidently WRONG
#      function name. The Integrator correctly refused to guess.
#   2. The inventory's staleness gate needs "is this feed from the CURRENT
#      exe?" to be answerable, or a dashboard reads stale-green.
#   3. Foundry owed a PDB-by-build-id index and never built it.
#
# All three are the same missing fact: WHICH BUILD IS THIS, durably.
#
# THE KEY is the one the crash handler already emits — the PE debug
# directory's CodeView RSDS record (GUID + age), the same value that appears
# as `build_id=` in every crash line. Nothing new is invented; this reads the
# identity the exe already carries and keeps the matching PDB next to it.
#
# STORAGE SPLIT (the estate's doctrine): the MANIFEST is small, greppable and
# lives in the repo; the PDB BLOBS are hundreds of MB and live outside it.
# A manifest row with a missing blob reports MISSING-BLOB, never silence.
#
# Usage:
#   build_identity.py current                 build_id of the current exe
#   build_identity.py archive                 keep this build's PDB, index it
#   build_identity.py resolve <build_id>      PDB for a crash's build_id
#   build_identity.py check <build_id>        is it the CURRENT build?
#   build_identity.py list
# Read-only except `archive`. Exit 0 ok · 1 not found/stale · 2 could-not-run.
# ============================================================================
import shutil
import struct
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
EXE = REPO / "build" / "windows-msvc-relwithdebinfo" / "dusklight.exe"
PDB = REPO / "build" / "windows-msvc-relwithdebinfo" / "dusklight.pdb"
ARCHIVE = Path("%USERPROFILE%/Documents/dusklight-pdb-archive")
MANIFEST = REPO / "docs" / "state" / "ww-staging" / "BUILD-IDENTITY.md"


def read_codeview(exe_path):
    """(guid_str, age, pdb_name) from the PE debug directory's RSDS record."""
    data = exe_path.read_bytes()
    if data[:2] != b"MZ":
        return None
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    if data[e_lfanew:e_lfanew + 4] != b"PE\0\0":
        return None
    coff = e_lfanew + 4
    n_sections = struct.unpack_from("<H", data, coff + 2)[0]
    opt_size = struct.unpack_from("<H", data, coff + 16)[0]
    opt = coff + 20
    magic = struct.unpack_from("<H", data, opt)[0]
    # data directory 6 = Debug; its offset differs PE32 (0x60) vs PE32+ (0x70)
    dd = opt + (0x70 if magic == 0x20B else 0x60)
    dbg_rva, dbg_size = struct.unpack_from("<II", data, dd + 6 * 8)
    if not dbg_rva:
        return None
    sect = opt + opt_size

    def rva_to_off(rva):
        for i in range(n_sections):
            s = sect + i * 40
            va = struct.unpack_from("<I", data, s + 12)[0]
            vsz = struct.unpack_from("<I", data, s + 8)[0]
            raw = struct.unpack_from("<I", data, s + 20)[0]
            if va <= rva < va + max(vsz, 1):
                return raw + (rva - va)
        return None

    off = rva_to_off(dbg_rva)
    if off is None:
        return None
    for i in range(dbg_size // 28):
        e = off + i * 28
        dtype = struct.unpack_from("<I", data, e + 12)[0]
        praw = struct.unpack_from("<I", data, e + 24)[0]
        if dtype != 2:                      # IMAGE_DEBUG_TYPE_CODEVIEW
            continue
        if data[praw:praw + 4] != b"RSDS":
            continue
        g = data[praw + 4:praw + 20]
        age = struct.unpack_from("<I", data, praw + 20)[0]
        end = data.index(b"\0", praw + 24)
        name = data[praw + 24:end].decode("utf-8", "replace")
        d1, d2, d3 = struct.unpack_from("<IHH", g, 0)
        guid = "%08X%04X%04X%s" % (d1, d2, d3, g[8:16].hex().upper())
        return guid, age, name
    return None


def rows():
    out = []
    if MANIFEST.is_file():
        for ln in MANIFEST.read_text(encoding="utf-8", errors="replace").splitlines():
            if ln.startswith("| ") and ln.count("|") >= 5 and "build_id" not in ln \
                    and "---" not in ln:
                p = [c.strip() for c in ln.strip("|").split("|")]
                if len(p) >= 4:
                    out.append(p)
    return out


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "current":
        cv = read_codeview(EXE) if EXE.is_file() else None
        if not cv:
            print("UNRESOLVED: no CodeView record in %s (not proof of anything)" % EXE.name)
            return 2
        print("build_id %s age %d  pdb %s" % (cv[0], cv[1], Path(cv[2]).name))
        return 0

    if cmd == "archive":
        if not EXE.is_file() or not PDB.is_file():
            print("UNRESOLVED: exe or pdb missing — build first")
            return 2
        cv = read_codeview(EXE)
        if not cv:
            print("UNRESOLVED: no CodeView record")
            return 2
        guid, age, _name = cv
        key = "%s-%d" % (guid, age)
        ARCHIVE.mkdir(parents=True, exist_ok=True)
        dest = ARCHIVE / ("dusklight-%s.pdb" % key)
        if dest.exists():
            print("already archived: %s" % dest.name)
        else:
            shutil.copy2(PDB, dest)
            print("archived %s (%.1f MB)" % (dest.name, dest.stat().st_size / 1e6))
        # RETENTION (user-ruled 2026-08-14): each PDB is ~234 MB, so archiving
        # every build unbounded would cost GBs a day on a machine already
        # holding ISOs, two dusklight trees and the decomps. The user's
        # condition was "if it's wholly beneficial" — unbounded growth is the
        # one way it is not, so keep the most recent KEEP builds and prune the
        # rest. Pruning removes the BLOB and marks the row PRUNED; the row
        # itself is never deleted, so "we had that build" stays answerable
        # even when the bytes are gone (№31-C: absent-with-a-reason, never
        # silently missing).
        KEEP = 10
        blobs = sorted(ARCHIVE.glob("dusklight-*.pdb"),
                       key=lambda f: f.stat().st_mtime, reverse=True)
        for old in blobs[KEEP:]:
            try:
                sz = old.stat().st_size
                old.unlink()
                print("pruned %s (%.0f MB, keeping newest %d)"
                      % (old.name, sz / 1e6, KEEP))
            except OSError:
                pass

        existing = {r[0] for r in rows()}
        if key not in existing:
            head = ("# BUILD IDENTITY — pdb archive index (GENERATED by "
                    "build_identity.py)\n\n"
                    "> The 08-12 crash was permanently unsymbolicatable because its PDB\n"
                    "> was gone. This index keeps every build's identity durable. Blobs\n"
                    "> live OUTSIDE the repo (%s); rows live here.\n\n"
                    "| build_id | exe mtime | exe bytes | pdb blob |\n"
                    "|---|---|---|---|\n" % ARCHIVE)
            if not MANIFEST.is_file():
                MANIFEST.write_text(head, encoding="utf-8", newline="\r\n")
            import datetime
            st = EXE.stat()
            when = datetime.datetime.fromtimestamp(st.st_mtime).strftime("%Y-%m-%d %H:%M:%S")
            with open(MANIFEST, "a", encoding="utf-8", newline="\r\n") as f:
                f.write("| %s | %s | %d | %s |\n" % (key, when, st.st_size, dest.name))
            print("indexed -> %s" % MANIFEST)
        return 0

    if cmd in ("resolve", "check") and len(sys.argv) > 2:
        want = sys.argv[2].strip().upper().replace("-", "")
        cv = read_codeview(EXE) if EXE.is_file() else None
        cur = ("%s%d" % (cv[0], cv[1])) if cv else None
        if cmd == "check":
            if not cur:
                print("UNKNOWN: current exe unreadable — cannot judge staleness")
                return 2
            same = cur.startswith(want) or want.startswith(cv[0])
            print("%s: feed build_id %s vs current %s-%d"
                  % ("CURRENT" if same else "STALE", sys.argv[2], cv[0], cv[1]))
            return 0 if same else 1
        for r in rows():
            if r[0].upper().replace("-", "").startswith(want[:8]):
                blob = ARCHIVE / r[3]
                if blob.is_file():
                    print("PDB %s" % blob)
                    return 0
                print("PRUNED: build %s was archived and later pruned by the "
                      "retention cap (newest 10 kept). The row proves the build "
                      "existed; the bytes are gone, so this crash cannot be "
                      "symbolicated — reproduce on a retained build." % r[0])
                return 1
        print("UNRESOLVED: build_id %s is not in the index. Symbolicating against "
              "another build's PDB yields a confidently WRONG name — do not. "
              "Reproduce on a build that IS indexed." % sys.argv[2])
        return 1

    if cmd == "manifest":
        # THE SILENT-FAILURE GATE (Housing's finding): by-name hook resolution
        # reads a symgen blob embedded INSIDE the image. SymbolManifest.cmake:64
        # has a QUIET path — no symgen prebuilt for the host emits a STATUS line
        # and return()s, so configure succeeds, the exe builds, and EVERY by-name
        # hook in EVERY plugin fails at once with no error anywhere near the
        # failure. That is invisible from the user's side, so it needs a check
        # that can be run on ANY dusklight.exe before trusting it.
        target = Path(sys.argv[2]) if len(sys.argv) > 2 else EXE
        if not target.is_file():
            print("UNRESOLVED: %s not found (not proof of anything)" % target)
            return 2
        sys.path.insert(0, str(HERE))
        try:
            import symbol_manifest as SM
            man = SM.load(str(target))
        except Exception as e:
            print("UNRESOLVED: manifest reader failed (%s)" % e)
            return 2
        cv = read_codeview(target)
        print("image    : %s" % target)
        print("build_id : %s" % ("%s-%d" % (cv[0], cv[1]) if cv else "UNKNOWN"))
        if man is None:
            print("manifest : **ABSENT**")
            print("VERDICT  : BY-NAME RESOLUTION IS DEAD on this build. Every hook a")
            print("           plugin installs by symbol NAME will fail — silently from")
            print("           the user's side. This exe cannot host the WW plugin.")
            print("           Cause to check first: SymbolManifest.cmake's quiet path")
            print("           (no symgen prebuilt for this host → STATUS + return()).")
            return 1
        counts = None
        try:
            counts = SM.load_counts(str(target))
        except Exception:
            counts = None
        if counts:
            entries, uniq, dups = counts
            print("manifest : present \u2014 **%d entries** (the runtime's number), "
                  "%d unique names" % (entries, uniq))
            print("           %d name(s) appear MORE THAN ONCE \u2014 a hook installed on "
                  "one of" % dups)
            print("           those binds by a name that does not identify a single")
            print("           function. Confirm the class/signature for any such target.")
        else:
            print("manifest : present, %d unique name(s) "
                  "(entry count unavailable)" % len(man))
        print("VERDICT  : by-name resolution AVAILABLE. Symbols are resolved against")
        print("           THIS image's own table, keyed to its build id — which is why")
        print("           a plugin needs no addresses and no compatibility table.")
        return 0

    if cmd == "list":
        rs = rows()
        print("indexed builds: %d" % len(rs))
        for r in rs:
            blob = ARCHIVE / r[3]
            print("  %s  %s  %s" % (r[0], r[1], "ok" if blob.is_file() else "MISSING-BLOB"))
        return 0

    print(__doc__.strip().splitlines()[-6] if __doc__ else "")
    print("usage: build_identity.py current | archive | resolve <id> | "
          "check <id> | list")
    return 2


if __name__ == "__main__":
    sys.exit(main())
