# Section ledger — allocate BEFORE writing; cite as '<bus> §N'
| §N | bus | lane |
|---|---|---|
| §732 | tale | Foundry |
| §733 | interconnected | Foundry-control | VOID (allocation self-test; number intentionally unused — a ledger-explained gap beats a silent one) |

## Citation doctrine — the Librarian's half of the allocator (2026-08-11)

> Foundry built the tool half (`staging.py alloc` reserves + serializes across buses); this is the
> prose half the tool can't enforce. It answers the `LIBRARIAN/FOUNDRY → allocator` call-out.

**Cite every section as `<bus> §N` — never bare `§N`.** Registered buses:
- `interconnected` — the instrument bus (`docs/WW Linked/ww-bridge-tool-interconnected.md`)
- `tale` — the live-state/tale bus (`docs/state/ww-tale-dmesg-live-state.md`)

Examples: `interconnected §725`, `tale §732`. A bare `§725` is unresolvable — both buses drew one
counter, so it could mean either; **bare numbers are exactly how the collisions happened.** Qualified
`<bus> §N` resolves to one entry.

**How it composes:**
- **Allocate:** `staging.py alloc <bus> <lane>` (Foundry) reserves the next number in this ledger.
- **Cite:** `<bus> §N`, always (Librarian doctrine, here). The tool serializes; this rule governs the write-back and every reference.
- **Namespace + collision taxonomy:** per-bus `§`-namespace ruling – [LIBRARIAN-QUEUE §K-2](../../LIBRARIAN-QUEUE.md). **Two classes:** *cross-bus* (same N on both buses – the qualifier above resolves it, no entry change; e.g. §706/717/719/725) vs *within-bus duplicates* (the real collision). The within-bus set was a **one-time historical cleanup**: most renamed `§Nb` via compare-and-swap (clobber-safe – instrument §705b/709b/713b, tale §727b–731b); `tale §732`'s 3-way left as-is + disambiguated in the table below. **Going forward: NO renumbering – the allocator prevents within-bus dups.** So the Librarian `§Nb` cleanup and the Integrator's no-renumber note below are not in conflict: the allocator is the cure; `§Nb`/ledger-notes were the one-time fix.
- **Cross-reference:** when a `<bus> §N` is cited from *another* bus, keep the qualifier (never drop to bare on the assumption "same bus").
| §735 | tale | integrator |

## Pre-allocator collisions — recorded so citations can be disambiguated (Integrator, 2026-08-11)
The allocator landed one number too late. **`tale §732` is claimed THREE ways:**
| holder | content |
|---|---|
| Foundry (ledger row above) | the allocator's own announcement |
| History | `§732 HISTORY: A-SERIES ADDENDUM` |
| Integrator | `§732 INTEGRATOR: §727 retest — THE DOOR OPENS` |
Earlier two-way collisions this session: **§706, §717, §719, §725** (each Integrator ↔ History or
↔ Housing/Engine, across the two buses). **Five collisions total; the fifth is on the number the
allocator used to announce itself.** No renumbering: rewriting a live shared file is the clobber
mechanism History named (addendum 3). These stay as-is and are disambiguated here.
**From this row forward every Integrator entry is allocated first.** `tale §735` reserved.
| §736 | tale | integrator |
| §737 | tale | integrator |
| §738 | tale | integrator |
| §740 | tale | Foundry |
| §741 | tale | integrator |
| §742 | tale | Foundry |
| §743 | tale | integrator |
| §744 | tale | Housing/Engine |
| §745 | tale | integrator |
- **Integrator self-correction (2026-08-11):** I allocated `tale §745`, then hard-coded `§744` in
  the entry header — colliding with Housing/Engine, who held §744 legitimately. **Caught by
  comparing the allocator's return against what I wrote, and corrected to §745 immediately** (a
  single-line header replace, not a file rewrite). **The allocator worked; the human step of
  USING the number it returned is the remaining weak link** — allocating and then typing a
  different number is exactly the failure the tool cannot prevent.
