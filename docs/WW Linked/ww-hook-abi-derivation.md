# Hook ABI — derivation from census Axis W (roadmap step 15)

**Owner:** Foundry specs · Engine builds. **Blocked by:** step 8 (done).
**Blocks:** step 16 (native plugin loading — the long pole).
**Status:** DERIVATION landed. **B2 CLOSED (user ruled NO VETO, §516) · B4
CLOSED (§517).** The ABI is now specifiable for everything except the three
UNKNOWN gates (B1), pending the user's B3 call on step 18 — which decides whether
it is shaped for subsystems or for legs.

This document is **reviewable and updatable** per the standing provision: every
number carries its instrument, its bound direction, and the trigger that would
change it. Nothing here is a decision — decisions are the user's and Engine's.


> ## ⚠ STANDING RULING — C3e: NO ICF VALIDATION BUILD (USER, 2026-08-07)
>
> **We will NOT build an ICF-enabled dusklight to validate the fold detector.**
>
> **ICF is not our setting.** It is a linker optimisation in *dusklight's* build
> that merges two functions with byte-identical code into one address. Dusklight's
> **Windows build already disables it** (`/OPT:NOICF`) so that hooks are safe; the
> Clang/macOS/Linux path does not.
>
> **What this ruling costs, stated so it is never mistaken for closure:** the fold
> detector has been tested only against fixtures we wrote and against a binary
> that cannot fold. **It has never been shown to recognise a real merge.** It is
> shipped as a load-time check that runs on the user's actual image, so it
> protects the non-Windows case as well as an unvalidated detector can — and its
> failure mode is a MISS, never a false alarm, so it degrades to no worse than not
> checking.
>
> **C3e is therefore NOT CONTROLLED, permanently, by decision — not by oversight.**
> Never record it as passed. Revisit only if macOS/Linux becomes a real target.

---

## 0. The headline, because it changes roadmap sequencing

> **No subsystem carries a WHOLESALE verdict. The count is zero, out of ten.**

| verdict | subsystems |
|---|---|
| PIECEWISE | 8 |
| ~~VETO-PENDING~~ → PIECEWISE | 2 — **user ruled NO VETO (§516)**; `GXTexObj` / `dMsgFlow_c` are shared state to solve, not disqualifiers |
| **WHOLESALE** | **0** |

Roadmap **step 18** reads *"Relocate WHOLESALE-verdict subsystems into the
plugin. **Cheap; they are already namespaced + gated.**"* That step's work list
is **empty**. It is not blocked, not deferred — there is nothing in it.

**Consequence for Stage D:** the boundary's entire weight lands on **step 19**
(leg → hook migration, one at a time, each with its kill switch), which the
roadmap already calls "the long tail". The cheap half of the plan does not exist.
**This is a finding for the user and for Bridge+Engine, not a Foundry decision**
— it may be read as "re-scope step 18", "re-examine the PIECEWISE thresholds", or
"accept that Stage D is step 19". Foundry states it and rules on none of it.

It also redirects step 15 itself. An ABI for relocating whole subsystems and an
ABI for migrating individual legs are different objects: the first wants a few
subsystem-sized interfaces, the second wants many small hooks with independent
kill switches. **The Hook ABI must be designed for step 19**, since step 18 has
no clients.

---

## 1. The measured surface, both directions

Axis W was built to measure the **gate** — what the receiver calls INTO a
subsystem. An ABI needs the other direction too: what the subsystem calls back
INTO the receiver. That direction is larger by an order of magnitude, and it is
the one that determines the plugin boundary's real cost.

### 1a. Inbound (the gate) — receiver → WW

