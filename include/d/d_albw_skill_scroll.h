#ifndef D_ALBW_SKILL_SCROLL_H
#define D_ALBW_SKILL_SCROLL_H

// ============================================
// NEW CODE — ALBW Port — Hidden Skills page-2 scrolls.
// See docs/albw-junior-postman-mail-handoff.md (Hidden Skills note).
// ============================================
#if TARGET_PC

#include "dolphin/types.h"

// Max entries shown on the ALBW page (reuses the same 7 BLO rows as vanilla).
static constexpr int kAlbwSkillScrollPageCap = 7;

// ============================================
// Detail-body auto-flow. Bodies are stored as plain prose (no manual
// line breaks); dAlbwSkillScroll_buildDetailPages() wraps them on word
// boundaries to the pane's line width and splits them into pages that
// fit the 4-line description pane ('mg_e4lin').
// ============================================
static constexpr int kAlbwSkillScrollLineChars  = 45;    // wrapped-line character budget
static constexpr int kAlbwSkillScrollPageLines  = 4;     // lines per detail page
static constexpr int kAlbwSkillScrollMaxPages   = 4;     // hard cap; overflow is dropped
static constexpr int kAlbwSkillScrollPageBufLen = 0x200; // per-page text buffer

enum dAlbwSkillScrollId {
    dAlbwSkillScroll_Meter = 0,
    dAlbwSkillScroll_FocusedArts1 = 1,
    dAlbwSkillScroll_FocusedArts2 = 2,
    dAlbwSkillScroll_FocusedArts3 = 3,
    dAlbwSkillScroll_Count,
};

struct dAlbwSkillScrollEntry {
    dAlbwSkillScrollId id;
    const char* listTitle;    // left column / subtitle
    const char* listName;     // right column / scroll name
    const char* detailTitle;  // detail header
    const char* detailBody;   // plain prose; auto-wrapped + paginated at display time
};

// Word-wrap i_body into up to i_maxPages pages of kAlbwSkillScrollPageLines
// lines (kAlbwSkillScrollLineChars chars each). Explicit '\n' in the source
// forces a line break. Returns the page count (always >= 1 for valid output).
int dAlbwSkillScroll_buildDetailPages(const char* i_body,
                                      char o_pages[][kAlbwSkillScrollPageBufLen],
                                      int i_maxPages);

// True if any ALBW page-2 scroll should appear (meter always; FA when purchased).
bool dAlbwSkillScroll_hasAny();

// Ordered list of currently unlocked ALBW scrolls (meter first, then FA tiers).
int dAlbwSkillScroll_getCount();
bool dAlbwSkillScroll_getEntry(int index, dAlbwSkillScrollEntry* out);

// Collect-menu icon: show Hidden Skills when ALBW scrolls exist even with no HS flags.
bool dAlbwSkillScroll_shouldShowCollectIcon();

#endif  // TARGET_PC

#endif /* D_ALBW_SKILL_SCROLL_H */
