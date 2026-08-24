# Commit and push — A Link Between Twilight fork

**Audience:** You and Cursor agents. This fork is **`WadeWinningWilson/A-Link-Between-Twilight`**, not `TwilitRealm/dusklight`.

**Build analysis chat:** After HUD/gameplay changes, validate FPS per [build-fps-guidelines.md](build-fps-guidelines.md) (and [hud-performance-handoff.md](hud-performance-handoff.md) if HUD touched) **before** commit. Doc map for reviewers: [build-fps-guidelines.md](build-fps-guidelines.md) § “Doc map”.

**Related:** [build-fps-guidelines.md](build-fps-guidelines.md) (build before commit if gameplay changed), [performance-handoff.md](performance-handoff.md) (perf investigation only).

---

## Remotes

| Remote | URL | Use |
|--------|-----|-----|
| **`upstream`** | `https://github.com/WadeWinningWilson/A-Link-Between-Twilight.git` | **Push here** — your ALBW fork |
| `origin` | `https://github.com/TwilitRealm/dusklight.git` | Upstream Dusklight reference; **do not push** unless you have TwilitRealm write access |

```powershell
git remote -v   # confirm upstream points at A-Link-Between-Twilight
```

---

## Never commit

These stay local (see `.gitignore`):

| Category | Examples |
|----------|----------|
| **Drive automation** | `src/dusk/drive.cpp`, `include/dusk/drive.h`, `local_dev_backup/` |
| **CoNavigate** | `src/dusk/conavigate.cpp`, `include/dusk/conavigate.h`, `docs/conavigate.md`, `build_conavigate.bat` |
| **Temp debug dumps** | `albw_*_debug.txt` (runtime **output** only — not required in repo) |
| **Build artifacts** | `build/`, `*.exe`, `build_stdout.txt`, `build_stderr.txt` |
| **Local backups** | `albw_backup_*/`, `.claude/` |

Drive/CoNavigate tooling belongs in `local_dev_backup/` (gitignored) or on another machine.

---

## Before commit

1. **Only commit when you explicitly want to** (or after playtest approval).
2. **Cross-chat scope — ASK the user.** The working tree usually holds uncommitted WIP from *other* feature chats (Magic Armor, wardrobe, HUD, quick-swap, etc.). Do **not** silently exclude another chat's work, hedge in the commit message about "whose chat it belongs to," or assume chat-scoped commits. **Ask the user whether they want all work committed regardless of chat source.** Standing user preference (2026-06-30): **commit everything** — a full `git add -A` snapshot, no splitting by chat ownership (debug dumps / build artifacts from "Never commit" still excluded).
3. If gameplay/HUD changed, rebuild and spot-check:

```powershell
cmd /c build_run.bat
# Exe: build\windows-msvc-relwithdebinfo\dusklight.exe
```

4. Review scope:

```powershell
git status
git diff
git diff --staged
```

---

## Stage

```powershell
git add src/ include/ files.cmake CMakeLists.txt .gitignore docs/
# Add README or other paths only when intentionally changed
```

**Do not** `git add .` without checking — debug dumps and build logs must stay out.

---

## Commit

Use a short title + body (why, not just what):

```powershell
git commit -m "Short summary of why" -m "Optional detail: what changed and what was excluded."
```

---

## Privacy gating (BEFORE pushing — public repo)

The `upstream` fork is **public**. A local commit leaks nothing, but a **push** publishes file
*contents*. Scan the range about to be pushed for personal/machine-specific data and scrub it first.
This applies to **pushes only** — do not block or rewrite local commits over it.

**What to look for** (most common offender: hardcoded absolute paths in tooling/docs/scripts):

| Leak | Pattern | Fix |
|------|---------|-----|
| Windows username | `C:\Users\<name>\…` | `%USERPROFILE%\…` or a relative / env-var root |
| Machine drive paths | `<decomp-root>\…`, other absolute local roots | a configurable `ROOT`/env var, or relative path |
| Personal contact | email, real name in file *contents* | remove (committer identity in git metadata is separate) |
| Secrets | tokens, keys, passwords | remove + rotate |

**Run before every push** (scans the exact commits going out):

```powershell
# what will be pushed:
git log --oneline upstream/main..HEAD
# scan those files for personal paths / PII (adjust the name):
git grep -niE 'Users\\[A-Za-z0-9._-]+|/Users/[A-Za-z0-9._-]+|D:\\|toocoolfordeadpool' `
  $(git diff --name-only upstream/main..HEAD)
```

If hits: **scrub the strings** (paths still work via `%USERPROFILE%`/env/relative) — do **not** delete
files (un-tracking ≠ scrubbing, and the work is preserved either way). Amend or add a follow-up commit,
then push. Core game source (`src/`, `include/`) is usually clean; companion-mod tooling, Blender
scripts, and handoff docs are the usual carriers.

> **Known standing item (2026-07):** the MM-SkullKid / companion-mod tooling under
> `tools/companion_mod/`, `tools/blender_socket/`, and `docs/Blender-MM-SkullKid.md` contain
> `%USERPROFILE%\…` / `<decomp-root>\…` literals. Scrub (or un-track, since `companion_mods/` data is
> already gitignored for legal isolation) **before** those files are ever pushed.

---

## Push (ALBW fork)

If remote moved ahead (e.g. README edit on GitHub), merge first — **avoid rebase** if submodule conflicts appear:

```powershell
git fetch upstream
git merge upstream/main -m "Merge remote A-Link-Between-Twilight main"
git push upstream main
```

**Never** `git push origin main` for routine ALBW work unless you intend to push to TwilitRealm.

---

## Verify

```powershell
git status          # clean working tree (ignored files OK)
git log -1 --oneline
```

On GitHub: confirm latest commit on `WadeWinningWilson/A-Link-Between-Twilight` → `main`.

---

## Quick reference

```powershell
# Full happy path (after review)
git add <paths>
git commit -m "..."
# PRIVACY GATE before push (public repo) — scan outgoing files for personal paths / PII:
git grep -niE 'Users\\[A-Za-z0-9._-]+|/Users/[A-Za-z0-9._-]+|D:\\' $(git diff --name-only upstream/main..HEAD)
git fetch upstream
git merge upstream/main    # if push rejected
git push upstream main
```
