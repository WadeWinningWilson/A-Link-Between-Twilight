import os
from pathlib import Path

d = Path(os.environ["APPDATA"]) / (
    "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration/audio/ww_jaudio1"
)
print("dir", d)
for p in sorted(d.glob("seq_events*.csv")):
    print(p.name, p.stat().st_size)

bridge = d / "seq_events_i_link.csv"
print("--- bridge header + set_param samples ---")
if bridge.exists():
    with bridge.open(encoding="utf-8", errors="replace") as f:
        print(next(f).rstrip())
        n = 0
        for line in f:
            if ",set_param," in line:
                print(line.rstrip())
                n += 1
                if n >= 6:
                    break
else:
    print("missing bridge golden")
