// ============================================
// NEW CODE — ALBW Port — Hidden Skills page-2 scrolls.
// ============================================
#include "d/d_albw_skill_scroll.h"

#if TARGET_PC

#include "d/d_focused_arts.h"

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

}  // namespace

bool dAlbwSkillScroll_hasAny() {
    return dAlbwSkillScroll_getCount() > 0;
}

int dAlbwSkillScroll_getCount() {
    int count = 1;  // ALBW meter is always the first page-2 entry.
    const int faTiers = dFocusedArts_getPurchasedTier();
    if (faTiers > 0) {
        count += faTiers;
        if (count > kAlbwSkillScrollPageCap) {
            count = kAlbwSkillScrollPageCap;
        }
    }
    return count;
}

bool dAlbwSkillScroll_getEntry(int index, dAlbwSkillScrollEntry* out) {
    if (out == NULL || index < 0 || index >= dAlbwSkillScroll_getCount()) {
        return false;
    }

    if (index == 0) {
        *out = kMeterScroll;
        return true;
    }

    const int faIndex = index - 1;
    if (faIndex < 0 || faIndex >= 3) {
        return false;
    }
    *out = kFocusedArtsScrolls[faIndex];
    return true;
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
