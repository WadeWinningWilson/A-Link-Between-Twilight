# ALBT Companion Mods

Unzip these folders yourself. **`INSTALL.bat` is only a reminder** — it does not
install into a portable alpha (that build reads `data\` next to the exe, not
AppData).

| Pack | What it does |
|---|---|
| `Armogohma Custom` | Reveal model for the refined Armogohma phase-3 fight |
| `MM-SkullKid-Reskin` | Majora's Mask Skull Kid over TP's `E_PM` |
| `Wind Waker Skins` | Wind Waker-styled Link and equipment models |
| `Wind Waker Deku Leaf` | Deku Leaf item model |

## Where to put them

Copy each pack folder (the folder itself, not just the files inside) into
**either or both**:

1. **Portable alpha** (the zip you downloaded):

   `build\windows-msvc-relwithdebinfo\data\model_replacements\`

2. **Local Dusklight** (AppData):

   `%APPDATA%\TwilitRealm\Dusklight\model_replacements\`

Example after placing Armogohma:

`...\model_replacements\Armogohma Custom\B_gm_37.bmd`

If `model_replacements` is missing, create it next to `texture_replacements`.

## In-game gates

The Armogohma reveal also needs **Boss Refinement** and **Custom Models** on
in Settings. A pack listed as disabled under Custom Models will not load even
when the files are present.
