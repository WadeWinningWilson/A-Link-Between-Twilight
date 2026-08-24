#ifndef D_DEMO_LEFTOVER_VIEWER_H
#define D_DEMO_LEFTOVER_VIEWER_H

#if TARGET_PC

// Demo leftover viewer — all unique BMD/BDL from retail Demo*.arc.
// Editor ALBW tab filters by category; play-tick loads the arc; Link draw
// renders at Link's feet. No placeable / AI actor.

namespace dDemoLeftoverViewer {

enum Category {
    CAT_GOLD = 0,  // original / high / henkei / demo00 crumbs / beta cast
    CAT_LINK = 1,
    CAT_MIDNA = 2,
    CAT_WOLF = 3,
    CAT_NPC = 4,
    CAT_BOSS = 5,
    CAT_PROP = 6,
    CAT_COUNT = 7,
    CAT_ALL = 8,  // filter only — not stored on entries
};

struct Entry {
    int category;  // Category (not CAT_ALL)
    const char* kind;   // "body" / "face" / "hand" / "head" / "piece" / "mesh"
    const char* label;  // UI label with [kind] prefix
    const char* arcName;
    const char* resName;
};

int entryCount();
const Entry* entry(int index);

int selectedIndex();
void setSelectedIndex(int index);

int categoryFilter();  // CAT_* including CAT_ALL
void setCategoryFilter(int category);
const char* categoryName(int category);

// Indices into the full catalog that match the current category filter.
int filteredCount();
int filteredCatalogIndex(int filteredIndex);  // maps UI row -> catalog index

void requestSpawn();
void requestDespawn();
void tick();
void draw();

const char* status();

}  // namespace dDemoLeftoverViewer

#endif  // TARGET_PC

#endif  // D_DEMO_LEFTOVER_VIEWER_H
