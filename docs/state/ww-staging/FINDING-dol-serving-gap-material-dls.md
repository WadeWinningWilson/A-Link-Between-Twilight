# FINDING — 9 vegetation assets are NOT servable by content match; 7 are material display lists

era: era-2 (Outset served)
<!-- era rationale: blocks part of the vegetation port for the currently served set | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Found by:** checking the
DRAW PATH against the served table, on History/Bridge's instruction to read the
consumer rather than the asset headers.

## The correction this makes to my own earlier claim

I reported **"all 39 blobs located, 0 misses, 27.1 KB"**. That is true about
what the tool SEARCHED, and it is not the whole picture. The same run printed
`skipped 9 (non-byte or tiny tables)` — a benign-sounding count I did not
investigate. **Two of those 9 are `l_matDL` and `l_Vmori_matDL`, material
display lists the grass draw path calls EVERY FRAME.**

A summary that hides a required asset behind a reassuring word is the same
defect as a gate whose silence is only as wide as its scope — the third
instance of that shape today. The tool now NAMES every skip.

## What is actually missing

Nine assets, and the split matters:

**Seven macro-built material display lists** — headers contain
`LOAD_BP_REG(...)` macro INVOCATIONS, not bytes:

| symbol | consumer |
|---|---|
| `l_matDL__d_grass` (`l_matDL`) | `mpMatDL`, `mMatDLSize = 0xA0` |
| `l_Vmori_matDL` | `mpMatDL` for the Vmori grass variant |
| `l_matDL__d_flower`, `l_matDL2`, `l_matDL3` | flower material setup |
| `l_matDL__d_tree` | tree material setup |
| `g_dTree_shadowMatDL` | tree shadow pass |

**Two sub-16-byte colour tables**, below the tool's search floor:
`l_color3__d_flower::l_color3` (8 B) and `l_color__d_flower::l_color` (8 B).
Real data; the floor exists because an 8-byte pattern occurs everywhere in a
3.8 MB image and a match would not be evidence.

## Why content matching cannot fix this

The method's whole basis is that the decomp's asset headers carry the byte
VALUES, so the bytes are their own search key. **For these seven, the header
carries macro calls that only the COMPILER turns into bytes.** There is nothing
to search for. This is a limit of the method, not a bug in the run.

## What I tried, and it failed

A GX material display list should be a run of BP register writes, so I scanned
`0x375000`–`0x376400` (around the located Vmori cluster) for `0xA0`-byte windows
with `0x61` appearing at a strict 5-byte stride, 28+ of 32 slots.

**Zero candidates.** So the encoding is not what I assumed. I am recording that
as a FAILED ATTEMPT rather than concluding the data is absent — the blobs are
certainly in the image, since the retail game draws grass with them.

## What this changes for the port

**HISTORY/BRIDGE:** the serving layer can supply the geometry, texcoords,
colours, textures and per-instance display lists — including `l_Oba_kusa_aDL`
AND `l_Oba_kusa_a_cutDL`, so the cut-grass hazard you named is covered. **It
cannot yet supply `mpMatDL` for any of the three managers.** Do not design
around it being there; a manager that draws with no material setup is exactly
the "draws but wrong" state you said the port must not leave open.

**Scope note:** the tree half is otherwise fully served — `l_Txa_swood_aTEX`,
`l_Oba_swood_a_*DL` and `g_dTree_Oba_kage_32DL` are all in the located table.
Only `l_matDL__d_tree` and `g_dTree_shadowMatDL` are open.

## Options, none yet taken

1. **Expand the macros offline** — resolve `LOAD_BP_REG` and the `GX_BP_REG_*`
   values, generate the bytes, then content-match as before. Most faithful; needs
   the macro definition, which is not under `include/` in the decomp.
2. **Locate by adjacency** — the matDLs sit inside the same object files as
   blobs already located, so their addresses are bracketed. Needs a structural
   signature to confirm a candidate, and my first guess at that signature failed.
3. **Ask the port what it actually needs** — if the receiver's own material
   setup can stand in at the consumption boundary, the donor's matDL may not
   need serving at all. That is a DN-10 question, not a serving question, and
   it is not mine to answer alone.

## Reproduce

```bash
python tools/foundry/ww_dol_locate.py "D:/XXXXXXX/Ex WW/sys/main.dol" --vegetation
```
The `SKIPPED, NAMED` block at the end is the gap.
