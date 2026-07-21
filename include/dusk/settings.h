#pragma once

#include <array>

#include "dusk/config_var.hpp"
#include "dusk/ui/controls.hpp"

namespace dusk {

using namespace config;

enum class BloomMode : int {
    Off = 0,
    Classic = 1,
    Dusk = 2,
};

enum class DepthOfFieldMode : int {
    Off = 0,
    Classic = 1,
    Dusk = 2,
};

enum class Resampler : int {
    Bilinear = 0,
    Area = 1,
};

enum class GameLanguage : u8 {
    English = OS_LANGUAGE_ENGLISH,
    German = OS_LANGUAGE_GERMAN,
    French = OS_LANGUAGE_FRENCH,
    Spanish = OS_LANGUAGE_SPANISH,
    Italian = OS_LANGUAGE_ITALIAN,
};

enum class DiscVerificationState : u8 {
    Unknown = 0,
    Success,
    HashMismatch,
};

enum class FrameInterpMode : u8 {
    Off = 0,
    Capped = 1,
    Unlimited = 2,
};

enum class MenuScaling : u8 {
    GameCube = 0,
    Wii = 1,
    Dusklight = 2,
};

enum class MagicArmorMode : u8 {
    NORMAL = 0,
    ON_DAMAGE = 1,
    DOUBLE_DEFENSE = 2,
    INVINCIBLE = 3,
    COSMETIC = 4,
    ALBW = 5,
};

// ALBW parry/bash charge HUD icon style.
enum class ParryIcons : u8 {
    SpurOnly = 0,    // original Epona-spur graphic
    SpurShield = 1,  // spur starburst + shield emblem composited
    ShieldOnly = 2,  // shield emblem only (starburst hidden)
};

// Shield aux HUD visibility (durability meter + parry charge icons).
enum class ShieldHudVisibility : u8 {
    Off = 0,               // guard + 2s linger (vanilla ALBW behavior)
    DurabilityAlways = 1,  // durability meter always on when eligible
    ParryAlways = 2,       // parry charge icons always on when eligible
    BothAlways = 3,        // both always on when eligible
};

// ============================================
// NEW CODE — ALBW Port (Lies of Link HUD mode)
// Lies of Link HUD layout/health style. Off keeps the vanilla corner HUD; the two
// On modes share the LoP relayout and differ only in how life is shown.
// ============================================
enum class LopHudMode : u8 {
    Off = 0,            // vanilla TP corner layout
    VanillaHearts = 1,  // LoP relayout, keep the heart containers
    HealthBar = 2,      // LoP relayout, Lies-of-P health bar instead of hearts
};
// ============================================
// NEW CODE ENDS HERE
// ============================================

// Extra Item Slot + optional D-Pad Quick Swap (Midna left, Z item, field equip/transform).
enum class ExtraItemSlotMode : int {
    Off = 0,
    ExtraOnly = 1,
    ExtraAndQuickSwap = 2,
};

// Off = vanilla reclaim shop; TrueAlbw = full rental catalog; TrueTest = enables TRUETEST new-save
// prompt and per-save world bootstrap (see docs/TrueALBWWorld.md).
enum class TrueAlbwMode : int {
    Off = 0,
    TrueAlbw = 1,
    TrueTest = 2,
};

// Cap Wear: which headpiece Link wears, applied GLOBALLY to every outfit (the sumo body AND
// the native clothes), decoupled so each cap resolves on any base via the cap donor.
//   Off   — native: each outfit keeps its own hat (the sumo body keeps its topknot).
//   None  — force the bald sumo topknot (alSumou 0x33) on every outfit.
//   Green — Link's cap   al_head (Kmdl).
//   Red   — Magic helmet ml_head (Mmdl).
//   Blue  — Zora helmet  zl_head (Zmdl).
// (Green/Red/Blue are the "Link cap" modes; see dAlbwSumoTest_wantLinkCap().)
enum class CapWearMode : u8 {
    Off = 0,
    None = 1,
    Green = 2,
    Red = 3,
    Blue = 4,
};

// Which WW itemmdl mesh replaces Link's held/worn item (dev skin selector). Single-select:
// the chosen item's slot is skinned, others stay vanilla. Bow/Hookshot are held-in-hand
// (bow-analog). IronBoots is worn/rigged — reversible via this selector while its mesh path lands.
enum class WwHeldSkinMode : u8 {
    Off = 0,
    Bow = 1,
    IronBoots = 2,
    Hookshot = 3,
};

// Focused Arts playtest cheat: tier 3 without shop; WithDebug adds an in-game HUD overlay.
enum class FocusedArtsCheatMode : int {
    Off = 0,
    On = 1,
    WithDebug = 2,
    OnMaxBank = 3,
    WithDebugMaxBank = 4,
};

namespace config {
template <>
struct ConfigEnumRange<BloomMode> {
    static constexpr auto min = BloomMode::Off;
    static constexpr auto max = BloomMode::Dusk;
};

template <>
struct ConfigEnumRange<DepthOfFieldMode> {
    static constexpr auto min = DepthOfFieldMode::Off;
    static constexpr auto max = DepthOfFieldMode::Dusk;
};

template <>
struct ConfigEnumRange<Resampler> {
    static constexpr auto min = Resampler::Bilinear;
    static constexpr auto max = Resampler::Area;
};

template <>
struct ConfigEnumRange<GameLanguage> {
    static constexpr auto min = GameLanguage::English;
    static constexpr auto max = GameLanguage::Italian;
};

template <>
struct ConfigEnumRange<DiscVerificationState> {
    static constexpr auto min = DiscVerificationState::Unknown;
    static constexpr auto max = DiscVerificationState::HashMismatch;
};

template <>
struct ConfigEnumRange<FrameInterpMode> {
    static constexpr auto min = FrameInterpMode::Off;
    static constexpr auto max = FrameInterpMode::Unlimited;
};

template <>
struct ConfigEnumRange<MenuScaling> {
    static constexpr auto min = MenuScaling::GameCube;
    static constexpr auto max = MenuScaling::Dusklight;
};

template <>
struct ConfigEnumRange<MagicArmorMode> {
    static constexpr auto min = MagicArmorMode::NORMAL;
    static constexpr auto max = MagicArmorMode::ALBW;
};

template <>
struct ConfigEnumRange<ParryIcons> {
    static constexpr auto min = ParryIcons::SpurOnly;
    static constexpr auto max = ParryIcons::ShieldOnly;
};

template <>
struct ConfigEnumRange<ShieldHudVisibility> {
    static constexpr auto min = ShieldHudVisibility::Off;
    static constexpr auto max = ShieldHudVisibility::BothAlways;
};

template <>
struct ConfigEnumRange<LopHudMode> {
    static constexpr auto min = LopHudMode::Off;
    static constexpr auto max = LopHudMode::HealthBar;
};

template <>
struct ConfigEnumRange<ExtraItemSlotMode> {
    static constexpr auto min = ExtraItemSlotMode::Off;
    static constexpr auto max = ExtraItemSlotMode::ExtraAndQuickSwap;
};

template <>
struct ConfigEnumRange<TrueAlbwMode> {
    static constexpr auto min = TrueAlbwMode::Off;
    static constexpr auto max = TrueAlbwMode::TrueTest;
};

template <>
struct ConfigEnumRange<FocusedArtsCheatMode> {
    static constexpr auto min = FocusedArtsCheatMode::Off;
    static constexpr auto max = FocusedArtsCheatMode::WithDebugMaxBank;
};

template <>
struct ConfigValueTraits<ui::ControlLayout> {
    static constexpr bool enabled = true;
};
}  // namespace config

// Persistent user settings

struct UserSettings {
    // Program settings

