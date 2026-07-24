// ============================================
// NEW CODE — ALBW Port — Hidden Skills page-2 scrolls.
// ============================================
#include "d/d_albw_skill_scroll.h"

#if TARGET_PC

#include "d/d_albw_mail.h"
#include "d/d_focused_arts.h"
#include "d/d_albw_wolf_stun.h"

namespace {

// Bodies are plain prose — no manual line breaks. The menu wraps and
// paginates them at display time (dAlbwSkillScroll_buildDetailPages).
const dAlbwSkillScrollEntry kMeterScroll = {
    dAlbwSkillScroll_Meter,
    "Combat Basics",
    "ALBW Magic Meter",
    "ALBW Magic Meter",
    "Similar to another Green tunic wearing Hero, your journey has become "
    "one of endurance! The magic meter fuels swordplay, agility, item use, "
    "and even secret techniques. It recovers when you aren't guarding, but "
    "you will also need to focus in when you expend all your energy to get "
    "your meter back to full. A tired hero cannot swing his sword!",
};

const dAlbwSkillScrollEntry kAboutItemsScroll = {
    dAlbwSkillScroll_AboutItems,
    "Mail Scroll",
    "About Your Items",
    "About Your Items",
    "When Link gets tired, never fear, you still have your tools! Each will "
    "aid you in different ways when you're in need, with many even increasing "
    "in their power and effectiveness. Be sure to keep an eye on your meter "
    "to see how it reacts!",
};

const dAlbwSkillScrollEntry kParryingScroll = {
    dAlbwSkillScroll_Parrying,
    "Mail Scroll",
    "Parrying",
    "Parrying",
    "Tired of being smacked around by enemies larger, faster, or more "
    "powerful than you? That's why any true Hero needs to obtain mastery "
    "over their shield. Block enemy attacks at the right time, or else your "
    "shield- or even you- may get damaged!",
};

const dAlbwSkillScrollEntry kFocusedArtsScrolls[3] = {
    {
        dAlbwSkillScroll_FocusedArts1,
        "Shop Scroll",
        "Focused Arts I",
        "Focused Arts I",
        "Increase your power through striking your foe without fault! Spend "
        "a charge to empower the skills the legendary hero taught you. "
        "Higher tiers offer greater power.",
    },
    {
        dAlbwSkillScroll_FocusedArts2,
        "Shop Scroll",
        "Focused Arts II",
        "Focused Arts II",
        "Your level of focus has grown to new depths! Unleash your hidden "
        "skills with more raw power!",
    },
    {
        dAlbwSkillScroll_FocusedArts3,
        "Shop Scroll",
        "Focused Arts III",
        "Focused Arts III",
        "Now having honed your sword techniques, new abilities flow through "
        "your blade. Finish your enemies in style by spending all of your "
        "meter. Test your hidden skills to see what special boons they "
        "award as you obliterate your enemies.",
    },
};

const dAlbwSkillScrollEntry kWolfHowlScroll = {
    dAlbwSkillScroll_WolfHowl,
    "Shop Scroll",
    "Wolf Howl",
    "Wolf Howl",
    "The light spirits of Ordona and Farore have bestowed on you a new "
    "ability! Let your howl ring with more divine power as you learn songs!",
};

const dAlbwSkillScrollEntry kMidnaGraspScroll = {
    dAlbwSkillScroll_MidnaGrasp,
    "Shop Scroll",
    "Midna's Grasp",
    "Midna's Grasp",
    "Grateful for the generosity of the light spirits, Midna now aids in "
    "battle to protect you on all sides!",
};

const dAlbwSkillScrollEntry kWolfChargeScroll = {
    dAlbwSkillScroll_WolfCharge,
    "Shop Scroll",
    "Wolf Charge",
    "Wolf Charge",
    "Tasty meat from an abandoned town.",
};

}  // namespace

bool dAlbwSkillScroll_hasAny() {
    return dAlbwSkillScroll_getCount() > 0;
}

int dAlbwSkillScroll_getCount() {
    int count = 1;  // ALBW meter is always the first page-2 entry.
    if (dAlbwMail_hasTutorialScrolls()) {
        count += 2;  // About Your Items + Parrying
    }
    const int faTiers = dFocusedArts_getPurchasedTier();
    if (faTiers > 0) {
        count += faTiers;
    }
    if (dAlbwWolfArts_isHowlUnlocked()) {
        count++;
    }
    if (dAlbwWolfArts_isArmUnlocked()) {
        count++;
    }
    if (dAlbwWolfArts_isChargeUpgradeUnlocked()) {
        count++;
    }
    return count;
}