| subsystem | GATE | total surface |
|---|---|---|
| `receiver engine TUs w/ donor legs` | 122–230 | 736–844 |
| `WW host systems (d_ext_)` | 100–153 | 626–679 |
| `WW engine legs (_ww)` | 48–54 | 214–220 |
| `JAudio1 / sequence lane` | 14–160 | 115–261 |
| `JEvent1` | 2–3 | 41–42 |
| `WW direct-port actors (TP-named)` | 1–103 | 465–567 |
| `MDoExt1 (3D line)` | 0–4 | 9–13 |
| `JPA (WW particle)` | **UNKNOWN** | 15–18 |
| `JSystem (receiver-owned, donor legs)` | **UNKNOWN** | 40–55 |
| `WW actors (ext)` | **UNKNOWN** | 189–213 |

GATE is entry points only. **Total surface is not a gate width** (§502 Finding A)
and must not be summed into one.

### 1a-L. Inbound, MEASURED BY THE LINKER (§577) — 129 symbols / 86 TUs

§1a above estimates the gate by static analysis and leaves **three subsystems
UNKNOWN**. The exclusion link test measures the same direction with a stronger
oracle — **the linker cannot be wrong about what does not resolve** — and returns
no UNKNOWNs:

> **129 symbols across 86 TUs** (`build/ww-excluded-link.log`, 352 unresolved-
> external lines). Every referencing TU is RECEIVER code: `d_a_alink`,
> `d_a_arrow`, `d_camera`, `d_s_play`, `d_stage`, `d_npc`.

**This is the exact complement of §1b.** 19a measured what the plugin needs from
the host — **15**. This measures what the host calls into the plugin — **129**.
Both directions are now measured rather than estimated, and **the inbound side is
~8.6× the outbound**.

**The count is not a progress metric, and treating it as one inverts it.** It
measures how much of the WW layer's API the receiver touches, so *excluding more
WW code reveals more of that surface*: 194 → 61 → 129 is not better-then-worse.
The first drop removed self-inflicted noise; the rise is coverage. **It can never
reach zero by excluding more** — it converges only when the call sites change,
i.e. when the receiver attaches by hook instead of calling directly.

> **Therefore step 11's link test CANNOT pass before step 19, by construction.**
> The roadmap's "partial by definition until 19" is not a caveat but a proof.

**Migration shape falls out of the distribution, and these are not one job:**

| cluster | syms / TUs | shape |
|---|---|---|
| `dWwItemmdl` | 38 / 12 | deep + narrow |
| `dExtNpcMount` | 36 / 17 | deep + narrow — with the above, 57% of the surface |
| `JEvent1` | 6 / 41 | shallow + wide — the opposite problem |
| `g_profile` | 20 / 1 | not a hook at all; mechanical |

### 1b. Outbound (the import surface) — WW → receiver

> **B4 RESOLVED 2026-08-06 — the floor is 1,150 symbols / 6,448 sites, not 468.
> My §512 framing was wrong and understated the ABI by ~2.5x.**

The §512 reading treated the 956 ambiguous symbols as probable collision
inflation. **Ambiguity is not inflation.** Resolving each symbol by the OWNERSHIP
of its declarations rather than by their count:

| bucket | symbols | sites | is it a host import? |
|---|---|---|---|
| host, unambiguous (1 decl, receiver) | 409 | 1,714 | **yes** |
| host, ambiguous but **every** decl receiver-owned | 741 | 4,734 | **yes** — the overload is unknown, the ownership is not |
| **FLOOR** | **1,150** | **6,448** | |
| MIXED receiver/WW decls | 206 | 1,415 | **UNKNOWN** |
| **UPPER** | **1,356** | **7,863** | |
| WW-owned decls only | 67 | 165 | **no** — true inflation, removed |

**The correction that matters:** a symbol declared in 144 places is still
certainly a host import if all 144 declarations are receiver-owned. Only 67
symbols were genuine inflation, not 956. **The boundary is more expensive than
§512 said, not less.**

**`info` — excluded, and it was never game coupling.** 514 `DuskLog.info(` calls
against 0 other member forms and 1 free call: it is the Aurora *logger*. A large
share is our own probe instrumentation, already on the never-push-strip-set. Its
382 sites leave the ABI entirely.