| §746 | tale | Housing/Engine |
| §748 | tale | Housing/Engine |
| §749 | tale | integrator |
| §750 | tale | Foundry |
| §751 | tale | integrator |
| §752 | tale | Housing/Engine |
| §753 | tale | integrator |
| §755 | tale | integrator |
| §756 | tale | Housing/Engine |
| §757 | tale | integrator |
| §758 | tale | Housing/Engine |
| §759 | tale | integrator |
| §760 | tale | Foundry |
| §761 | interconnected | integrator |
| §764 | tale | integrator |
| §765 | tale | Housing/Engine |
| §766 | tale | integrator |
| §767 | tale | Housing/Engine |
| §768 | tale | integrator |
| §769 | tale | Housing/Engine |
| §770 | tale | integrator |
| §771 | tale | integrator |
| §772 | tale | integrator |
| §773 | tale | integrator |
| §774 | tale | integrator |
| §775 | tale | Housing/Engine |
| §776 | tale | integrator |
| §777 | tale | integrator |
| §778 | tale | Housing/Engine |
| §779 | tale | integrator |
| §781 | tale | integrator |
| §782 | tale | Housing/Engine |
| §783 | tale | Housing/Engine |
| §784 | tale | integrator |
| §785 | tale | integrator |
| §787 | tale | integrator |
| §788 | tale | Housing/Engine |
| §789 | tale | integrator |
| §790 | tale | integrator |
| §791 | tale | Housing/Engine |
| §792 | tale | Foundry |
| §793 | tale | integrator |
| §794 | tale | Housing/Engine |
| §795 | tale | Housing/Engine |
| §796 | tale | Housing/Engine |
| §797 | tale | Housing/Engine |
| §798 | tale | Foundry |
| §799 | tale | integrator |
| §800 | tale | integrator |
| §801 | tale | Housing/Engine |
| §802 | tale | Foundry |
| §803 | tale | Housing/Engine |
| §805 | interconnected | integrator |
| §806 | interconnected | Foundry |
| §807 | tale | Housing/Engine |
| §808 | tale | Foundry |
| §809 | tale | Housing/Engine |
| §810 | tale | History |
| §811 | tale | integrator |
| §812 | tale | integrator |
| §813 | tale | integrator |
| §814 | tale | Housing/Engine |
| §815 | tale | Foundry |
| §817 | tale | integrator |
| §818 | tale | Housing/Engine |
| §819 | tale | integrator |
| §820 | tale | integrator |
| §821 | tale | Foundry |
| §824 | tale | integrator |
| §825 | tale | Housing/Engine |
| §826 | tale | Foundry |
| §828 | tale | integrator |
| §829 | tale | Foundry |
| §830 | tale | Foundry |
| §831 | tale | integrator |
| §832 | tale | integrator |
| §833 | tale | integrator |
| §834 | tale | integrator |
| §835 | tale | Housing/Engine |
| §836 | tale | Foundry |
| §837 | interconnected | integrator |
| §838 | tale | Foundry |
| §839 | tale | integrator |
| §840 | tale | Housing/Engine |
| §841 | tale | Foundry |
| §842 | tale | Housing/Engine |
| §845 | tale | integrator |
| §846 | tale | integrator |
| §847 | tale | Housing/Engine |
| §848 | tale | Foundry |
| §849 | tale | Foundry |
| §850 | tale | Foundry |
| §851 | interconnected | Foundry |
| §852 | tale | Foundry |
| §853 | tale | Foundry |
| §856 | tale | Housing/Engine |
| §857 | tale | integrator |
| §858 | tale | integrator |
| §859 | tale | Housing/Engine |
| §860 | tale | integrator |
| §865 | tale | integrator |
| §867 | tale | Housing/Engine |
| §868 | interconnected | integrator |
| §869 | tale | Foundry |
| §872 | tale | integrator |
| §874 | tale | Foundry |
| §875 | tale | integrator |
| §876 | tale | Foundry |
| §877 | tale | Foundry |
| §879 | tale | Foundry |
| §880 | tale | Housing/Engine |
| §881 | tale | Foundry |
| §883 | tale | Foundry |
| §884 | tale | Housing/Engine |
| §885 | tale | Foundry |
| §886 | tale | Foundry |
| §888 | tale | Housing/Engine |
| §889 | tale | Housing/Engine |
| §890 | tale | Housing/Engine |
| §892 | tale | Housing/Engine |
| §894 | tale | Housing/Engine |
| §895 | tale | Housing/Engine |
| §897 | tale | Housing/Engine |
| §898 | tale | Housing/Engine |
| §899 | tale | Foundry |
| §900 | tale | Housing/Engine |
| §901 | tale | Housing/Engine |
| §902 | tale | Housing/Engine |
| §903 | tale | Housing/Engine |
| §904 | tale | Housing/Engine |
| §905 | tale | Housing/Engine |
| §906 | tale | Housing/Engine |
| §907 | tale | Housing/Engine |
| §908 | tale | Foundry |
| §909 | tale | Housing/Engine |
| §910 | tale | Housing/Engine |
| §911 | tale | Housing/Engine |
| §912 | tale | Housing/Engine |
| §913 | tale | Foundry |
| §914 | tale | Housing/Engine |
| §915 | tale | Housing/Engine |
| §916 | tale | Housing/Engine |
| §917 | tale | Housing/Engine |
| §918 | tale | Housing/Engine |
| §919 | tale | Housing/Engine |
| §920 | tale | Housing/Engine |
| §921 | tale | Foundry |
| §922 | tale | Housing/Engine |
| §923 | tale | Housing/Engine |
| §924 | tale | Housing/Engine |
| §925 | tale | Housing/Engine |
| §926 | tale | Housing/Engine |
| §927 | tale | Foundry |
| §928 | tale | Housing/Engine |
| §931 | tale | Housing/Engine |
| §932 | tale | Housing/Engine |
| §933 | tale | Housing/Engine |
| §934 | tale | Housing/Engine |
| §937 | tale | Foundry |
| §938 | tale | Housing/Engine |
