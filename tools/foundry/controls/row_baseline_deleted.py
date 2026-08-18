#!/usr/bin/env python3
# ============================================================================
# CONTROL FIXTURE — row_baseline must FAIL when a baselined row vanishes.
#
# The input whose PASS proves the gate is blind: a store pinned with two rows,
# one then DELETED with no retirement entry. If row_baseline reports PASS on
# this, its passes mean nothing and the deletion bypass is open.
#
# Self-contained: builds the store in a temp dir, never touches the live one.
# ============================================================================
import importlib.util
import json
import sys
import tempfile
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("rb", HERE.parent / "row_baseline.py")
rb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(rb)

d = Path(tempfile.mkdtemp())
rows = d / "rows"
rows.mkdir()
for rid in ("sc0000001", "sc0000002"):
    (rows / ("%s.md" % rid)).write_text("id: %s\nsymbols: control\n" % rid,
                                        encoding="utf-8", newline="\n")
bp = d / "_baseline.json"
bp.write_text(json.dumps({"pinned_head": "control", "rows": rb.row_set(rows)}) + "\n",
              encoding="utf-8", newline="\n")

# THE DELETION — no retirement entry anywhere
(rows / "sc0000001.md").unlink()
rb.RETIRED = d / "RETIRED.md"          # deliberately absent

rc, vanished = rb.check(rows, bp, verbose=False)
if rc == 1 and "sc0000001" in vanished:
    print("CONTROL OK — row_baseline DETECTED the unretired deletion (rc=1, %s)" % vanished)
    sys.exit(1)          # non-zero == the gate fired, which is what control.py wants
print("**CONTROL DID NOT FIRE** — row_baseline PASSED a store with a deleted, "
      "unretired row. Its passes prove nothing.")
sys.exit(0)
