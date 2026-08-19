#pragma once

class dCamera_c;

#define WW_CAM_ALG_CRAWL 20

// Donor crawlCamera. Vanilla engine_tbl[19] (test2Camera) is the socket
// because the host has no extra-engine hook and indexes the table directly.
bool wwCam_runCrawl(dCamera_c* i_cam, int i_style);