**Instrument trap found while resolving this, worth recording:** Axis W's
`callbacks[name]["sites"]` list is **capped at 8 entries per subsystem** while
`site_count` carries the true total. Reading `len(sites)` silently understates —
it reported `info` at 51 sites against a true 382. Any future analysis must use
`site_count`.

**Where the dependency concentrates** — the ABI's shape follows from this:

| symbols | sites | header |
|---|---|---|
| 198 | 1,567 | `include/d/d_com_inf_game.h` |
| 75 | 486 | `include/f_op/f_op_actor_mng.h` |
| 39 | 114 | `include/d/d_ext_ww_actor_shims.h` |
| 37 | 94 | `include/d/d_kankyo.h` |
| 36 | 347 | `include/m_Do/m_Do_ext.h` |
| 35 | 136 | `include/d/d_npc.h` |

`d_com_inf_game.h` alone carries **14% of the symbols and 19% of the sites**. Any
plugin boundary is, first and mostly, a boundary against that one header.

**432 symbols are shared by more than one subsystem** — they are the candidate
*common* ABI, as opposed to per-subsystem hooks.

---

## 2. What blocks the ABI spec

| # | blocker | owner | effect |
|---|---|---|---|
| B1 | 3 subsystems have **GATE UNKNOWN** — no entry point resolves and they use indirect dispatch (L1) | **decode lane (D-2)** | their interfaces cannot be sized; "unmeasured, not narrow" |
| ~~B2~~ | ~~2 subsystems VETO-PENDING on spec §5~~ **CLOSED 2026-08-06 — NO VETO (§516).** Both return to candidacy | **USER** ✔ | shared state to be solved, not a disqualifier |
| B3 | step 18's work list is **empty** (§0) | **USER / Bridge+Engine** | determines whether the ABI targets subsystems or legs |
| ~~B4~~ | ~~`info` + the ambiguous 956 unverified~~ **CLOSED 2026-08-06 (§517).** Floor moved **468 → 1,150**; only 67 symbols were inflation; `info` excluded as logging | **Foundry** ✔ | the boundary is ~2.5x more expensive than §512 said |

**Remaining: B1** (decode lane) and **B3's second half** (user). Neither is
Foundry's to clear.

---

## 3. What CAN be stated now, and would survive B1–B4

These follow from the floor, so clearing the blockers can only add to them:

1. **The ABI is import-dominated, not gate-dominated.** **≥1,150** host symbols
   over ≥6,448 sites are called by WW code, against gates of 0–230. Any design that treats the plugin
   boundary as "a few entry points" is contradicted by the measurement.
2. **`d_com_inf_game.h` is the boundary's centre of mass** (198 symbols / 1,567
   sites). It is the first header any hook layer must abstract, and the largest
   single source of coupling.
3. **A shared core exists**: 432 symbols are used by more than one subsystem, so
   per-subsystem hook sets would duplicate them. A common host-interface layer is
   indicated by the data, not by preference.
4. **Step 19's granularity is already measured**: 14 declared legs across 11 TUs,
   447 lines, 9 file-scope / 5 in-function (adoption record §1c). File-scope and
   in-function legs need different hook shapes.

---

## 4. What this document does NOT authorise

It does not rule on step 18's re-scoping, does not select a plugin technology,
does not name or shape a single hook, and does not claim the import surface is
exactly 1,150 *or* 1,356 — it claims the true value lies between them, with the
67 genuinely inflated symbols removed and the 206 MIXED ones still unresolved. **Foundry measures; Engine designs the hook; the user rules
on scope.**

---

## 5. Revisit triggers

* decode lane clears D-2 → B1 lifts → three GATE values become measurable
* user rules spec §5 → B2 lifts → VETO-PENDING resolves either way
* any subsystem's verdict changes to WHOLESALE → §0 and step 18 change
* Engine lands further `KIT-DONOR-HUNK` markers → §3.4's leg count rises (it is a
  floor and is expected to)
* ~~the `info` / ambiguous-956 check completes~~ **done (§517): floor 468 → 1,150**
* the 206 MIXED-ownership symbols resolve → the floor moves again, upward only
