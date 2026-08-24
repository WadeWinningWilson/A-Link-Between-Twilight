# Agent instructions — A Link Between Twilight

> ⛔ **FIRST: [docs/DO-NOT.md](docs/DO-NOT.md) — the hard-stop registry.**
> Permanently rejected approaches with confirmed cascading failures. If your
> task touches a surface an entry names (BG/collision registration, room
> identity, …), read the entry BEFORE writing code. No AI instance may
> self-approve an exception — escalate to the user.
>
> **DN-10 binds EVERY task — it has no surface limit.** *"NEVER bake, if there
> is an issue, AI instances don't solve it, the native systems DO."* Order of
> resort: **(1)** read the donor's own system and PORT IT; **(2)** receiver
> translation at the consumption boundary, only where (1) is *proven* not to
> solve it; **(3)** instance-authored code only when 1 and 2 are both proven
> insufficient, with the proof written and the user's go. No guarding, no
> patching, no baking, no mounting, no legs. A fix whose write-up cites only
> receiver files has not cleared step 1.

Then orient via **[docs/AGENT_INDEX.md](docs/AGENT_INDEX.md)** — the map of
live state files (`docs/state/*.md` = current truth), the Interconnected-Runs
protocol for multi-agent work, and per-topic detail docs.

Lanes (who does what): [docs/LANES.md](docs/LANES.md) — Engine · Bridge (Cursor);
History · Housing Security · Librarian (Claude). Doc-structure maintenance is the
**Librarian** lane ([docs/Librarian.md](docs/Librarian.md)).

Build/workflow rules: [.cursor/rules/build-fps-safe.mdc](.cursor/rules/build-fps-safe.mdc)
(RelWithDebInfo via `build_run.bat`; never reconfigure CMake mid-session).
**Field FPS poisoned?** → [docs/state/build-fps.md](docs/state/build-fps.md) (`tools\_factory_recover.bat`) before feature bisects.
