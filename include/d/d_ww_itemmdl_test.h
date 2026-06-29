#ifndef D_WW_ITEMMDL_TEST_H
#define D_WW_ITEMMDL_TEST_H

#if TARGET_PC

// Editor/dev helper: replay the Hero's Bow get-item demo (daDitem + PROC_GET_ITEM).
namespace dWwItemmdl {

void requestBowGetItemDemoReplay();
void tickBowGetItemDemoReplay();

// Current replay status for the editor help pane (nullptr if none).
const char* getBowGetItemDemoReplayStatus();

}  // namespace dWwItemmdl

#endif

#endif
