"""Foundry P14: donor lighting-palette artifacts (Pale/Virt tables per stage).

The donor's authored lighting law, machine-readable: every Pale palette's
actor/bg C0+K0 colors and every Virt sky set, straight from the fact-sheet
JSONs. EnvR/Colo (which palette WHEN) are unparsed selector tables —
[INFERENCE-NEEDED] pending decomp struct transcription (dStage EnvR/pselect).

Usage: palette_maps.py <fact-sheets-json-dir> <stage> <out.md>
"""
import sys, os, json, re

RGB = re.compile(r"r=(\d+), g=(\d+), b=(\d+)")

def swatch(v):
    m = RGB.search(v or "")
    return f"{m.group(1)},{m.group(2)},{m.group(3)}" if m else (v or "")

def main():
    json_dir, stage, out_md = sys.argv[1:4]
    data = json.load(open(os.path.join(json_dir, stage + ".json"), encoding="utf-8"))
    lines = [f"# Donor lighting palettes — stage `{stage}`", ""]
    for arc, rows in data.items():
        for r in rows:
            if r["chunk"] == "Pale" and r["entries"]:
                lines.append(f"## `{arc}` Pale — {len(r['entries'])} palettes")
                keys = [k for k in r["entries"][0]
                        if k not in ("name",) and "RGB" in str(r["entries"][0][k])]
                lines.append("| # | " + " | ".join(keys) + " |")
                lines.append("|---|" + "---|" * len(keys))
                for i, e in enumerate(r["entries"]):
                    lines.append(f"| {i} | " +
                                 " | ".join(swatch(e.get(k)) for k in keys) + " |")
            if r["chunk"] == "Virt" and r["entries"]:
                lines.append(f"\n## `{arc}` Virt — {len(r['entries'])} sky sets")
                keys = [k for k in r["entries"][0] if "color" in k]
                lines.append("| # | " + " | ".join(keys) + " |")
                lines.append("|---|" + "---|" * len(keys))
                for i, e in enumerate(r["entries"]):
                    lines.append(f"| {i} | " +
                                 " | ".join(swatch(str(e.get(k))) for k in keys) + " |")
    for arc, rows in data.items():
        for r in rows:
            if r["chunk"] == "EnvR" and r["entries"]:
                lines.append(f"\n## `{arc}` EnvR — {len(r['entries'])} environments "
                             "(8 weather slots → Colo/pselect index; d_stage.h:162)")
                lines.append("| env# | pselect ids (weather 0-7) |")
                lines.append("|---|---|")
                for i, e in enumerate(r["entries"]):
                    lines.append(f"| {i} | {e.get('pselect_id')} |")
            if r["chunk"] == "Colo" and r["entries"]:
                lines.append(f"\n## `{arc}` Colo — {len(r['entries'])} pselects "
                             "(8 time slots → Pale index + change_rate; d_stage.h:103)")
                lines.append("| pselect# | palette ids (time 0-7) | change_rate |")
                lines.append("|---|---|---|")
                for i, e in enumerate(r["entries"]):
                    lines.append(f"| {i} | {e.get('palette_id')} | "
                                 f"{e.get('change_rate')} |")
    lines.append("\n> Selection chain (donor law): EnvR[env][weather] → Colo/pselect"
                 " → palette_id[time slot] → Pale colors; time slots map to hours"
                 " via dKyd_Schedule (d_kankyo_data — engine-static, not stage data).")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {out_md}")

if __name__ == "__main__":
    main()
