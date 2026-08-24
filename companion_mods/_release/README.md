# ALBT Companion Mods

Model packs for **A Link Between Twilight / Dusklight**. Separate release: these
are not part of the main repository.

| Pack | What it does |
|---|---|
| `Armogohma Custom` | Reveal model for the **refined Armogohma** phase-3 fight |
| `MM-SkullKid-Reskin` | Majora's Mask Skull Kid over TP's `E_PM` |
| `Wind Waker Skins` | Wind Waker-styled Link and equipment models |
| `Wind Waker Deku Leaf` | Deku Leaf item model |

## Install

Double-click **`INSTALL.bat`**. It copies the packs to
`%APPDATA%\TwilitRealm\Dusklight\model_replacements\` and enables them.

Run the game **once** first if you have never launched it, so the config folder
exists.

## Why the installer touches config.json

Copying the Armogohma model is not enough on its own. The phase-3 reveal is
gated on `game.bossRefinement`, which **defaults to off**, and any pack listed
in `game.customModelsDisabled` is ignored even when its files are present. The
installer sets the first and removes these packs from the second, then makes
sure each is un-prefixed in `game.customModelsOrder`.

Your previous `config.json` is saved as `config.json.bak` before any change.

## Uninstall

Delete the four folders from `model_replacements\`. To undo the settings
change, restore `config.json.bak` or turn Boss Refinement off in the in-game
Dusklight menu.
