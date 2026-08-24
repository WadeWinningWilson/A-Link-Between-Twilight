from pathlib import Path

p = Path(r"%USERPROFILE%\Documents\dusklight\src\d\d_albw_rental.cpp")
c = p.read_text(encoding="utf-8")

start = c.find("    if (sVisibleList[visIdx].kind == VISIBLE_MIDNA_ARM) {")
if start < 0:
    raise SystemExit("start not found")
end = c.find(
    "    // ============================================\n    // Deity Armor purchase",
    start,
)
if end < 0:
    raise SystemExit("end not found")

nl = "\\" + "n"
insert = f"""    if (sVisibleList[visIdx].kind == VISIBLE_WOLF_CHARGE) {{
        const int price = dAlbwWolfArts_getChargeShopPrice();
        u16 rupees      = dComIfGs_getRupee();
        if (price <= 0) {{
            return;
        }}
        if (rupees < (u16)price) {{
            sStatusMsg          = "Sincerest apologies, but we can't return{nl}that to you for that little..";
            sStatusExpiry       = clock::now() + kPurchaseDataFailure;
            sJustFailedPurchase = true;
            return;
        }}
        if (!dAlbwWolfArts_tryPurchaseChargeUpgrade()) {{
            return;
        }}
        dComIfGs_setRupee(rupees - (u16)price);
        sPurchasedThisSession = true;
        sJustPurchased        = true;
        sStatusMsg            = "Dried meat for a hungry wolf..{nl}Thank you for your patronage!";
        sStatusExpiry         = clock::now() + kPurchaseDataSuccess;
        rebuildActivePages();
        rebuildVisibleList();
        return;
    }}

"""

midna = c[start:end]
midna = midna.replace(
    f"A glove that small, gone at last.{nl}Thank you for your patronage!",
    f"May that Twilight blessing find its mark.{nl}Thank you for your patronage!",
    1,
)

c2 = c[:start] + insert + midna + c[end:]
c2 = c2.replace(
    f"Off it goes with the meat.. and you.{nl}Thank you for your patronage!",
    f"A curious dream-scroll, now yours.{nl}Thank you for your patronage!",
    1,
)

if "tryPurchaseChargeUpgrade" not in c2:
    raise SystemExit("insert failed")

p.write_text(c2, encoding="utf-8", newline="\n")
print("OK")
