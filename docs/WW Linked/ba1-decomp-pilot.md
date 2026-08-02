# Ba1 (Grandma) un-stub pilot — the donor-decomp pipeline (Foundry §253)

**Ruling implemented:** user greenlit decomp participation (REL extension + m2c + Ba1
pilot). This doc is the pipeline's state and the pilot's worklist.

## The pipeline (all local, all donor-derived)

```
donor extracted tree (D:\XXXXXXX\Ex WW: sys/ + files/)
  └─junction→ WW-DP orig/GZLE01                 (DOL sha1 == build.sha1 receipt)
       └─ dtk dol split (v1.8.3, D:\XXXXXXX\tools\dtk.exe)
            → build/GZLE01/<module>/asm/*.s     416 modules / 39,319 fns in 10 s;
                                                relocated + symbol-annotated (the
                                                "REL extension" needed NO custom
                                                parser — dtk does REL relocation)
       └─ m2c (-t ppc = ppc-mwcc-c++, D:\XXXXXXX\m2c)
            → C draft                           NO context (tww ctx is C++; m2c's
                                                context parser is C-only)
       └─ tools/foundry/rel_decomp.py           driver + fopAc offset receipts:
            → port-kits/<mod>/decomp-draft/     every inferred field ≤0x290 gets
                                                its real fopAc_ac_c member cited;
                                                past-base = [INFERENCE-NEEDED]
```

- `rel_decomp.py <module> --list | -f fn ... | --all [--out o]`
- Hand-decode fallback for m2c failures: `dol_disasm.py` (§251 — proven on the
  getP_BtpData trio, landed + acceptance-passed §252).

## Pilot status (2026-07-30)

- **`ba1_full_draft.cpp`: 100/104 functions drafted clean (96%)** —
  `port-kits/npc_ba1/decomp-draft/`. 4 m2c failures to hand-decode:
  `next_msgStatus` (works in small-batch mode — retry there first),
  `event_actionInit`, `event_action`, `talk_1`.
- Template validation: ba1's `eventOrder` draft is structurally the SAME function
  as decompiled ls1's (`unkFA |= 1` = `eventInfo.onCondition(dEvtCnd_CANTALK_e)`)
  — WW NPC actors share the family skeleton (`init_*`, `setBtp`, `anmNum_toResID`,
  `getMsg_*`, `eventOrder/checkOrder`, `chk_talk`, `set_action` roster), so the
  decompiled siblings (ls1, bm1, ah, zl1, jb1) are the naming/typing oracle.

## Worklist to "ba1 un-stubbed" (order matters)

1. **Hand-decode the 4 failures** (dol_disasm; each < ~200 insns).
2. **Naming pass** — map past-base offsets by sibling-template analogy
   (ls1/bm1 layouts); every analogy-name carries `[INFERENCE]` until receipted
   (debug-map or behavior). Behavior is donor-verbatim regardless of names.
3. **Types pass** — replace m2c's inferred scalars with real donor types via the
   sibling headers; struct size sanity vs the draft's `size >= 0xNNN` floors.
4. **Assemble `d_a_npc_ba1.cpp`** in donor file order (the .s preserves it), keep
   the donor's `/* VA */` comments per function for future receipts.
5. **Port it with kit v4** (it's a roster CUTSCENE actor — in `tale.stb`/`tale_2`:
   demo shims load-bearing; §251 cutscene.md applies).
6. **Acceptance** — oracle stack: state-tap session + probe differ vs the golden
   `tale` trace (P3 target). DRAFT → ACCEPTED only on gate pass; never MATCH.

## Covenant notes

- Everything derives from the user's own donor files (arcs, DOL, RELs).
- Drafts are **port-grade decomp**: behavioral fidelity, explicitly NOT
  byte-match; do not upstream without separate match work.
- **Upstream sync rule:** `git -C "D:\XXXXXXX\WW DP" pull` weekly before touching
  the next stub — upstream is actively decompiling NPCs (zl1 cleanup 3 days old);
  never duplicate an actor they've started.
