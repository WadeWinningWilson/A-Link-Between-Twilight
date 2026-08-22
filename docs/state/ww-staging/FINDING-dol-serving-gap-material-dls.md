# FINDING — the material DLs ARE servable (address is 0 on disc, patched at runtime); everything else located

era: era-2 (Outset served)
<!-- era rationale: blocks part of the vegetation port for the currently served set | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Found by:** checking the
DRAW PATH against the served table, on History/Bridge's instruction to read the
consumer rather than the asset headers.

> ## ⚠️ THIS DOCUMENT HAS TURNED OVER TWICE. THE THIRD ACCOUNT IS THE MEASURED ONE.
>
> **v1 (mine):** the 7 material DLs are a coverage gap my method could not reach.
> **v2 (History/Bridge, and I adopted it):** they bake a texture ADDRESS, so they
> are relocation-dependent, unserveable, and their absence is the method working.
> **v3 (History/Bridge, measured, and I verified it myself): THE ADDRESS IS NOT
> BAKED. It is ZERO on disc and filled in at runtime.**
>
> They generated the 136 command bytes from the donor macro and searched the
> user's DOL with only the 3-byte SETIMAGE3 value wildcarded: **three exact
> matches, 133 of 136 bytes identical, SETIMAGE3 = `0x000000` at all three.**
>
> I verified at their offsets rather than taking it on report. All three read
> `61 80 00 01 3a | 61 84 00 00 00 | 61 88 e0 fc 3f | 61 94 00 00 00 …` — BP
> writes at a 5-byte stride — with bytes[17:20] = `000000`.
>
> | site | offset |
> |---|---|
> | `d_grass` `l_matDL` | `dol+0x376AE0` |
> | `d_flower` | `dol+0x380CE0` |
> | `d_flower` | `dol+0x380DA0` |
>
> **SO THE BYTES ARE STABLE CONTENT AND A LEGITIMATE SEARCH KEY, AND THEY CAN BE
> SERVED** — the consumer writes the real texture address into bytes [17:20]
> before use. What survives from v2 is only the practical half: the list cannot
> be used AS-IS, because SETIMAGE3 = 0 is an image base of 0. Still wrong, but a
> different wrong.
>
> **TWO ERRORS OF MINE, and the first is the serious one:**
>
> **① I ADOPTED v2 WITHOUT VERIFYING IT** — wrote it into this finding, filed a
> row on it, and withdrew a decode request because of it. My own standing note
> says *never repeat another lane's premise unverified; repetition feels like
> corroboration.* I did exactly that, and my repetition made a wrong mechanism
> look confirmed by two lanes.
>
> **② I MISDIAGNOSED MY OWN FAILED SCAN.** I scanned for `0x61` at a 5-byte
> stride and found nothing, and concluded "the encoding is not what I assumed."
> **The encoding was exactly what I assumed.** My window stopped at `0x376400`
> and the grass matDL is at `0x376AE0`, and my 28-of-32 threshold was too strict
> for a list that is not uniformly BP writes. A search that fails tells you about
> the search first.

## What my own coverage claim got wrong, and what it got right

I reported **"all 39 blobs located, 0 misses, 27.1 KB"** and the same run printed
`skipped 9 (non-byte or tiny tables)` — a benign-sounding count I did not
investigate. **Investigating it was right; my conclusion from it was wrong.**

The tool now NAMES every skip, and that change stands on its own merits: a count
that hides what it dropped is the same defect as a gate whose silence is only as
wide as its scope. But the thing it was hiding turned out to be **correct
behaviour**, not a gap.

## The nine, and why seven are not a gap

**Seven macro-built material display lists** — `l_matDL__d_grass`,
`l_Vmori_matDL`, `l_matDL__d_flower`, `l_matDL2`, `l_matDL3`, `l_matDL__d_tree`,
`g_dTree_shadowMatDL`.

These are not blobs. Each is a MACRO taking a texture symbol —
`l_matDL__d_flower(l_Txo_ob_flower_white_64x64TEX)` — that expands to a GX
register-load list embedding the texture's own address via
`IMAGE_ADDR(a) = (u32)(a) >> 5`, and bakes `TEX_NAME##__width` / `__height` as
literals beside it.

**They are relocation-dependent, so they are not stable content, so they are not
a legitimate search key.** A locator whose whole basis is "the bytes are their
own search key" MUST NOT match them. Their absence is the method holding its own
line — the same property that made 39/39 match exactly.

**Two sub-16-byte colour tables** — `l_color3__d_flower::l_color3` (8 B) and
`l_color__d_flower::l_color` (8 B). **These are the only real gap.** The floor
exists because an 8-byte pattern occurs everywhere in a 3.8 MB image and a match
would not be evidence; locating them needs an anchor, not a wider search.

## Serving the seven would have been worse than missing them

It would hand our renderer a GX display list carrying a GameCube texture address
from the DONOR's address space, pointing at nothing in ours — a register loaded
with a garbage image base. That does not present as "a texture failed to load";
it presents as corrupt or black vegetation, **the exact symptom this port exists
to fix**, which is how it would have cost a week.

## Options — ① WITHDRAWN, ② unnecessary, ③ is the answer

1. ~~Expand the macros offline and content-match the result.~~ **WITHDRAWN, and
   the request I routed to Decoder is withdrawn with it.** Expanding them
   faithfully still yields a donor-address-bearing list; the bytes would be wrong
   by construction no matter how correctly they were generated.
2. ~~Locate by adjacency.~~ Unnecessary — there is nothing legitimate to locate.
   (My structural-signature attempt found zero candidates, which now reads as
   consistent rather than as a failure.)
3. **The material DL must be BUILT at draw time against OUR texture object**, not
   served. That is a consumption-boundary translation, which is where zero-bake
   says donor/receiver differences belong. It is the port's call, not the serving
   layer's.

## What the serving layer promises

**Serves:** geometry (`mpPosArr`), colours (`mpColorArr`), texcoords
(`mpTexCoordArr`), every texture, and per-instance display lists — including
`l_Oba_kusa_a_cutDL` beside `l_Oba_kusa_aDL`, so cut grass gets its own geometry
rather than silently the wrong list.

**Does not serve, correctly:** `mpMatDL` for any manager. Not a hole to be
filled later — a thing that must not come from the donor's bytes at all.

## Reproduce

```bash
python tools/foundry/ww_dol_locate.py "D:/XXXXXXX/Ex WW/sys/main.dol" --vegetation
```
The `SKIPPED, NAMED` block lists all nine; only the two 8-byte tables are open.