bool dAlbwSkillScroll_getEntry(int index, dAlbwSkillScrollEntry* out) {
    if (out == NULL || index < 0 || index >= dAlbwSkillScroll_getCount()) {
        return false;
    }

    int cursor = 0;
    if (index == cursor++) {
        *out = kMeterScroll;
        return true;
    }

    if (dAlbwMail_hasTutorialScrolls()) {
        if (index == cursor++) {
            *out = kAboutItemsScroll;
            return true;
        }
        if (index == cursor++) {
            *out = kParryingScroll;
            return true;
        }
    }

    const int faTiers = dFocusedArts_getPurchasedTier();
    for (int i = 0; i < faTiers && i < 3; i++) {
        if (index == cursor++) {
            *out = kFocusedArtsScrolls[i];
            return true;
        }
    }

    if (dAlbwWolfArts_isHowlUnlocked() && index == cursor++) {
        *out = kWolfHowlScroll;
        return true;
    }
    if (dAlbwWolfArts_isArmUnlocked() && index == cursor++) {
        *out = kMidnaGraspScroll;
        return true;
    }
    if (dAlbwWolfArts_isChargeUpgradeUnlocked() && index == cursor++) {
        *out = kWolfChargeScroll;
        return true;
    }

    return false;
}

bool dAlbwSkillScroll_shouldShowCollectIcon() {
    return dAlbwSkillScroll_hasAny();
}

// ============================================
// NEW CODE — ALBW Port
// Detail-body auto-flow: greedy word wrap to kAlbwSkillScrollLineChars
// columns, kAlbwSkillScrollPageLines lines per page. Words longer than a
// full line are hard-split. Explicit '\n' in the source forces a break.
// Text past the page cap is dropped.
// ============================================
namespace {

inline void pageAppend(char* io_buf, int& io_len, char i_char) {
    if (io_len < kAlbwSkillScrollPageBufLen - 1) {
        io_buf[io_len++] = i_char;
        io_buf[io_len] = '\0';
    }
}

}  // namespace

int dAlbwSkillScroll_buildDetailPages(const char* i_body,
                                      char o_pages[][kAlbwSkillScrollPageBufLen],
                                      int i_maxPages) {
    if (o_pages == NULL || i_maxPages <= 0) {
        return 0;
    }
    for (int p = 0; p < i_maxPages; p++) {
        o_pages[p][0] = '\0';
    }
    if (i_body == NULL || *i_body == '\0') {
        return 1;
    }

    int page = 0;  // current page index
    int line = 0;  // completed lines on the current page
    int col = 0;   // characters on the current line
    int len = 0;   // characters written to the current page buffer

    // Advance to the next line, rolling to a new page when full.
    // Returns false once the page cap is exhausted.
    const auto breakLine = [&]() -> bool {
        line++;
        col = 0;
        if (line >= kAlbwSkillScrollPageLines) {
            page++;
            line = 0;
            len = 0;
            return page < i_maxPages;
        }
        pageAppend(o_pages[page], len, '\n');
        return true;
    };

    const char* s = i_body;
    while (*s != '\0' && page < i_maxPages) {
        if (*s == ' ') {  // inter-word spaces are re-emitted as needed
            s++;
            continue;
        }
        if (*s == '\n') {  // authored hard break
            s++;
            if (!breakLine()) {
                break;
            }
            continue;
        }

        int wordLen = 0;
        while (s[wordLen] != '\0' && s[wordLen] != ' ' && s[wordLen] != '\n') {
            wordLen++;
        }

        // Word (plus its separating space) doesn't fit: wrap first.
        if (col > 0 && col + 1 + wordLen > kAlbwSkillScrollLineChars) {
            if (!breakLine()) {
                break;
            }
        }

        if (col > 0) {
            pageAppend(o_pages[page], len, ' ');
            col++;
        }
        for (int c = 0; c < wordLen && page < i_maxPages; c++) {
            if (col >= kAlbwSkillScrollLineChars) {  // hard-split oversized word
                if (!breakLine()) {
                    break;
                }
            }
            pageAppend(o_pages[page], len, s[c]);
            col++;
        }
        s += wordLen;
    }

    if (page >= i_maxPages) {
        return i_maxPages;
    }
    if (len == 0 && page > 0) {
        return page;  // don't report an empty trailing page
    }
    return page + 1;
}
// ============================================
// NEW CODE ENDS HERE
// ============================================

#endif  // TARGET_PC
