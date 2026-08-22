# FINDING — the WW vegetation assets live in the EXECUTABLE, not an arc (27.1 KB) — located in the user's own DOL, SOLVED

era: era-2 (Outset served)
<!-- era rationale: blocks the vegetation port for the currently served set | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Raised BEFORE the port, not after**

> ## ✅ SOLVED SAME DAY — and one of my own numbers was wrong
>
> **ALL 39 SEARCHABLE BLOBS LOCATED IN THE USER'S OWN RETAIL `main.dol`, 0 MISSES.**
> Tool: `tools/foundry/ww_dol_locate.py`. Offsets: `ww-vegetation-dol-offsets.txt`.
> The whole vegetation asset set sits in one span, `0x374C00`–`0x380B80`.
>
> **CORRECTION TO THIS DOCUMENT'S OWN HEADLINE: the "186 KB" below is the size of
> the C SOURCE (hex text), not of the data. The actual binary payload is 27,780
> bytes — 27.1 KB.** I inflated it roughly 9x by measuring the decomp's source
> files instead of the bytes they encode. The blocker was real; its size was not.
>
> **THE DEBUG→RETAIL ADDRESS MAPPING TURNED OUT TO BE UNNECESSARY.** The section
> below calls that mapping "the actual work" — it was not. The decomp's asset
> headers carry the byte VALUES, so the bytes are their own search key: match the
> content, get the retail offset, and the match verifies itself. No symbol table,
> no relocation arithmetic.
>
> **BONUS, unlooked-for but load-bearing for the port: every blob matched even
> though the decomp is built from `D44J01` (JP) and the user's disc is USA.** These
> assets are byte-identical across those builds. That is measured, not assumed —
> 39/39 exact matches of whole blobs.
>
> **WHAT A HIT DOES NOT PROVE, and the tool says so in its own header:** that the
> runtime consumer reads from that offset. The DOL is relocated at load; serving a
> table means handing the ported manager the BYTES, exactly as arc members are
> served today. That is the remaining build work, and it is straightforward.

## Why this exists

The vegetation port was just handed to History
([FINDING-outset-vegetation-is-TP-grass.md](FINDING-outset-vegetation-is-TP-grass.md)).
Before anyone writes it: **the donor's vegetation assets are not in an arc.**
They are static C arrays compiled into the game executable, and copying them
into plugin source would violate the zero-bake directive on the first commit.

This is the housing/serving lane's problem to solve, not History's to discover.

## What was measured

The three donor managers — `d_grass.cpp`, `d_flower.cpp`, `d_tree.cpp` — pull in
**48 asset headers**, all resolved, none missing:

| kind | bytes of C source |
|---|---:|
| textures (`*TEX.h`) | 108,373 |
| display lists (`*DL.h`) | 38,752 |
| geometry / colour / texcoord | 43,444 |
| **TOTAL (C source)** | **190,569 (186.1 KB)** |

⚠️ **Those are C-SOURCE sizes, not data sizes.** The binary payload the port
actually needs is **27,780 bytes (27.1 KB)** — see the banner. Quoting the
source size made the problem look ~9x bigger than it is.

Examples, verbatim from the donor source:

- `d_grass.cpp` — `l_K_kusa_00TEX.h` (26 KB), `l_Txa_ob_kusa_aTEX.h` (13 KB),
  `l_Oba_kusa_aDL.h`, `l_Vmori_texCoord.h`, and `mpTexCoordArr = l_Vmori_texCoord;`
- `d_flower.cpp` — `l_Txq_bessou_hanaTEX.h` (26 KB),
  `l_Txo_ob_flower_white_64x64TEX.h` and `..._pink_...` (13 KB each),
  `l_QbsafDL.h` / `l_QbsfwDL.h`, plus `l_pos__`, `l_color__`, `l_texCoord__`
- `d_tree.cpp` — `l_Txa_swood_aTEX.h`, `l_Oba_swood_a_*DL.h`, shadow mats

**These are textures, display lists and vertex data — content, not code.**

## Why it is a problem

The zero-bake directive is explicit that the port ships **no donor bytes at
all**, with every donor→receiver difference translated at the consumption
boundary in receiver code. Pasting `l_Txq_bessou_hanaTEX.h` into plugin source
ships donor texture as a C array (4,096 bytes of data, 26 KB of source text).
That the bytes arrive as source rather than as a file does not change what they
are — and it is 27.1 KB of data across 39 blobs.

The lwood precedent is the standard to match: its geometry is served from the
user's own `Lwood.arc`, verified byte-identical against the disc. Vegetation
must reach the same bar by a different route, because its bytes are in a
different container.

## The route, and it is viable — checked, not assumed

**The data is DOL-resident and the DOL is on the user's disc.**

- `/d/Decomps/Ex WW/sys/main.dol` — **3,822,272 bytes**, present.
- The debug linker maps carry these symbols with addresses AND sizes.
  From `frameworkD.map`:

```
00008580 001000 805a82e0 32 l_K_kusa_00TEX        d_grass.o
00016760 001000 805b64c0 32 l_Txq_bessou_hanaTEX  d_flower.o
```

Section offset, **size 0x1000**, load address, alignment, symbol, object file.
Every symbol spot-checked (`l_K_kusa_00TEX`, `l_Txq_bessou_hanaTEX`,
`l_Txo_ob_flower_white_64x64TEX`, `l_QbsfwDL`, `l_Vmori_texCoord`) is present.

So the shape of a zero-bake-compliant port is: **resolve symbol → address →
read the bytes out of the user's own `main.dol` at runtime**, exactly as the
plugin already serves arc members from the user's disc.

✅ **THIS ROUTE WAS SUPERSEDED BY A SIMPLER ONE — see the banner.** The debug
addresses do not transfer to retail, and mapping them turned out to be
unnecessary: the decomp's asset headers carry the byte VALUES, so exact content
matching finds the retail offsets directly and verifies itself. The debug map
remains useful for CONFIRMING sizes (`l_K_kusa_00TEX` is 0x1000, and the located
blob is 4,096 bytes — they agree).

## What this changes for the port

**HISTORY — do not paste the `assets/*.h` headers into plugin source.** The
code port (`d_grass` / `d_flower` / `d_tree` managers + the `d_a_grass`
spawner) is unaffected and can proceed; only the DATA needs the serving route.

**HOUSING/ENGINE — the serving route is mine.** Extracting DOL-resident tables
from the user's disc is the same job as serving arc members, and it is now on
this lane's plate.

## What is still open

Only the build work: hand the ported managers their bytes at load, reading from
the user's `main.dol` at the recorded offsets, exactly as arc members are served
today. Nothing about the location or the identity of the data is unknown any
more — 39/39 blobs matched exactly, 0 misses.

The one thing a content match does NOT establish is that the runtime consumer
reads from that file offset; the DOL is relocated at load. Serving hands over
the BYTES, so that distinction does not block the port — but it would block any
scheme that tried to point the game at its own DOL image.