    struct {
        // Video
        ConfigVar<bool> enableFullscreen;
        ConfigVar<bool> enableVsync;
        ConfigVar<bool> lockAspectRatio;
        ConfigVar<bool> enableFpsOverlay;
        ConfigVar<int> fpsOverlayCorner;
        ConfigVar<int> maxFrameRate;
        ConfigVar<bool> rememberWindowSize;
        ConfigVar<int> lastWindowWidth;
        ConfigVar<int> lastWindowHeight;
    } video;

    struct {
        // Audio
        ConfigVar<int> masterVolume;
        ConfigVar<int> mainMusicVolume;
        ConfigVar<int> subMusicVolume;
        ConfigVar<int> soundEffectsVolume;
        ConfigVar<int> fanfareVolume;
        ConfigVar<bool> enableReverb;
        ConfigVar<bool> enableHrtf;
        ConfigVar<bool> menuSounds;
    } audio;

    // Game settings

    struct {
        ConfigVar<GameLanguage> language;

        // QoL
        ConfigVar<bool> enableQuickTransform;
        // Off | Extra only (Midna + Z item) | Extra + Quick Swap (d-pad equip/transform).
        ConfigVar<ExtraItemSlotMode> extraItemSlotMode;
        // Hold OPEN_ITEM_WHEEL → filtered tools → release assigns Z only (needs Extra Item Slot).
        ConfigVar<bool> quickEquipWheel;
        ConfigVar<bool> hideTvSettingsScreen;
        ConfigVar<bool> biggerWallets;
        ConfigVar<bool> noReturnRupees;
        ConfigVar<bool> disableRupeeCutscenes;
        ConfigVar<bool> noSwordRecoil;
        // ============================================
        // NEW CODE — ALBW Port (Phase 1 manual shield, Dawnlight 9ff9d35 / kamilink7)
        // ============================================
        ConfigVar<bool> manualShielding;
        // Phase 4: LoP-style parry, bash charges, ALBW rewards/penalties. Off = vanilla guard slip only.
        ConfigVar<bool> shieldParryCombat;
        // Helm punish flow, hidden-skill dispatch, ALBW meter costs, Jump Strike charge gate.
        // Always on (ALBW core); retained for save compat — not shown in settings UI.
        ConfigVar<bool> hiddenSkillRework;
        // Focused Arts charge bank + spend columns (Settings → Gameplay).
        ConfigVar<bool> focusedArts;
        // Legacy key game.focusedArtsTest — migrated into focusedArts on load; kept registered.
        ConfigVar<bool> focusedArtsTest;
        // Playtest cheat (Editor → ALBW): effective shop tier 3; WithDebug shows in-game FA overlay.
        ConfigVar<FocusedArtsCheatMode> focusedArtsCheat;
        // Flurry Rush (perfect-dodge slow-mo melee + Back Slice aerial bow). Requires Focused Arts.
        ConfigVar<bool> flurryRush;
        // Custom Models: '|'-delimited DISABLED folder names under model_replacements/
        // (empty = all enabled). LEGACY (pre-load-order): superseded by
        // customModelsOrder; still dual-read for one release as the fallback for
        // folders not yet in the order list, and mirrored on every write so a
        // downgrade keeps the same enabled/disabled state. Retire after that.
        ConfigVar<std::string> customModelsDisabled;
        // ============================================
        // NEW CODE — ALBW Port (load-order mod system, Phase 1)
        // Custom Models LOAD ORDER: '|'-delimited folder names under
        // model_replacements/ in PRIORITY order — TOP (first) wins conflicts
        // (decision D1, docs/Mod-Load-Order-Design.md). A '-' prefix marks a
        // DISABLED folder (e.g. "linkle|-oldboss|sumo"); order + enabled state
        // live in this ONE setting (decision D2). Empty = not yet migrated:
        // seeded on first scan() from the alphabetical folder list with the
        // legacy customModelsDisabled flags carried verbatim.
        // ============================================
        ConfigVar<std::string> customModelsOrder;
        // ============================================
        // NEW CODE — ALBW Port (load-order mod system, Phase 2 — Core tier D4)
        // Allow USER mods to override CORE pack assets (<exe>/content/core/).
        // OFF (default): core wins every conflict with a user mod — overriding
        // core content can break boss fights / features, so it is opt-in with
        // a warning in the editor. ON: user mods win (MO2 "base at the bottom,
        // mods on top" layering, §4.1). Flipping rescans + re-installs the
        // overlay set (reload-scoped, like a toggle).
        // ============================================
        ConfigVar<bool> customModelsAllowCoreOverride;
        // WW itemmdl Track A: use retail itemmdl.arc vbow for bow get-item (dev toggle).
        ConfigVar<bool> wwItemmdlGetItem;
        // Phase 2 diagnostic: itemmdl arc + O_gD_bow heap mesh (loader isolate 2D).
        ConfigVar<bool> wwItemmdlGetItem2DIsolate;
        // Track A SC A/B: suppress SC_Vbow_v ink pass at draw (cream-tip bloom diagnostic).
        ConfigVar<bool> wwItemmdlBowScSuppress;
        // Track A SC 4b: cosmetic K0 konst RGB cap for silver caps (255=authentic white).
        ConfigVar<int> wwItemmdlBowScK0Cap;
        // Track A SC 4b: cosmetic SC-pass output RGB ceiling (bloom threshold; 255=off).
        ConfigVar<int> wwItemmdlBowScOutputCeiling;
        // Track B: which WW itemmdl mesh skins Link's held/worn item (Off/Bow/IronBoots/Hookshot).
        ConfigVar<WwHeldSkinMode> wwItemmdlHeldSkin;
        // Track B: held WW skin scale percent (100 = 1.0x; live hand-size tuning).
        ConfigVar<int> wwItemmdlHeldBowScalePct;
        // Track B A/B: render the held WW skin boots-style — ambient-only, NO SC draw
        // scope / authentic TEV replay (the scope's white-konst spec pass is what blooms).
        ConfigVar<bool> wwItemmdlHeldBootsStyle;
        // Deku Leaf glide (WIP P1): debug toggle that re-gates TP's cucco-glide chassis onto a
        // "leaf out" state — while ON, running/jumping off a ledge floats (gentle terminal
        // velocity, stick-steer, wind updraft) with no cucco held. Reuses the cucco HIO tuning.
        ConfigVar<bool> dekuLeafGlideTest;
        // №105 P2: gated hold-A crawl — DoStatus NONE + stick ≤ front-roll rate so it
        // never steals door/talk/ENTER and does not fight stick-forward roll.
        ConfigVar<bool> enableHoldACrawl;
        // Viewer: itemmdl BDL index the get-item replay loads (0xF=vbow default). Set + Replay to
        // preview any of the 21 WW meshes in the get-item spin. See itemmdl.h for indices.
        ConfigVar<int> wwItemmdlViewerBdlIndex;
        // Sumo Outfit: swap Link's body to the shirtless sumo model. Drives the model flag only,
        // never the sumo minigame.
        //   sumoOutfit      — LEGACY/unused: "worn" is now per-save (save bit 700, via dAlbwOutfit_*).
        //                     Kept registered for back-compat; no code reads it.
        //   sumoOutfitFists — hide weapons (fists only)
        ConfigVar<bool> sumoOutfit;
        ConfigVar<bool> sumoOutfitFists;
        //   capWear         — GLOBAL Cap Wear (Off/None/Green/Red/Blue): which headpiece Link
        //                     wears across EVERY outfit (sumo body and native clothes alike).
        //                     Off = each outfit's native hat; None = bald topknot everywhere;
        //                     Green/Red/Blue = the Link/Magic/Zora cap on any base.  Replaces the
        //                     old sumoOutfitHat bool + sumoCapColor (sumo-only) settings.
        ConfigVar<CapWearMode> capWear;
        ConfigVar<bool> shieldDurability;
        // Halve wallet on death and spawn a Tear-of-Light recovery orb (F_0625 gate unchanged).
        ConfigVar<bool> deathRecoveryOrb;
        // Wolf charge economy, field attack damage/stun, bite healing. Off = vanilla TP wolf.
        ConfigVar<bool> wolfLinkCombat;
        // DEV/TEST: bypass the wolf-art shop unlocks AND the wolf-charge cost so the arts
        // (howl / punch / giant) can be triggered immediately without grinding or the story
        // unlock. Keep OFF for release. Only meaningful with wolfLinkCombat + quick-swap on.
        ConfigVar<bool> wolfArtsDevTest;
        // Master on/off for the Wolf Howl VFX tuner sliders below.  OFF = the built-in default look
        // (rotation on, no extra tilt, 1.0x); ON = apply the sliders.  Cosmetic only.
        ConfigVar<bool> wolfHowlVfxOverride;
        // Wolf Howl ring VFX tuner (live in the editor; applied only when the override above is ON):
        //   Tilt      = global X-pitch in degrees (0-360).
        //   Roll      = global Z-roll in degrees (0-360) — level the other axis for a flat ring.
        //   Width     = ring particle scale on X/Z (100 = 1.0x).
        //   Height    = ring particle scale on Y   (100 = 1.0x).
        //   SweepRate = emit-yaw wind per elapsed frame (the rotation; 0 = no spin).
        //   Orbit     = emission-point orbit radius (how far off-center the sweep swings).
        //   Period    = re-emit cadence in frames (burst density; 1 = every frame).
        ConfigVar<int> wolfHowlTiltDeg;
        ConfigVar<int> wolfHowlRollDeg;
        ConfigVar<int> wolfHowlWidthPct;
        ConfigVar<int> wolfHowlHeightPct;
        ConfigVar<int> wolfHowlSweepRate;
        ConfigVar<int> wolfHowlOrbit;
        ConfigVar<int> wolfHowlPeriod;
        // Direct wallet credit on enemy kill and boss fight victory (additive; vanilla drops unchanged).
        ConfigVar<bool> enemyDeathRupees;
        // Master Quest: Postman heart/stamina upgrades; halved heart container/piece grants.
        ConfigVar<bool> masterQuest;
        // Boss Refinement: any-sword boss damage gates; future Zant/Ganon redesign layers.
        ConfigVar<bool> bossRefinement;
        // Shade's Refuge: Lies-of-P-style Shade Watcher rest/respawn/return system
        // (watcher spawns, death-screen "Last Shade Watcher" option, shop return
        // service). Off = whole system disabled. WIP — default off.
        ConfigVar<bool> shadeRefuge;
        // Hero's Shade Secret Boss: post-game real duel vs the Hero's Shade after
        // all Hidden Skills (F_0344). Off = disabled. WIP — default off.
        ConfigVar<bool> heroShadeSecretBoss;
        // Junior Postman mail Phase 0 test: ignore story/delivered save gates for spawn.
        ConfigVar<bool> albwJuniorMailTest;
        // Editor ALBW WIP: grant/remove soulbound red potion in inventory slot 11.
        ConfigVar<bool> albwSoulboundRedPotion;
        // Parry/bash charge HUD icon style: spur only, spur+shield, or shield only.
        ConfigVar<ParryIcons> parryIconsMode;
        // Durability meter / parry charge HUD visibility (orthogonal to parry icon style).
        ConfigVar<ShieldHudVisibility> shieldHudVisibility;
        // Boss health bar HUD (name + bar) for main dungeon bosses / Ganondorf duel.
        ConfigVar<bool> bossHealthBars;
        // Lies of Link HUD: relayout vanilla HUD into a Lies-of-P spatial arrangement
        // (top-left life/meter/shield stack, top-right rupees, bottom-left items/spurs).
        // Off = vanilla TP corner layout; VanillaHearts = relayout keeping hearts;
        // HealthBar = relayout with a LoP health bar. See docs/albw-hud-lop-layout-brief.md.
        ConfigVar<LopHudMode> lopHud;
        // Epona dash-spur icons while riding. Does not affect wolf charges, shield HUD, or parry icons.
        ConfigVar<bool> showEponaSpurHud;
        // True ALBW / TRUETEST mode — see docs/TrueALBWWorld.md. Change only before loading a file.
        ConfigVar<TrueAlbwMode> trueAlbwMode;
        // ============================================
        // NEW CODE ENDS HERE
        // ============================================
        ConfigVar<int> damageMultiplier;
        // ============================================
        // NEW CODE — ALBW Port
        // Per-category true max-HP multipliers (1–16×, default 1 = off).
        // Applied once per enemy in fopAc_Execute via dAlbwHP_tryApplyTrueMaxHp().
        // ============================================
        ConfigVar<int> hpMultNormal;
        ConfigVar<int> hpMultMidBoss;
        ConfigVar<int> hpMultBoss;
        ConfigVar<int> hpMultFinalBoss;
        // Global Link attack-power divisor (1–16×, default 1 = off).
        // Applied in d_cc_uty.cpp via dAlbwHP_applyMult().
        ConfigVar<int> linkDamageDecreaseMult;
        // On-screen HP readout while Z-targeting (health / max HP / category mult).
        ConfigVar<bool> showLockonHpDebug;
        // In-game Quick Swap wardrobe / ALBW recovery mult overlay (Editor → ALBW tab).
        ConfigVar<bool> showWardrobeRecoveryDebug;
        // Editor-only: show Magic Armor as rentable before story strip. Save ownership is unchanged
        // when this is turned off (stash / equip bits are not cleared).
        ConfigVar<bool> albwMagicArmorRentableDebug;
        // Cloth damage mults, swim buffs, Sumo offensive kit — see docs/Outfit Stats.md.
        ConfigVar<bool> outfitStats;
        // File log of Darknut ALBW bash/guard-break state (Documents/dusklight/albw_darknut_debug.txt).
        ConfigVar<bool> showDarknutBashDebug;
        // Right stick cycles nearby battle enemies instead of manual lock-on camera.
        ConfigVar<bool> stickCycleLockon;
        // ============================================
        // NEW CODE ENDS HERE
        // ============================================
        ConfigVar<bool> noHeartDrops;
        ConfigVar<bool> noAmmoDrops;
        ConfigVar<bool> instantDeath;
        ConfigVar<bool> fastClimbing;
        ConfigVar<bool> noMissClimbing;
        ConfigVar<bool> fastTears;
        ConfigVar<bool> no2ndFishForCat;
        ConfigVar<bool> buttonFishing;
        ConfigVar<bool> instantSaves;
        ConfigVar<bool> instantText;
        ConfigVar<bool> sunsSong;
        ConfigVar<bool> autoSave;
        ConfigVar<bool> enhancedMapMenus;

