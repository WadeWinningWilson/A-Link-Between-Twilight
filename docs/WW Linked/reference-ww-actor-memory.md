# Reference — WW Actor Loading / Unloading & Heap (external)

> **EXTERNAL reference** from [zeldaspeedruns.com/tww/techniques/actor-unloading](https://www.zeldaspeedruns.com/tww/techniques/actor-unloading).
> Describes the **GameCube WW** actor/heap system. Our port runs on the TP engine; the numbers are
> WW-specific, but the *architecture* is shared J3D/JSystem lineage and clarifies behaviour we
> already work with (room lanes, actor lifecycle, the spawn FIFO). Tags: ⚑ relevant to our work.

Ledger: №195. Companion: [reference-ww-flags-triggers.md](reference-ww-flags-triggers.md).

---

## Heap spaces (WW figures)

| Heap | Alias | Size | Holds |
|---|---|---|---|
| **ACT** | ArchiveHeap | `0xA3F000` (~10.7 MB) | file data + actor objects |
| **DYN** | ZeldaHeap | `0x63D580` (~6.5 MB) | dynamic actors (rupee spawns, runtime vars) |
| **GAME** | — | ~1.3 MB | additional actor data |

> ⚑ The **ACT vs DYN** split is the same distinction we hit with WW arcs: statically-placed actors
> and their arcs vs runtime-spawned ones. Our J3D-pointer-fix rule (never re-parse a fixed buffer,
> never free an arc while parsed data is cached) lives on the ACT side.

---

## Allocation — a doubly-linked free list

> *"Each entry points to the prior and next one. Free space has a size parameter so the game knows
> immediately how much can fit there."* When loading an actor, *"the game loops through the list
> from top to bottom and tries to find a big enough slot to fit the data in."*

First-fit into a doubly-linked list of blocks. This is JKR heap behaviour and explains
fragmentation-driven load failures.

---

## Unloading — instance-counted, and DEFERRED past the next load

> *"An actor gets destroyed when you kill it, hit a loading zone, or leave the room — the actor is
> destroyed AFTER the door closes, which occurs AFTER the next room loaded."*
>
> Tracking: *"a separate list of loaded actors (by name)"* with instance counters — **++ on
> construction, -- on destruction**; the archive unloads when the count hits zero.

> ⚑ **This is the exact ordering hazard behind our lifecycle bugs.** The old room's actors are still
> alive while the new room loads — both resident briefly — and free blocks only merge *"shortly
> after the next room was loaded."* Our room-lane teardown guards (`roomLaneMountIsUnloading`, the
> "don't touch matrices while unloading" rule) exist precisely because destruction is deferred past
> the next load. The name-keyed instance counter is also why the spawn-FIFO identity work mattered:
> two actors sharing a name/count is how identity corruption crept in (№126/№129).

---

## Merge timing

> *"The memory manager attempts to merge free space with adjacent free blocks… shortly after the
> next room was loaded."*

So peak memory pressure is **during** the overlap window (old room not yet freed, new room loaded),
not after. Relevant when a WW room port fails to load only under a specific transition — the culprit
is the transient double-residency, not the room's own size.
