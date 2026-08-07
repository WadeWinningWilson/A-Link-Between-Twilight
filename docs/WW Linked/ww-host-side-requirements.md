# Host-side requirements — fixes the WW layer needs that live OUTSIDE our control

## What this is, and why it is a register rather than a blocker list

The WW layer sometimes needs a fix in code we do not own. When that happens the
fix cannot ship with the plugin: it has to reach the host, through parties who
are not us and on timelines we do not set.

```
dusklight   origin    TwilitRealm/dusklight               not ours
            upstream  WadeWinningWilson/A-Link-...        our fork
aurora      origin    encounter/aurora                    not ours
```

So the route for any entry here is: **our fork → a PR to the owning project →
that project accepts → dusklight bumps its pin → stock builds gain it.** Three
of those four steps belong to somebody else.

**These are logged, not escalated.** The port is at roughly half of Outset
Island. Host-side gaps are the expected output of porting content nobody has
run before, and there will be many more; treating each as a critical-path
blocker for a product that does not exist yet is noise. The point of this file
is that they accumulate in one place, keep their evidence, and can be submitted
as a BATCH when submitting is actually worth doing.

**What it costs to defer: nothing today.** Our fork carries every fix locally,
so development is unaffected. The debt is only realised the day someone wants
the plugin to run on a stock dusklight build, which is a step-20 concern.

## Register

### HR-1 — aurora `LOAD_INDX` indexed-XF path

| | |
|---|---|
| owner | `encounter/aurora` |
| status | fix live in our fork; patch prepared, **not submitted** |
| patch | `patches/aurora/upstream/0001-load-indx-fix.patch` — 32 added lines |
| found | §449/§450, 2026-08-04 |

Two defects in the FIFO parser's indexed-XF path: an operator-precedence error
selecting the vertex array (`GX_POS_MTX_ARRAY + (opcode - (CP_CMD_LOAD_INDX_A /
0x08))` divides the constant, indexing 23–47 entries out of bounds), and a
misparsed 4-byte payload that advanced one byte too far and desynced the rest of
the FIFO.

**Why it is a host requirement and not ours to carry.** It sits in the graphics
backend's command parser, below game code entirely — a plugin hooks game
functions, not the renderer's internals.

**Why stock dusklight does not already hit it.** The path has been broken since
the port began, but TP content never emits the opcode. WW content does: the
Outset rope bridge was the first thing to reach it (§449 — *"ropes only made one
reachable at Outset"*). So this is invisible upstream until WW content runs.

Local diff is ~1300 lines across 13 files; **only 32 are the fix.** The rest is
debug instrumentation and must never travel — the upstream patch already has it
stripped, and the full state is parked at `patches/aurora/local/working-tree.patch`
purely so nothing is lost before someone strips it.

**It does not become obsolete as more of WW lands — the opposite.** The opcode
is emitted by core J3D for ANY model using indexed matrix loads, so every extra
skinned character sends more of them. Porting WW's own native systems would not
help: a faithful renderer emits the same opcode, because that is the hardware.
Ropes revealed it; ropes are not its scope.

## Known gap in how we would DETECT these

19b's conformance check resolves the WW import list against fork and upstream
images, which is the right instrument for *symbols* — and this class of problem
is not a symbol problem. `LOAD_INDX` resolves fine on both; it simply behaves
wrongly. **We currently have no instrument that catches a host-side BEHAVIOUR
gap**, and the one entry here was found by accident, by rendering something that
happened to reach it.

Recorded rather than solved: the honest detector is running WW content on a
stock-aurora build and seeing what breaks, which is not worth building until
there is much more content to run.

## Design note — what a hook CAN and CANNOT reach (2026-08-07)

Step 19 Phase 1 adopted a hook on `fpcPf_Get` after verifying the symbol
resolves in a shipped build, not just ours:

```
fpcPf_Get / ?fpcPf_Get     our fork exe: present     upstream exe: present
```

That verification is what makes the plugin's end state credible: **download,
drop in the mod folder, supply your own WW ISO, run.** No binary patching, no
file surgery, no third-party tooling — the plugin attaches itself at load.

**It does not, and cannot, address anything in this register.** A hook attaches
to a symbol in the host; HR-1 is a *behaviour* defect BELOW the plugin boundary,
in code that has already run by the time a plugin loads. So the honest statement
of the end state is:

> The plugin will LOAD and REGISTER correctly on a stock dusklight build. It
> will still crash when WW content reaches the indexed-matrix path, until the
> host itself carries the fix.

Those are different claims and collapsing them would be the same error as
reading "symbols resolve" (19b) as "it works". **19b proves attachment. Nothing
currently proves behaviour**, which is the gap this register already records.

Practical consequence for distribution: an end user cannot apply an aurora
patch. Aurora is compiled into `dusklight.exe`. The fix has to be in the build
they already have, which is why entries here are worth batching toward upstream
eventually — not urgently, but not never.