        // Preferences
        ConfigVar<bool> enableMirrorMode;
        ConfigVar<bool> minimalHUD;
        ConfigVar<float> hudScale;
        ConfigVar<bool> pauseOnFocusLost;
        ConfigVar<bool> enableLinkDollRotation;
        ConfigVar<bool> enableAchievementToasts;
        ConfigVar<bool> enableControllerToasts;
        ConfigVar<bool> enableDiscordPresence;
        ConfigVar<MenuScaling> menuScalingMode;

        // Graphics
        ConfigVar<BloomMode> bloomMode;
        ConfigVar<float> bloomMultiplier;
        ConfigVar<DepthOfFieldMode> depthOfFieldMode;
        ConfigVar<bool> disableWaterRefraction;
        ConfigVar<bool> enableTextureReplacements;
        ConfigVar<bool> allowTextureDumps;  ///< dump every texture (hash-named) for making replacements
        ConfigVar<FrameInterpMode> enableFrameInterpolation;
        ConfigVar<int> internalResolutionScale;
        ConfigVar<int> shadowResolutionMultiplier;
        ConfigVar<Resampler> resampler;
        ConfigVar<bool> enableMapBackground;
        ConfigVar<bool> disableCutscenePillarboxing;

        // Audio
        ConfigVar<bool> noLowHpSound;
        ConfigVar<bool> midnasLamentNonStop;

