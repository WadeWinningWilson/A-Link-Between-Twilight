# WW Bridge Tool — live state

**Tip (overwrite, don’t append):** ASK 17 engine event dump shipped — set `DUSK_EXTSEQ_EVENT_DUMP=1`, enter Outset/house once; CSVs land next to Bridge goldens. **Diff streams; no fix until disagreement is a line.**

| Field | Value |
|-------|--------|
| **status** | Both streams ready for mechanical diff |
| **next** | Diff `seq_events_*.csv` vs `seq_events_engine_*.csv`; fix the first disagreement |
| **do-not** | Change rates/exponents; share decoder with Bridge; fix from theory |
| **tool_root** | `C:\Users\xxxxx\Documents\albt bridge` |
| **tool_version** | **0.18.0** |
| **decomp** | `D:\XXXXXXX\WW DP` (`WW_DECOMP`) |
| **ww_extract** | `D:\XXXXXXX\Ex WW` (`WW_EXTRACT`) |
| **schema_version** | **2** |
| **updated** | 2026-07-21 |

## Commands (recent)

| Cmd | Role |
|-----|------|
| `seq-events` | **ASK 17** independent BMS event decode (Bridge golden) |
| `tempo-map` | ASK 16 tempo/timebase |
| `vel-calib` | ASK 15 velocity hist + initVol |

## Headline (ASK 17)

| Side | Artifact |
|------|----------|
| Bridge | `seq_events_{i_link,house}.csv` |
| Engine | `seq_events_engine_{i_link,house}.csv` (flag `DUSK_EXTSEQ_EVENT_DUMP=1`) |

Columns: `tick,track_id,event,note_param,velocity` · per-track local ticks

## Open

| Item | Owner |
|------|--------|
| Diff Bridge vs engine → first disagreeing line | user / HS |
| Fix that line only | engine (after diff) |

## Closed this lane

ASK 1–17 · §52 (B) · §53b · engine dump companion · …
