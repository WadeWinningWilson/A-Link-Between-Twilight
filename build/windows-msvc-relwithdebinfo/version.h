#ifndef VERSION_H
#define VERSION_H

#define DUSK_WC_DESCRIBE "v1.4.1-631-dirty"
#define DUSK_VERSION_STRING "1.4.1.631"

#define DUSK_WC_BRANCH "integrate/dusk-api-coexist"
#define DUSK_WC_REVISION "71a32518568c3fd61968e401571e9ce2686a9ee3"
#define DUSK_WC_DATE "Sun Aug 23 14:58:10 2026 -0700"
#define DUSK_BUILD_TYPE "RelWithDebInfo"

#if defined(__x86_64__) || defined(_M_AMD64)
#define DUSK_ARCH "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
#define DUSK_ARCH "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define DUSK_ARCH "arm64"
#endif

#define DUSK_PLATFORM_NAME "win32"
#define DUSK_DLPACKAGE "dusklight-v1.4.1-631-dirty-" DUSK_PLATFORM_NAME "-" DUSK_ARCH

#define DUSK_SENTRY_DSN ""
#define DUSK_SENTRY_ENVIRONMENT "development"

#endif
