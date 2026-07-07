#include "JSystem/JSystem.h" // IWYU pragma: keep

#include "JSystem/JAudio2/JASWaveArcLoader.h"
#include "JSystem/JAudio2/JASDvdThread.h"
#include "JSystem/JAudio2/JASTaskThread.h"
#include "JSystem/JAudio2/JASMutex.h"
#include "JSystem/JKernel/JKRDvdAramRipper.h"
#include <cstring>
#include <os.h>
#include <stdint.h>

#include "dusk/string.hpp"
// ============================================
// NEW CODE — ALBW Port / Dusklight
// Custom-audio shadow-wave hooks: record each wave arc's entrynum->name, load
// the mod twin into RAM as the vanilla bank loads to ARAM, and drop it on erase.
// See dusk/custom_assets.hpp and dusk/audio/DuskDsp.hpp. All no-ops when the mod
// provides no twin (or the D_ALBW_AUDIO_SHADOW kill switch is off).
// ============================================
#include "dusk/custom_assets.hpp"

JASHeap* JASWaveArcLoader::sAramHeap;

JASHeap* JASWaveArcLoader::getRootHeap() {
    if (JASWaveArcLoader::sAramHeap) {
        return JASWaveArcLoader::sAramHeap;
    }
    return JASKernel::getAramHeap();
}

char JASWaveArcLoader::sCurrentDir[DIR_MAX] = "/AudioRes/Waves/";

void JASWaveArcLoader::setCurrentDir(char const* dir) {
    JUT_ASSERT(40, std::strlen(dir) < DIR_MAX - 1);
    SAFE_STRCPY(sCurrentDir, dir);
    u32 len = strlen(sCurrentDir);
    if (sCurrentDir[len - 1] != '/') {
        JUT_ASSERT(45, len + 1 < DIR_MAX);
        sCurrentDir[len] = '/';
        sCurrentDir[len + 1] = '\0';
    }
}

char* JASWaveArcLoader::getCurrentDir() {
    return sCurrentDir;
}

JASWaveArc::JASWaveArc() : mHeap(this) {
    _48 = 0;
    mStatus = 0;
    mEntryNum = -1;
    mFileLength = 0;
    _58 = 0;
    _5a = 0;
    OSInitMutex(&mMutex);
}

JASWaveArc::~JASWaveArc() {}

bool JASWaveArc::loadSetup(u32 param_0) {
    JASMutexLock mutexLock(&mMutex);
    if (_58 != param_0) {
        return false;
    }
    if (mStatus != 1) {
        return false;
    }
    _48 = 1;
    mStatus = 2;
    return true;
}

bool JASWaveArc::eraseSetup() {
    JASMutexLock mutexLock(&mMutex);
    if (mStatus == 0) {
        return false;
    }
    if (mStatus == 1) {
        mStatus = 0;
        return false;
    }
    _48 = 0;
    mStatus = 0;
    return true;
}

void JASWaveArc::loadToAramCallback(void* this_) {
    loadToAramCallbackParams* tmp = (loadToAramCallbackParams*)this_;
    JASWaveArc* wavArc = tmp->mWavArc;
    if (JKRDvdAramRipper::loadToAram(tmp->mEntryNum, tmp->mBase, EXPAND_SWITCH_UNKNOWN0, 0, 0, NULL) == NULL) {
        JUT_WARN(129, "%s", "loadToAram Failed");
        return;
    }
    // Vanilla bank is now resident at [mBase, mBase+mFileLength). If a mod
    // provides a size-matched twin, load it into RAM and register the redirect.
    dusk::custom_assets::acquire_audio_shadow(
        tmp->mEntryNum, static_cast<unsigned int>(tmp->mBase),
        static_cast<unsigned int>(wavArc->mFileLength));
    wavArc->_5a--;
    if (wavArc->loadSetup(tmp->_c)) {
        wavArc->onLoadDone();
    }
}

bool JASWaveArc::sendLoadCmd() {
    JASMutexLock mutexLock(&mMutex);
    _48 = 0;
    mStatus = 1;
    loadToAramCallbackParams commandInfo;
    commandInfo.mWavArc = this;
    commandInfo.mEntryNum = mEntryNum;
    commandInfo.mBase = (uintptr_t)mHeap.getBase();
    commandInfo._c = ++_58;

    _5a++;

    if (JASDvd::getThreadPointer()->sendCmdMsg(loadToAramCallback, &commandInfo, sizeof(commandInfo)) == 0) {
        JUT_WARN(193, "%s", "sendCmdMsg loadToAramCallback Failed");
        mHeap.free();
        return false;
    }
    return true;
}

bool JASWaveArc::load(JASHeap* heap) {
    if (mEntryNum < 0) {
        return false;
    }
    JASMutexLock mutexLock(&mMutex);
    if (mStatus != 0) {
        return false;
    }
    if (heap == NULL) {
        heap = JASWaveArcLoader::getRootHeap();
    }
    if (mHeap.alloc(heap, mFileLength) == false) {
        return false;
    }
    return sendLoadCmd();
}

bool JASWaveArc::loadTail(JASHeap* heap) {
    if (mEntryNum < 0) {
        return false;
    }
    JASMutexLock mutexLock(&mMutex);
    if (mStatus != 0) {
        return false;
    }
    if (heap == NULL) {
        heap = JASWaveArcLoader::getRootHeap();
    }
    if (mHeap.allocTail(heap, mFileLength) == false) {
        return false;
    }
    return sendLoadCmd();
}

bool JASWaveArc::erase() {
    // Drop any mod-twin redirect for this bank's ARAM base BEFORE the vanilla
    // heap is freed (the base is reused by later banks, so this must always run).
    // C-style cast mirrors sendLoadCmd's `(uintptr_t)mHeap.getBase()` so this
    // matches the u32 base registered at load time regardless of getBase()'s type.
    dusk::custom_assets::release_audio_shadow((u32)(uintptr_t)mHeap.getBase());
    return mHeap.free();
}

void JASWaveArc::onDispose() {
    if (eraseSetup()) {
        onEraseDone();
    }
}

void JASWaveArc::setEntryNum(s32 entryNum) {
    DVDFileInfo fileInfo;
    if (entryNum < 0) {
        return;
    }
    if (DVDFastOpen(entryNum, &fileInfo) == FALSE) {
        return;
    }
    mFileLength = fileInfo.length;
    DVDClose(&fileInfo);
    mEntryNum = entryNum;
}

void JASWaveArc::setFileName(char const* fileName) {
    char* currentDir = JASWaveArcLoader::getCurrentDir();
    size_t length = strlen(currentDir);
    length = length + strlen(fileName);
    char* path = JKR_NEW_ARRAY_ARGS(char, length + 1, JASKernel::getSystemHeap(), -4);
    JUT_ASSERT(322, path);
    SAFE_STRCPY_BOUNDED(path, length + 1, currentDir);
    SAFE_STRCAT_BOUNDED(path, length + 1, fileName);
    path[length] = '\0';
    int entryNum = DVDConvertPathToEntrynum(path);
    JKR_DELETE_ARRAY(path);
    if (entryNum < 0) {
        return;
    }
    setEntryNum(entryNum);
    // Record entrynum -> leaf name so the ARAM-load hook can find the mod twin.
    dusk::custom_assets::note_audio_wave_arc(entryNum, fileName);
}
