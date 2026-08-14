#!/usr/bin/env python3
# ===========================================================================
# kit_output_law.py — THE OUTPUT LAW (integrator kit-audit, user-assigned).
#
# THE GAP IT CLOSES: kit_laws lints C++ TUs, covenant_gate lints the exe —
# NOTHING linted what a kit WRITES. A kit could emit a baked donor arc and no
# instrument objected. This helper makes the bake STRUCTURALLY IMPOSSIBLE
# instead of merely forbidden:
#
#   A kit may write a donor-format file ONLY IF
#     (a) the bytes are BYTE-IDENTICAL to the disc original (hash vs ww_disc), or
#     (b) the write is DECLARED — reason + strip trigger — and the declaration
#         is LOGGED to the output ledger where the §723 guard rules can see it.
#   Anything else refuses, loudly, before the write.
#
# Kits adopt it by replacing raw open/copy with lawful_write()/lawful_copy().
# The retooled kits (actor_kit, enemy_port_kit skeleton emitter) route here;
# a kit that bypasses it is findable by grep (raw .arc writes = the kit_laws
# tools-law that pairs with this helper).
#
# Usage as a tool:  kit_output_law.py --selftest
# ===========================================================================
import hashlib
import io
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ww_disc

LEDGER = (Path(__file__).resolve().parents[2] / "docs" / "state" / "ww-staging"
          / "OUTPUT-LEDGER.md")

_DONOR_MAGIC = (b"Yaz0", b"RARC")


class OutputLawViolation(RuntimeError):
    pass


def _disc_index():
    """name -> (offset, length) for the configured WW image; {} if none."""
    if not hasattr(_disc_index, "_c"):
        _disc_index._c = {}
        path = os.environ.get("WW_ISO", "")
        if not path:
            try:
                import json
                cfg = os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight\config.json")
                path = json.load(open(cfg, encoding="utf-8")).get("backend.extraIsoPath", "")
            except OSError:
                path = ""
        if path and Path(path).is_file():
            try:
                f, boot = ww_disc.iso_open(path, ww_disc.WW_IDS)
                _disc_index._f = f
                for p, off, ln in ww_disc.fst_walk(f, boot):
                    _disc_index._c.setdefault(p.rsplit("/", 1)[-1], []).append((p, off, ln))
            except (OSError, ValueError):
                pass
    return _disc_index._c


def _matches_disc(name, data):
    """True if data is byte-identical to A disc file of the same name."""
    for p, off, ln in _disc_index().get(name, []):
        if ln == len(data):
            _disc_index._f.seek(off)
            if hashlib.md5(_disc_index._f.read(ln)).digest() == hashlib.md5(data).digest():
                return p
    return None


def _declare(dest, reason, strip_trigger):
    if not LEDGER.parent.is_dir():
        LEDGER.parent.mkdir(parents=True, exist_ok=True)
    new = not LEDGER.is_file()
    with io.open(LEDGER, "a", encoding="utf-8", newline="\r\n") as f:
        if new:
            f.write("# Kit output ledger — every DECLARED non-verbatim donor-format "
                    "write, with its strip trigger (§723 shape)\n"
                    "| dest | reason | strip trigger |\n|---|---|---|\n")
        f.write("| %s | %s | %s |\n" % (dest, reason, strip_trigger))


def lawful_write(dest, data, reason=None, strip_trigger=None, kit="?"):
    """The single gate. Non-donor-format bytes pass freely; donor-format bytes
    must be disc-verbatim or declared."""
    dest = Path(dest)
    if data[:4] in _DONOR_MAGIC or dest.suffix.lower() == ".arc":
        hit = _matches_disc(dest.name, bytes(data))
        if hit:
            print("[OUTPUT-LAW] %s: VERBATIM vs disc %s — lawful" % (dest.name, hit))
        elif reason and strip_trigger:
            _declare(str(dest), reason, strip_trigger)
            print("[OUTPUT-LAW] %s: DECLARED (%s; strips when: %s) — ledgered"
                  % (dest.name, reason, strip_trigger))
        else:
            raise OutputLawViolation(
                "%s refuses to write %s: donor-format bytes that are neither "
                "disc-verbatim nor declared(reason+strip_trigger). The bake is "
                "structurally impossible; declare it or serve it from the disc."
                % (kit, dest))
    dest.parent.mkdir(parents=True, exist_ok=True)
    with open(dest, "wb") as f:
        f.write(data)
    return dest


def lawful_copy(src, dest, reason=None, strip_trigger=None, kit="?"):
    return lawful_write(dest, Path(src).read_bytes(), reason, strip_trigger, kit)


def selftest():
    import tempfile
    d = Path(tempfile.mkdtemp())
    ok = True
    # 1: non-donor bytes pass freely
    lawful_write(d / "notes.txt", b"hello", kit="selftest")
    # 2: donor-format, undeclared, not on disc -> REFUSED
    try:
        lawful_write(d / "fake.arc", b"RARC" + b"\0" * 64, kit="selftest")
        ok = False
        print("  FAIL: undeclared donor-format write was allowed")
    except OutputLawViolation:
        print("  refusal path: OK")
    # 3: declared -> ledgered + written
    global LEDGER
    old = LEDGER
    LEDGER = d / "ledger.md"
    lawful_write(d / "declared.arc", b"RARC" + b"\0" * 64,
                 reason="selftest bridge", strip_trigger="never (test)", kit="selftest")
    ok = ok and (d / "declared.arc").is_file() and "selftest bridge" in io.open(LEDGER, encoding="utf-8").read()
    LEDGER = old
    # 4: disc-verbatim (if an image is configured): pull a real file and rewrite it
    idx = _disc_index()
    if idx:
        name = "stage.dzs"  # not top-level; pick a real arc instead
        for nm, lst in idx.items():
            if nm.endswith(".arc") and lst[0][2] < 300000:
                p, off, ln = lst[0]
                _disc_index._f.seek(off)
                data = _disc_index._f.read(ln)
                lawful_write(d / nm, data, kit="selftest")
                print("  verbatim path: OK (%s)" % nm)
                break
    else:
        print("  verbatim path: SKIPPED (no image configured)")
    print("selftest:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(selftest() if "--selftest" in sys.argv else
                     print(__doc__ or "see header") or 2)
