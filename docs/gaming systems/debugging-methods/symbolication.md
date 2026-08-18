# Symbolication — turn a crash address into a function + line

The method that ended the "still a crash" cycle. When Dusklight prints an access-violation with a
`rva=` frame, **symbolicate first** — do not guess across rebuilds.

## The method

Resolve the `rva=` to `function + line` with the x64 `llvm-symbolizer` and the RelWithDebInfo PDB:

```bash
llvm-symbolizer.exe --relative-address --exe=<...>/dusklight.exe <rva>

<!-- CORRECTED 2026-08-16 (Librarian, while symbolicating the A_mori exit crash):
     this line said `--exe=<...>/dusklight.pdb`. llvm-symbolizer wants the EXE and
     finds the PDB beside it; pointing --exe at the .pdb yields SILENT EMPTY OUTPUT --
     no error, no frames, just nothing, which reads as "unsymbolicatable build".
     Verified both ways on rva 0xe203c: .pdb -> blank; .exe -> fpcNd_Delete
     f_pc_node.cpp:97. Also: VS 18 ships it at
     `.../VC/Tools/Llvm/x64/bin/llvm-symbolizer.exe` (the doc's `Tools/Llvm/x64/bin`
     is right, but there is also a non-x64 `Tools/Llvm/bin` copy next to it). -->
```

- `llvm-symbolizer.exe` ships with Visual Studio at `…/Tools/Llvm/x64/bin/` (use the **x64** build).
- `--relative-address` matches Dusklight's `rva=` frames.
- Point `--exe`/`--obj` at the **RelWithDebInfo PDB** for the exact build that crashed.

## Why it matters (the receipt)

The Hero's Clothes demo prop crashed repeatedly. Wrong hypotheses — create-order, resource ids, NULL
guards — each cost a build. Symbolication resolved the frame to **`mDoExt_bckAnmRemove`**, which pointed
straight at `fuku.bdl` being **unparsed** (the missing BDL parsers; the magic is actually `J3D2bmd3`).
That one lookup replaced ~4 speculative rebuilds. See
[../models-and-lighting/bdl-bmd-parse-timing.md](../models-and-lighting/bdl-bmd-parse-timing.md).

## Rule

On any access-violation with a `rva=` frame, **symbolicate before forming a hypothesis.** A resolved
function name is worth more than any number of educated guesses, and it costs one command.

*(Also saved as the memory `reference_symbolicate_crash`.)*
