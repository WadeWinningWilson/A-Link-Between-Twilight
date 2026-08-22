# FINDING — 7 material DLs are CORRECTLY EXCLUDED (they embed a texture address); only 2 small tables are a real gap

era: era-2 (Outset served)
<!-- era rationale: blocks part of the vegetation port for the currently served set | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Found by:** checking the
DRAW PATH against the served table, on History/Bridge's instruction to read the
consumer rather than the asset headers.

> ## 🔄 REVERSED 2026-08-22, SAME DAY — READ THIS FIRST. THE PREMISE BELOW IS WRONG.
>
> History/Bridge answered the open question and it INVERTS this document:
> **`l_matDL` is not a blob at all.** It is a MACRO PARAMETERISED BY A TEXTURE
> SYMBOL — `l_matDL__d_flower(l_Txo_ob_flower_white_64x64TEX)` — expanding to a GX
> register-load list that embeds
> `LOAD_BP_REG(GX_BP_REG_SETIMAGE3_TEX0, IMAGE_ADDR(TEX_NAME))`, where
> `IMAGE_ADDR(a) = (u32)(a) >> 5`. **The material DL's bytes carry the texture's
> own ADDRESS.**
>
> **SO THE SEVEN ARE NOT A COVERAGE GAP — THEIR ABSENCE IS THE METHOD WORKING.**
> Those bytes are RELOCATION-DEPENDENT, so they are not stable content and not a
> legitimate search key. A locator built on "the bytes are their own search key"
> must not match them, and mine did not.
>
> **AND SERVING THEM WOULD HAVE BEEN ACTIVELY WRONG — worse than missing.** It
> would hand our renderer a GameCube texture address from the donor's address
> space, pointing at nothing in ours: a GX register loaded with a garbage image
> base, presenting as corrupt or black vegetation — *the exact symptom this port
> exists to fix.*
>
> ⚠️ **I ROUTED OPTION ① (expand the macros offline) TO DECODER. THAT REQUEST IS
> WITHDRAWN — it would have produced bytes that are wrong by construction.**
>
> **WHAT REMAINS A REAL GAP: only the two 8-byte flower colour tables.** Seven of
> the nine are correctly excluded. My framing of "a limit of the method" was
> backwards; it was the method refusing to serve something unstable.

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
