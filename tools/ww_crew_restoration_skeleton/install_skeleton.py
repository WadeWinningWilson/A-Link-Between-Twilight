#!/usr/bin/env python3
"""Install the WW-Crew-Restoration skeleton into the user's model_replacements/ folder.

Phase M4/A4: the game exe never writes mod content. Run this script (or copy manually).

Usage:
  python tools/ww_crew_restoration_skeleton/install_skeleton.py
"""
from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path


def main() -> int:
    here = Path(__file__).resolve().parent
    if not (here / "modinfo.ini").is_file():
        print("error: modinfo.ini missing next to this script", file=sys.stderr)
        return 1

    appdata = os.environ.get("APPDATA")
    if not appdata:
        print("error: APPDATA not set", file=sys.stderr)
        return 1

    dest = Path(appdata) / "TwilitRealm" / "Dusklight" / "model_replacements" / "WW-Crew-Restoration"
    dest.mkdir(parents=True, exist_ok=True)
    # Copy tree; never overwrite user arcs/.
    for src in here.rglob("*"):
        if src.is_dir():
            continue
        # ====================================================================
        # Integrator ruling (quarantine-escape follow-up): the mod folder needs
        # mod CONTENT; it has never needed tooling. NO .py ships — a kit copy
        # living beside the assets it bakes is the delivery mechanism that
        # planted adapt_bdl_arcs in the live overlay, invisible to every gate.
        # ====================================================================
        if src.suffix == ".py" or src.name.endswith(".pyc"):
            continue
        if src.name == "install_skeleton.py":
            continue
        rel = src.relative_to(here)
        out = dest / rel
        if rel.parts[0] == "arcs" and out.is_file():
            continue  # keep user's arcs
        out.parent.mkdir(parents=True, exist_ok=True)
        if out.is_file() and out.name.endswith(".ini"):
            # Refresh manifests; user can revert from git/mod backup if needed.
            shutil.copy2(src, out)
        elif not out.exists():
            shutil.copy2(src, out)
    print(f"installed skeleton -> {dest}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
