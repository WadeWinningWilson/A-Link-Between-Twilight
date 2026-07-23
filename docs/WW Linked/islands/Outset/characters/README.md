# Outset — Characters

One subfolder per character, because each restored NPC accumulates a lot of specific detail (census
code, assets, cutscene beats, animation/expression maps, open bugs) that would otherwise bury the
island-level docs.

## Convention

```
characters/
  <Name>/
    README.md      ← the character's index: identity, assets, status, open issues, ledger links
    <topic>.md     ← deep-dives that outgrow the README (e.g. cutscene beat sheet, expression map)
```

- **Naming.** Use the character's real name only when it is **user-locked or decomp-sourced**
  (Ivan rule). Until then, use the census code and mark `? (unverified)`. Aryll qualifies — her actor
  is `daNpc_Ls1_c` in the donor and the user has confirmed her.
- **What lives here vs. elsewhere.** Character-*specific* facts live here. **General, reusable
  procedures stay at the `WW Linked` level** and are linked to, not copied:
  [cutscene-recipe](../../../cutscene-recipe.md),
  [cutscene-animation-recipe](../../../cutscene-animation-recipe.md),
  [face-expression-recipe](../../../face-expression-recipe.md).
- **The ledger is not duplicated.** The chronological record stays in
  [docs/state/cut-actors-demo-restore.md](../../../../state/cut-actors-demo-restore.md); a character
  README *links* to the relevant №-entries, it does not restate them.

## Roster

| Folder | Census | Identity | Status |
|---|---|---|---|
| [Aryll](Aryll/README.md) | `Ls1` (proc `NPC_LS`) | ✓ decomp + user | Opening cutscene performs; one texture bug open |

_(Grandma, Orca, Sturgeon, Sue-Belle, Abe, Mesa, and the rest get folders as work reaches them.)_
