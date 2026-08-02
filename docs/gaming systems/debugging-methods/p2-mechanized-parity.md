# P2 — Mechanized parity (probe schema + differ)

> Foundry, 2026-07-29 (bus §206). Turns "does the port behave like vanilla?" from an
> eyeball judgment into a computed verdict. The BGM note differ
> (`tools/extseq_note_differ.py`, state-file §P2-audio) is the specialized exemplar;
> this is the general layer.

## The schema (probe-event JSONL v0)

One event per probe hit, donor (DuskTap) or receiver (DuskLog) alike:

```json
{"t":"MM:SS:mmm","t_s":9196.6,"site":"emitter_create","addr":"8025f0e4",
 "regs":["...r3..r12..."],"lr":"...","derefs":[{"reg":"r5","addr":"...","words":[...]}],
 "key":["id","00000031"]}
```

`key` is the semantic identity the differ aligns on (emitter/SE resource id, actor
proc id, placement name decoded from deref ascii); `regs`/`derefs` keep full depth.
`t_s` is monotonic (hour-wrap unwrapped — Dolphin logs carry no hour and append
across launches).

## Tools (run with the Foundry 3.12 venv python)

- `tools/foundry/dusktap_to_jsonl.py <dolphin.log> <out.jsonl>` — converter; knows the
  11-address tap roster; prints a per-site census (cross-check it against expectations).
- `tools/foundry/probe_differ.py <mode> ...` — the verdict tool:
  - `census A B --site S [--a-window S-E --b-window S-E] [--tol 0.25]` — per-(site,key)
    counts + rates; row verdicts MATCH / DRIFT(x%) / A-ONLY / B-ONLY; overall verdict.
    **Empty stream ⇒ UNKNOWN, never MATCH** (negative-control rule).
  - `seq A B --site S [windows]` — difflib alignment on key streams; similarity % +
    first divergence.
  - `profile A --site S [--key K] [--bucket 30]` — single-stream rate histogram.

## Calibration (2026-07-29, existing corpus — no new capture needed)

- **Converter validated**: reproduces the §196 hand censuses exactly (emitter 694,
  se_start 21,548) from the raw 14 MB log.
- **Census known-good**: two *independent* WW boots (user's F1 test vs the unattended
  recording run), same 13-s window: dominant boot SE `0x303D` at 29.1/s vs 30.0/s —
  **MATCH at 3% relative**; 1–2-event rows correctly held below significance.
- **Seq caveat (honest limit)**: sequence mode demands *scenario-identical,
  phase-aligned* windows — two boots that reached different screens legitimately
  diverge (69–79% here). Its home is record-vs-replay of one reel and P3 golden
  traces of one scripted scene, not cross-scenario comparison.

## First donor-law artifact

**Windline 0x31 rate profile** (Outset play, 906 s):
sustained ~50 per 30 s (≈1.7/s) across active exterior play, peak 2.1/s, near-zero
indoors; full histogram in
[../../WW Linked/windline-donor-profile.md](../../WW%20Linked/windline-donor-profile.md).
The receiver half needs Engine's ONE log line (state-file F2 spec); then
`probe_differ census` issues the first computed vanilla-law verdict on W-LINE density.

## Verdict semantics

MATCH = rates agree within tolerance on all significant rows. DRIFT names the rows.
UNKNOWN = insufficient data — a differ that can't see is required to say so.
Housing is invited to negative-control all three modes (feed empty/truncated inputs).
