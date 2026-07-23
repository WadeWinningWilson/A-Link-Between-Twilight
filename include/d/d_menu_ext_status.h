#ifndef D_MENU_EXT_STATUS_H
#define D_MENU_EXT_STATUS_H

#if TARGET_PC

#include "d/d_drawlist.h"
#include "d/d_ext_status.h"

struct CSTControl;
class STControl;

/** Paused Collect sibling — Tools / Quest / Atlas chrome (registry-driven). */
class dMenu_ExtStatus_c : public dDlst_base_c {
public:
    dMenu_ExtStatus_c(STControl* stick, CSTControl* cstick);
    virtual ~dMenu_ExtStatus_c() {}

    void _create();
    void _delete();
    void _move();
    virtual void draw();

    dExtStatusTab getTab() const { return mTab; }
    void setTab(dExtStatusTab tab);

    /** -1 = stay; 0 = return Collect (left wrap); 1 = return Collect (right wrap) */
    s8 getCollectHandoff() const { return mCollectHandoff; }
    void clearCollectHandoff() { mCollectHandoff = -1; }

    bool wantsClose() const { return mWantsClose; }

private:
    STControl* mpStick;
    CSTControl* mpCStick;
    dExtStatusTab mTab;
    u8 mCursor;
    s8 mCollectHandoff;
    bool mWantsClose;
};

#endif  // TARGET_PC

#endif /* D_MENU_EXT_STATUS_H */
