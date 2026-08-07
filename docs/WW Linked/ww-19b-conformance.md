# 19b CONFORMANCE — the import manifest resolved against real images
#
# Answers what the .def CANNOT: does resolve() actually find these names in a
# built image. RESOLVED* = matched via a qualified name, not an exact one.
# A MISSING row is NOT a defect by itself -- 7 of the 15 are CRT/STL names and
# parse artifacts that were never host imports (see adoption record 1f).
# regenerate: see tools/foundry/symbol_manifest.py
# manifest_sha256: c0aa1e319f8a1f5d95de850db28bf59a22be7e7fc0b1160277654e882801223e
# entries: 15

# image[fork]: build/windows-msvc-relwithdebinfo/dusklight.exe
#   symbols: 277452
# image[upstream]: C:/Users/xxxxx/Documents/dusklight-main/build/windows-msvc-relwithdebinfo/dusklight.exe
#   symbols: 272796

| symbol | fork | upstream |
|---|---|---|
| `ModuleEpilog` | MISSING | MISSING |
| `ModuleProlog` | MISSING | MISSING |
| `emplace_back` | MISSING | MISSING |
| `field_0x0` | MISSING | MISSING |
| `getManager` | RESOLVED* | RESOLVED* |
| `getenv` | RESOLVED | RESOLVED |
| `good` | MISSING | MISSING |
| `max` | RESOLVED | RESOLVED |
| `memset` | RESOLVED | RESOLVED |
| `msg` | MISSING | MISSING |
| `path` | RESOLVED* | RESOLVED* |
| `push_back` | RESOLVED* | RESOLVED* |
| `setBaseTRMtx` | RESOLVED* | RESOLVED* |
| `sin` | RESOLVED | RESOLVED |
| `sp54` | MISSING | MISSING |

**15 of 15 classify IDENTICALLY on both builds.**
