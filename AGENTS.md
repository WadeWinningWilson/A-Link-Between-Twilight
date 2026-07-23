# Agent instructions — A Link Between Dusklight

> ⛔ **FIRST: [docs/DO-NOT.md](docs/DO-NOT.md) — the hard-stop registry.**
> Permanently rejected approaches with confirmed cascading failures. If your
> task touches a surface an entry names (BG/collision registration, room
> identity, …), read the entry BEFORE writing code. No AI instance may
> self-approve an exception — escalate to the user.

Then orient via **[docs/AGENT_INDEX.md](docs/AGENT_INDEX.md)** — the map of
live state files (`docs/state/*.md` = current truth), the Interconnected-Runs
protocol for multi-agent work, and per-topic detail docs.

Build/workflow rules: [.cursor/rules/build-fps-safe.mdc](.cursor/rules/build-fps-safe.mdc)
(RelWithDebInfo via `build_run.bat`; never reconfigure CMake mid-session).
