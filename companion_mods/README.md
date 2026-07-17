# Companion mods (local only)

Drop **separate-release** mod packs here while developing. Contents of this folder (except this README) are **gitignored** — they do not ship with the main ALBT repo.

**v1 scope (locked):**

| Pack | What |
|------|------|
| `MM-SkullKid-Reskin` | TP Skull Kid (`E_PM`) — **textures/materials only** |
| `Fierce-Deity-Sword` | Sword visual — **only during active Deity Armor session** |

**Install for playtest:** copy a subfolder to:

`%AppData%/TwilitRealm/Dusklight/model_replacements/<ModName>/`

**Research & design:** [docs/research/albt-companion-mods-research.md](../docs/research/albt-companion-mods-research.md)

**Suggested layout per mod:**

```
<ModName>/
  modinfo.ini              # optional
  files/                   # Layer A — mirrors disc paths
  Link_12.bmd                # Layer B — loose BMD (example)
  icons/
  textures/
```

Do not commit Nintendo ripped assets to the main repository.