        // Input
        ConfigVar<bool> enableGyroAim;
        ConfigVar<bool> enableGyroRollgoal;
        ConfigVar<float> gyroSensitivityX;
        ConfigVar<float> gyroSensitivityY;
        ConfigVar<float> gyroSensitivityRollgoal;
        ConfigVar<float> gyroSmoothing;
        ConfigVar<float> gyroDeadband;
        ConfigVar<bool> gyroInvertPitch;
        ConfigVar<bool> gyroInvertYaw;
        ConfigVar<bool> enableMouseCamera;
        ConfigVar<bool> enableMouseAim;
        ConfigVar<float> mouseAimSensitivity;
        ConfigVar<float> mouseCameraSensitivity;
        ConfigVar<bool> invertMouseY;
        ConfigVar<bool> freeCamera;
        ConfigVar<bool> enableTouchControls;
        ConfigVar<bool> enableMenuPointer;
        ConfigVar<ui::ControlLayout> touchControlsLayout;
        ConfigVar<bool> invertCameraXAxis;
        ConfigVar<bool> invertCameraYAxis;
        ConfigVar<bool> invertFirstPersonXAxis;
        ConfigVar<bool> invertFirstPersonYAxis;
        ConfigVar<bool> invertAirSwimX;
        ConfigVar<bool> invertAirSwimY;
        ConfigVar<float> freeCameraXSensitivity;
        ConfigVar<float> freeCameraYSensitivity;
        ConfigVar<float> touchCameraXSensitivity;
        ConfigVar<float> touchCameraYSensitivity;
        ConfigVar<bool> debugFlyCam;
        ConfigVar<bool> debugFlyCamLockEvents;
        ConfigVar<bool> allowBackgroundInput;
        std::array<ConfigVar<bool>, 4> enableLED;
        ConfigVar<bool> swapDirectSelect;

