# ALBT Companion Mods

Model packs for **A Link Between Twilight / Dusklight**. The pack zip ships
next to this file; the installer copies it into both places the game reads.

| Pack | What it does |
|---|---|
| `Armogohma Custom` | Reveal model for the refined Armogohma phase-3 fight |
| `MM-SkullKid-Reskin` | Majora's Mask Skull Kid over TP's `E_PM` |
| `Wind Waker Skins` | Wind Waker-styled Link and equipment models |
| `Wind Waker Deku Leaf` | Deku Leaf item model |

## Install

1. Extract the **play zip** and run `dusklight.exe` once (iso / first-run setup).
2. Exit.
3. Double-click **`INSTALL.bat`** in this folder (`companion_mods/_release`).

The installer unpacks `ALBT-companion-mods-v1.zip` and copies the four packs to:

1. **Portable alpha** — `data\model_replacements\` next to `dusklight.exe`
2. **Local Dusklight** — `%APPDATA%\TwilitRealm\Dusklight\model_replacements\`

If it cannot see the alpha folder (source zip and play zip extracted in different
places), it asks you to pick the folder that contains `dusklight.exe`.

## Why the installer touches config.json

Copying the Armogohma model is not enough on its own. The phase-3 reveal is
gated on `game.bossRefinement`, which **defaults to off**, and any pack listed
in `game.customModelsDisabled` is ignored even when its files are present. The
installer sets the first and removes these packs from the second, then makes
sure each is un-prefixed in `game.customModelsOrder`.

Each `config.json` it changes is saved as `config.json.bak` first.

## Uninstall

Delete the four folders from each `model_replacements\` it wrote. To undo the
settings change, restore `config.json.bak` or turn Boss Refinement off in the
in-game menu.
