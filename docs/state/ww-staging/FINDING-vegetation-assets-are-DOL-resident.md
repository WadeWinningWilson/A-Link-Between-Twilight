# FINDING — the WW vegetation port hits ZERO-BAKE: 186 KB of its assets live in the executable, not in any arc

era: era-2 (Outset served)
<!-- era rationale: blocks the vegetation port for the currently served set | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Raised BEFORE the port, not after**

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
| **TOTAL** | **190,569 (186.1 KB)** |

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
ships 26 KB of donor texture as a C array. That the bytes arrive as source
rather than as a file does not change what they are — and it is 186 KB of it.

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

⚠️ **NOT YET DONE, and the gap is named rather than glossed:** the addresses
above are from the **DEBUG** build (`frameworkD.map`). The user's disc is
retail (`D44J01`-era `main.dol`), whose addresses differ. Mapping debug symbols
onto the retail DOL is the actual work, and it is unstarted. What is
established is that the data is DOL-resident, sized, and symbol-named — not
that the retail offsets are known.

## What this changes for the port

**HISTORY — do not paste the `assets/*.h` headers into plugin source.** The
code port (`d_grass` / `d_flower` / `d_tree` managers + the `d_a_grass`
spawner) is unaffected and can proceed; only the DATA needs the serving route.

**HOUSING/ENGINE — the serving route is mine.** Extracting DOL-resident tables
from the user's disc is the same job as serving arc members, and it is now on
this lane's plate.

## What is still unknown

Whether the retail DOL's layout can be resolved from the debug maps at
acceptable confidence, or whether it needs a signature/scan approach. Until
that is answered, the vegetation port can land its CODE but not its ASSETS.