        // Cheats
        ConfigVar<bool> infiniteHearts;
        ConfigVar<bool> infiniteArrows;
        ConfigVar<bool> infiniteSeeds;
        ConfigVar<bool> infiniteBombs;
        ConfigVar<bool> infiniteOil;
        ConfigVar<bool> infiniteOxygen;
        ConfigVar<bool> infiniteRupees;
        ConfigVar<bool> enableIndefiniteItemDrops;
        ConfigVar<bool> moonJump;
        ConfigVar<bool> superClawshot;
        ConfigVar<bool> alwaysGreatspin;
        ConfigVar<bool> hurricaneTest;
        ConfigVar<bool> enableFastIronBoots;
        ConfigVar<bool> canTransformAnywhere;
        ConfigVar<bool> fastRoll;
        ConfigVar<bool> fastSpinner;
        ConfigVar<MagicArmorMode> armorRupeeDrain;
        ConfigVar<bool> invincibleEnemies;

        // Technical
        ConfigVar<bool> restoreWiiGlitches;

        // Controls
        ConfigVar<bool> enableTurboKeybind;
        ConfigVar<bool> enableResetKeybind;

        // Tools
        ConfigVar<bool> speedrunMode;
        ConfigVar<bool> liveSplitEnabled;
        ConfigVar<bool> showSpeedrunRTATimer;
        ConfigVar<bool> recordingMode;
        ConfigVar<bool> removeQuestMapMarkers;
        ConfigVar<bool> showInputViewer;
        ConfigVar<bool> showInputViewerGyro;
    } game;

