#pragma once

class dCamera_c;

// Donor nextType/nextMode/style latch against the WW tables. Returns true
// when this frame's selection ran (WW host stage, attached). Vanilla Run
// then skips its own nextType/nextMode/style block via the matching hooks.
bool wwCamSelect(dCamera_c* i_cam);
bool wwCam_isAttached();
bool wwCam_inSelectLatch();
