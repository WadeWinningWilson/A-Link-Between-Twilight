# .githooks — B5, the convenience tier

**B1's seam gate in `build_run.bat` is the enforcement. These hooks are the same
checks, earlier.** A bad row edit caught at commit time costs you thirty seconds;
caught at build time it may cost someone else's build, hours later, with no idea
why it broke.

## Activation is a deliberate manual step

```bash
git config core.hooksPath .githooks
```

**It is NOT set by this repo, and that is a decision rather than an oversight.**
`core.hooksPath` changes git's behaviour for **every** lane and every tool sharing
this working tree — including builds, scripts, and any agent that commits. That is
a workflow change for other people, and Phase B assigns Bridge the *creation* of
the hook, not the reconfiguration of everyone's git.

Run the line above when you want it. To turn it off again:

```bash
git config --unset core.hooksPath
```

## What `pre-commit` runs

| check | what it catches | cost |
|---|---|---|
| `row_store.py validate` | malformed rows: bad id, undeclared `doorway`/`destination`, non-schema axis state, `PATCH` with no negative-control, stored `provenance`, citation without a read timestamp | sub-second |
| `seam_gate.py check --staged` | a baselined row gone with no `RETIRED.md` entry; a **staged** WW-layer file whose ownership category nobody has classified (working-tree WIP is B1 / `build_run.bat`) | sub-second |

Neither invokes the compiler. **A slow pre-commit hook gets disabled, and a
disabled hook protects nothing** — so this one stays cheap on purpose.

## Bypass

`git commit --no-verify` — git's own flag. **No bespoke env var was invented for
this**, because a second bypass mechanism is a second thing to forget is switched
on. (`build_run.bat`'s `DUSK_SKIP_SEAM_GATE=1` exists because a `.bat` has no
equivalent of `--no-verify`.)

## If the hook is wrong

Skip it and say so on `CALLS.md`. A gate that refuses wrongly and is silently
worked around is worse than no gate: the next person inherits both the bad rule
and the habit of routing around it.