    struct {
        ConfigVar<std::string> isoPath;
        ConfigVar<DiscVerificationState> isoVerification;
        ConfigVar<std::string> graphicsBackend;
        ConfigVar<bool> skipPreLaunchUI;
        ConfigVar<bool> showPipelineCompilation;
        ConfigVar<bool> wasPresetChosen;
        ConfigVar<bool> checkForUpdates;
        ConfigVar<int> cardFileType;
        ConfigVar<bool> enableAdvancedSettings;
        // ====================================================================
        // Level Editor (Phase 1) — surfaces the launch-menu "Level Editor"
        // entry. Requires enableAdvancedSettings; takes effect after relaunch.
        // ====================================================================
        ConfigVar<bool> enableLevelEditor;
    } backend;

    // Arrays of size 4 for 4 ports
    struct {
        std::array<ActionBindConfigVar, 4> firstPersonCamera;
        std::array<ActionBindConfigVar, 4> callMidna;
        std::array<ActionBindConfigVar, 4> openMapScreen;
        std::array<ActionBindConfigVar, 4> toggleMinimap;
        std::array<ActionBindConfigVar, 4> openDusklightMenu;
        std::array<ActionBindConfigVar, 4> turboSpeedButton;
        std::array<ActionBindConfigVar, 4> cycleSword;
        std::array<ActionBindConfigVar, 4> cycleShield;
        std::array<ActionBindConfigVar, 4> cycleOutfit;
        std::array<ActionBindConfigVar, 4> quickTransform;
        std::array<ActionBindConfigVar, 4> openItemWheel;
    } actionBindings;
};

UserSettings& getSettings();

void registerSettings();

// Transient settings

struct CollisionViewSettings {
    bool enableTerrainView;
    bool enableWireframe;
    bool enableAtView;
    bool enableTgView;
    bool enableCoView;
    float terrainViewOpacity;
    float colliderViewOpacity;
    float drawRange;
};

struct TransientSettings {
    CollisionViewSettings collisionView;
    bool skipFrameRateLimit;
    bool moveLinkActive;
    bool stateShareLoadActive;
};

TransientSettings& getTransientSettings();

}  // namespace dusk
