# FINDING — `getRes` returns a vtable-bearing object for every `.bmd` and `.btk`

era: mounted
<!-- era rationale: measurement against a named boot log | Foundry, 2026-08-18 -->

**Foundry, 2026-08-18.** Read-only, from a log already on disk. Ordered as "the
cheapest thing on the board" (enumerate the 38 `res_miss`). The `res_miss`
enumeration came back benign; **this is what was next to it.**

Source: `%AppData%/TwilitRealm/Dusklight/logs/dusklight-20260817-224807.log`.
Probe: `bg_model_lookup`, a **post-hook on `dRes_control_c::getRes`**
(`registry.cpp:3956`), so `got` is **the receiver's own return value**.

## The measurement

112 lookups. 71 null, 41 non-null. The 41 split **perfectly by extension**:

| ext | non-null | first qword of the returned buffer |
|---|---|---|
| `bdl` | 3 | real `J3D2` |
| `dzr` | 4 | real (BE chunk count + `SCLS`/`FILI`) |
| `dzs` `dzb` `kcl` `plc` | 7 | real |
| **`bmd`** | **21** | **`0x00007FF6_D873C798` — constant** |
| **`btk`** | **6** | **`0x00007FF6_D873B6A8` — constant** |

The `got` pointers themselves **vary** (11 distinct for `.bmd`, 6 for `.btk`),
but every object starts with the **same constant per extension**. `0x00007FF6`
is the high half of a Win64 **module-image** address.

**Distinct heap objects sharing a first-qword pointer into the image is the
signature of a C++ vtable** — i.e. these are *constructed objects*, not file
buffers. Two classes: one answering `.bmd`, one answering `.btk`.

*(Labelled inference, not measurement: the vtable reading. What is MEASURED is
the constant, its module-image range, and the perfect split by extension.)*

## Why this is a silent-non-draw shape

A **null** return lets a caller fall through to the sibling extension — and that
demonstrably works: `R44_00/model.bmd` returns null, `R44_00/model.bdl` returns
real `J3D2`. A **non-null** return does not fall through. It is accepted.

So wherever `.bmd` is probed first and answers non-null, the consumer takes an
object whose offset 0 is a vtable pointer where the `J3D2` magic should be.
**Valid pointer, no error path, nothing renders.** That is the exact failure
class the P1 correction describes as never having been examined — and it is
invisible to the seam census, which classifies SYMBOLS, not CONTENTS.

Not uniform across arcs: `R00_00/model.bmd` returns a stub (taken), while
`R44_00/model.bmd` returns null (falls through). **The same probe pair resolves
differently per arc**, which is why a single-room reading would have missed it.

## What this pass RULED OUT

The 38 `res_miss` are **benign**. Enumerated in full: `Stg_00` 10, `R00_00` 13,
`R44_00` 15. Classifying the 71 nulls: **48 have a sibling extension that HIT**
(ordinary probe pairs) and **23 are real absences** — all of them high-numbered
models (`model2`/`model4`/`model5`) in rooms that only have two or three. That
is the loader walking to exhaustion. **No room-model resolution failure exists.**

## Two corrections this pass produced, both mine

1. I first read the `res_miss` list as "R44 = Outset, its room models are not
   resolving." **Wrong** — I read a miss list without reading the hit list beside
   it. Outset's `model.bdl` and `model1.bdl` resolve with valid `J3D2`.
2. I then said "resource loading is not the blank," drawn from the **null**
   population only. **The defect is in the NON-NULL population** — the half I
   had not looked at when I said it.

## Also falsified

The proposed correlation `bg_model_lookup 112 - bmdl_pass 72 = 40 ~ res_miss 38`
**does not hold: those are different units.** `bg_model_lookup` is
per-RESOURCE; `bmdl_pass` is per-ARC-WALK (all 72 are `why:"walked"`, with their
own `count`/`nonNull`). The near-match is a coincidence of scale, so "almost
certainly the same population" should not be used to scope the investigation.

## Ferried, not rooted

Per the lane boundary — Foundry builds instruments, the data goes to the owning
lane. **Rooting this is Engine's** (what constructs those two objects, and which
consumer accepts them). What Foundry owes next is a probe that logs the
CONSUMER side, so the accept is observed rather than inferred.

Reproduce: any of the seven `2026-08-17` boot logs — all carry exactly 38
`res_miss`, so the shape is deterministic, not incidental.
