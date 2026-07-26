#include "dusk/config.hpp"
#include "absl/container/flat_hash_map.h"
#include "fmt/format.h"
#include "nlohmann/json.hpp"

#include "aurora/lib/logging.hpp"
#include "dusk/io.hpp"
#include "dusk/settings.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "dusk/action_bindings.h"
#include "dusk/main.h"

using namespace dusk::config;

constexpr auto ConfigFileName = "config.json";

using json = nlohmann::json;

aurora::Module DuskConfigLog("dusk::config");

static absl::flat_hash_map<std::string, ConfigVarBase*> RegisteredConfigVars;
// Keys read from config (or given as launch args) with no CVar registered yet. Kept so a mod
// registering its CVars long after startup still picks up its saved values, and so Save()
// preserves keys owned by a mod that is currently disabled.
static absl::flat_hash_map<std::string, nlohmann::json> UnregisteredConfigVars;
static absl::flat_hash_map<std::string, std::string> UnregisteredConfigVarOverrides;

namespace {

struct ChangeSubscription {
    Subscription token;
    ChangeCallback callback;
};
absl::flat_hash_map<std::string, std::vector<ChangeSubscription> > s_changeSubscriptions;
absl::flat_hash_map<Subscription, std::string> s_changeTokenNames;
Subscription s_nextChangeToken = 1;
// Names currently being notified; guards against a callback re-notifying its own CVar.
std::vector<std::string> s_activeChangeNotifications;

}  // namespace

static std::optional<dusk::ui::ControlAnchor> parse_control_anchor(std::string_view value) {
    if (value == "none") {
        return dusk::ui::ControlAnchor::None;
    }
    if (value == "top") {
        return dusk::ui::ControlAnchor::Top;
    }
    if (value == "left") {
        return dusk::ui::ControlAnchor::Left;
    }
    if (value == "bottom") {
        return dusk::ui::ControlAnchor::Bottom;
    }
    if (value == "right") {
        return dusk::ui::ControlAnchor::Right;
    }
    if (value == "topLeft") {
        return dusk::ui::ControlAnchor::TopLeft;
    }
    if (value == "topRight") {
        return dusk::ui::ControlAnchor::TopRight;
    }
    if (value == "bottomLeft") {
        return dusk::ui::ControlAnchor::BottomLeft;
    }
    if (value == "bottomRight") {
        return dusk::ui::ControlAnchor::BottomRight;
    }
    return std::nullopt;
}

static const char* control_anchor_value(dusk::ui::ControlAnchor anchor) {
    switch (anchor) {
    case dusk::ui::ControlAnchor::None:
        return "none";
    case dusk::ui::ControlAnchor::Top:
        return "top";
    case dusk::ui::ControlAnchor::Left:
        return "left";
    case dusk::ui::ControlAnchor::Bottom:
        return "bottom";
    case dusk::ui::ControlAnchor::Right:
        return "right";
    case dusk::ui::ControlAnchor::TopLeft:
        return "topLeft";
    case dusk::ui::ControlAnchor::TopRight:
        return "topRight";
    case dusk::ui::ControlAnchor::BottomLeft:
        return "bottomLeft";
    case dusk::ui::ControlAnchor::BottomRight:
        return "bottomRight";
    }
    return "none";
}

static std::optional<float> json_finite_float(const json& object, const char* key) {
    const auto iter = object.find(key);
    if (iter == object.end() || !iter->is_number()) {
        return std::nullopt;
    }

    const float value = iter->get<float>();
    if (!std::isfinite(value)) {
        return std::nullopt;
    }

    return value;
}

static std::optional<dusk::ui::ControlProps> parse_control_props(const json& value) {
    if (!value.is_object()) {
        return std::nullopt;
    }

    const auto x = json_finite_float(value, "x");
    const auto y = json_finite_float(value, "y");
    const auto w = json_finite_float(value, "w");
    const auto h = json_finite_float(value, "h");
    const auto scale = json_finite_float(value, "scale");
    const auto anchorIter = value.find("anchor");
    if (!x || !y || !w || !h || !scale || anchorIter == value.end() || !anchorIter->is_string()) {
        return std::nullopt;
    }

    const auto anchor = parse_control_anchor(anchorIter->get<std::string>());
    if (!anchor || *w <= 0.0f || *h <= 0.0f || *scale <= 0.0f) {
        return std::nullopt;
    }
    return dusk::ui::ControlProps{
        .x = *x,
        .y = *y,
        .w = *w,
        .h = *h,
        .scale = *scale,
        .anchor = *anchor,
    };
}

static std::filesystem::path GetConfigJsonPath() {
    return dusk::ConfigPath / ConfigFileName;
}

static std::filesystem::path GetTempConfigJsonPath(const std::filesystem::path& configJsonPath) {
    auto tempPath = configJsonPath;
    tempPath.replace_filename(fmt::format(".{}.tmp", configJsonPath.filename().string()));
    return tempPath;
}

static void ReplaceFile(const std::filesystem::path& source, const std::filesystem::path& target) {
    std::error_code ec;
    std::filesystem::rename(source, target, ec);
    if (ec) {
        const auto renameError = ec;
        std::filesystem::remove(source, ec);
        throw std::system_error(renameError);
    }
}

ConfigVarBase::ConfigVarBase(std::string name, const ConfigImplBase* impl)
    : name(std::move(name)), registered(false), layer(ConfigVarLayer::Default), impl(impl) {}

const char* ConfigVarBase::getName() const noexcept {
    return name.c_str();
}

const ConfigImplBase* ConfigVarBase::getImpl() const noexcept {
    return impl;
}

template <typename T>
static T sanitizeEnumValue(const ConfigVar<T>& cVar, T value) {
    if constexpr (std::is_enum_v<T>) {
        using Underlying = std::underlying_type_t<T>;
        const Underlying raw = static_cast<Underlying>(value);
        const Underlying min = static_cast<Underlying>(ConfigEnumRange<T>::min);
        const Underlying max = static_cast<Underlying>(ConfigEnumRange<T>::max);
        if (raw < min || raw > max) {
            return cVar.getDefaultValue();
        }
    }

    return value;
}

template <ConfigValue T>
void ConfigImpl<T>::loadFromJson(ConfigVar<T>& cVar, const json& jsonValue) {
    if constexpr (std::is_enum_v<T>) {
        if (jsonValue.is_boolean()) {
            DuskConfigLog.error("Doing default migration of CVar {} from bool, enum values may not "
                                "be what is expected!",
                cVar.getName());

            using Underlying = std::underlying_type_t<T>;
            const bool b = jsonValue.get<bool>();

            const Underlying raw = b ? static_cast<Underlying>(1) : static_cast<Underlying>(0);

            cVar.load_value(sanitizeEnumValue(cVar, static_cast<T>(raw)));
            return;
        }
    }

    cVar.load_value(sanitizeEnumValue(cVar, jsonValue.get<T>()));
}

template <ConfigValue T>
nlohmann::json ConfigImpl<T>::dumpToJson(const ConfigVar<T>& cVar) {
    return cVar.getValueForSave();
}

template <ConfigValue T>
requires std::is_integral_v<T>&& std::is_signed_v<T> static T parse_arg_value(
    const ConfigVar<T>&, const std::string_view stringValue) {
    const std::string str(stringValue);
    const auto result = std::stoll(str);
    if (result >= std::numeric_limits<T>::min() && result <= std::numeric_limits<T>::max()) {
        return static_cast<T>(result);
    }
    throw std::out_of_range("Value is too large");
}

template <ConfigValue T>
requires std::is_integral_v<T>&& std::is_unsigned_v<T> static T parse_arg_value(
    const ConfigVar<T>&, const std::string_view stringValue) {
    const std::string str(stringValue);
    const auto result = std::stoull(str);
    if (result <= std::numeric_limits<T>::max()) {
        return static_cast<T>(result);
    }
    throw std::out_of_range("Value is too large");
}

static f32 parse_arg_value(const ConfigVar<f32>&, const std::string_view stringValue) {
    const std::string str(stringValue);
    return std::stof(str);
}

static f64 parse_arg_value(const ConfigVar<f64>&, const std::string_view stringValue) {
    const std::string str(stringValue);
    return std::stod(str);
}

static std::string parse_arg_value(
    const ConfigVar<std::string>&, const std::string_view stringValue) {
    return std::string(stringValue);
}

template <ConfigValue T>
requires std::is_enum_v<T> static T parse_arg_value(
    const ConfigVar<T>& cVar, const std::string_view stringValue) {
    using Underlying = std::underlying_type_t<T>;
    const std::string str(stringValue);

    if constexpr (std::is_signed_v<Underlying>) {
        const auto result = std::stoll(str);
        if (result >= std::numeric_limits<Underlying>::min() &&
            result <= std::numeric_limits<Underlying>::max())
        {
            return sanitizeEnumValue(cVar, static_cast<T>(result));
        }
        throw std::out_of_range("Value is too large");
    } else {
        const auto result = std::stoull(str);
        if (result <= std::numeric_limits<Underlying>::max()) {
            return sanitizeEnumValue(cVar, static_cast<T>(result));
        }
        throw std::out_of_range("Value is too large");
    }
}

template <ConfigValue T>
void ConfigImpl<T>::loadFromArg(ConfigVar<T>& cVar, const std::string_view stringValue) {
    cVar.load_override_value(parse_arg_value(cVar, stringValue));
}

template <>
void ConfigImpl<bool>::loadFromArg(ConfigVar<bool>& cVar, const std::string_view stringValue) {
    if (stringValue == "1" || stringValue == "TRUE" || stringValue == "true" ||
        stringValue == "True")
    {
        cVar.load_override_value(true);
    } else if (stringValue == "0" || stringValue == "FALSE" || stringValue == "false" ||
               stringValue == "False")
    {
        cVar.load_override_value(false);
    } else {
        throw InvalidConfigError("Value cannot be parsed as boolean");
    }
}

// My IDE is convinced this namespace is necessary. It shouldn't be AFAICT?
namespace dusk::config {
template class ConfigImpl<bool>;
template class ConfigImpl<s8>;
template class ConfigImpl<u8>;
template class ConfigImpl<s16>;
template class ConfigImpl<u16>;
template class ConfigImpl<s32>;
template class ConfigImpl<u32>;
template class ConfigImpl<s64>;
template class ConfigImpl<u64>;
template class ConfigImpl<f32>;
template class ConfigImpl<f64>;
template class ConfigImpl<std::string>;
template class ConfigImpl<dusk::BloomMode>;
template class ConfigImpl<dusk::DepthOfFieldMode>;
template class ConfigImpl<dusk::DiscVerificationState>;
template class ConfigImpl<dusk::GameLanguage>;

template <>
void ConfigImpl<FrameInterpMode>::loadFromJson(
    ConfigVar<FrameInterpMode>& cVar, const json& jsonValue) {
    if (jsonValue.is_boolean()) {
        const bool b = jsonValue.get<bool>();

        const FrameInterpMode mode = b ? FrameInterpMode::Unlimited : FrameInterpMode::Off;

        cVar.load_value(sanitizeEnumValue(cVar, mode));
        return;
    }

    cVar.load_value(sanitizeEnumValue(cVar, jsonValue.get<FrameInterpMode>()));
}

template <>
void ConfigImpl<ui::ControlLayout>::loadFromJson(
    ConfigVar<ui::ControlLayout>& cVar, const json& jsonValue) {
    if (!jsonValue.is_object()) {
        return;
    }

    const int version = jsonValue.value("version", 0);
    if (version != ui::ControlLayout::Version) {
        return;
    }

    const auto controlsIter = jsonValue.find("controls");
    if (controlsIter == jsonValue.end() || !controlsIter->is_object()) {
        return;
    }

    ui::ControlLayout layout{.version = version};
    for (const auto& control : controlsIter->items()) {
        if (!ui::is_control_layout_id(control.key())) {
            continue;
        }

        if (const auto props = parse_control_props(control.value())) {
            layout.controls[control.key()] = *props;
        }
    }

    cVar.load_value(std::move(layout));
}

template <>
void ConfigImpl<ui::ControlLayout>::loadFromArg(
    ConfigVar<ui::ControlLayout>&, const std::string_view) {
    throw InvalidConfigError("Touch control layout cannot be parsed from launch arguments");
}

template <>
nlohmann::json ConfigImpl<ui::ControlLayout>::dumpToJson(const ConfigVar<ui::ControlLayout>& cVar) {
    const auto& layout = cVar.getValueForSave();
    json controls = json::object();
    for (const auto& [id, props] : layout.controls) {
        controls[id] = {
            {"x", props.x},
            {"y", props.y},
            {"w", props.w},
            {"h", props.h},
            {"scale", props.scale},
            {"anchor", control_anchor_value(props.anchor)},
        };
    }

    return {
        {"version", ui::ControlLayout::Version},
        {"controls", std::move(controls)},
    };
}

template class ConfigImpl<dusk::FrameInterpMode>;
template class ConfigImpl<dusk::MenuScaling>;
template class ConfigImpl<dusk::Resampler>;
template class ConfigImpl<dusk::MagicArmorMode>;
template class ConfigImpl<dusk::ParryIcons>;
template class ConfigImpl<dusk::ShieldHudVisibility>;
template class ConfigImpl<dusk::LopHudMode>;
template class ConfigImpl<dusk::ExtraItemSlotMode>;
template class ConfigImpl<dusk::TrueAlbwMode>;
template class ConfigImpl<dusk::FocusedArtsCheatMode>;
template class ConfigImpl<dusk::CapWearMode>;
template class ConfigImpl<dusk::WwHeldSkinMode>;
template class ConfigImpl<dusk::ui::ControlLayout>;
}  // namespace dusk::config

void dusk::config::Register(ConfigVarBase& configVar) {
    const std::string_view name = configVar.getName();
    if (RegisteredConfigVars.contains(name)) {
        DuskConfigLog.fatal("Tried to register CVar {} twice!", name);
    }

    RegisteredConfigVars[name] = &configVar;
    configVar.markRegistered();

    const auto unregPair = UnregisteredConfigVars.find(name);
    if (unregPair != UnregisteredConfigVars.end()) {
        const auto value = std::move(unregPair->second);
        UnregisteredConfigVars.erase(name);

        try {
            configVar.getImpl()->loadFromJson(configVar, value);
        } catch (std::exception& e) {
            DuskConfigLog.error("Failed to load key '{}' from config value: {}", name, e.what());
        }
    }

    const auto overridePair = UnregisteredConfigVarOverrides.find(name);
    if (overridePair != UnregisteredConfigVarOverrides.end()) {
        try {
            configVar.getImpl()->loadFromArg(configVar, overridePair->second);
        } catch (std::exception& e) {
            DuskConfigLog.error("Failed to load key '{}' from override arg: {}", name, e.what());
        }
    }
}

void dusk::config::unregister(ConfigVarBase& configVar) {
    const std::string_view name = configVar.getName();
    const auto it = RegisteredConfigVars.find(name);
    if (it == RegisteredConfigVars.end() || it->second != &configVar) {
        DuskConfigLog.fatal("Tried to unregister CVar '{}' that is not registered!", name);
    }

    const auto layer = configVar.getLayer();
    if (layer == ConfigVarLayer::Value || layer == ConfigVarLayer::Speedrun) {
        UnregisteredConfigVars.insert_or_assign(
            std::string{name}, configVar.getImpl()->dumpToJson(configVar));
    }

    RegisteredConfigVars.erase(it);
    configVar.unmarkRegistered();
}

void ConfigVarBase::markRegistered() {
    if (registered)
        abort();

    registered = true;
}

void ConfigVarBase::unmarkRegistered() {
    if (!registered)
        abort();

    registered = false;
}

bool ConfigVarBase::has_subscribers() const {
    return s_changeSubscriptions.contains(name);
}

void ConfigVarBase::notify_changed(const void* previousValue) {
    const auto subsIt = s_changeSubscriptions.find(name);
    if (subsIt == s_changeSubscriptions.end()) {
        return;
    }
    if (std::ranges::find(s_activeChangeNotifications, name) != s_activeChangeNotifications.end()) {
        DuskConfigLog.error("Recursive change notification for CVar '{}' suppressed", name);
        return;
    }

    s_activeChangeNotifications.push_back(name);
    // Copied so callbacks can subscribe/unsubscribe safely.
    const auto subscriptions = subsIt->second;
    for (const auto& sub : subscriptions) {
        sub.callback(*this, previousValue);
    }
    s_activeChangeNotifications.pop_back();
}

Subscription dusk::config::subscribe(std::string_view name, ChangeCallback callback) {
    const auto token = s_nextChangeToken++;
    s_changeSubscriptions[std::string{name}].push_back({token, std::move(callback)});
    s_changeTokenNames.emplace(token, std::string{name});
    return token;
}

void dusk::config::unsubscribe(Subscription token) {
    const auto nameIt = s_changeTokenNames.find(token);
    if (nameIt == s_changeTokenNames.end()) {
        DuskConfigLog.fatal("Tried to unsubscribe unknown change token {}!", token);
    }

    const auto subsIt = s_changeSubscriptions.find(nameIt->second);
    auto& subscriptions = subsIt->second;
    std::erase_if(
        subscriptions, [token](const ChangeSubscription& sub) { return sub.token == token; });
    if (subscriptions.empty()) {
        s_changeSubscriptions.erase(subsIt);
    }
    s_changeTokenNames.erase(nameIt);
}

void dusk::config::FinishRegistration() {}

void dusk::config::LoadFromUserPreferences() {
    const auto configJsonPath = GetConfigJsonPath();
    if (configJsonPath.empty()) {
        return;
    }
    const auto configPathString = io::fs_path_to_string(configJsonPath);
    LoadFromFileName(configPathString.c_str());
}

static void LoadFromPath(const char* path) {
    auto data = dusk::io::FileStream::ReadAllBytes(path);

    json j = json::parse(data);
    if (!j.is_object()) {
        DuskConfigLog.error("Config JSON is not an object!");
        return;
    }

    UnregisteredConfigVars.clear();

    for (const auto& el : j.items()) {
        const auto& key = el.key();
        auto configVar = RegisteredConfigVars.find(key);
        if (configVar == RegisteredConfigVars.end()) {
            // Not an error: mod CVars register long after the config is read.
            UnregisteredConfigVars.emplace(key, el.value());
            continue;
        }

        try {
            configVar->second->getImpl()->loadFromJson(*configVar->second, el.value());
        } catch (std::exception& e) {
            DuskConfigLog.error("Failed to load key '{}' from config: {}", key, e.what());
        }
    }

    if (j.contains("game.trueAlbwShop")) {
        try {
            if (j["game.trueAlbwShop"].get<bool>() &&
                dusk::getSettings().game.trueAlbwMode.getValue() == dusk::TrueAlbwMode::Off) {
                dusk::getSettings().game.trueAlbwMode.setValue(dusk::TrueAlbwMode::TrueAlbw);
            }
        } catch (std::exception&) {
        }
    }

    // Hidden Skill Rework is always on; normalize legacy saves that disabled it.
    dusk::getSettings().game.hiddenSkillRework.setValue(true, false);

    // Promote legacy Focused Arts Test into the official Gameplay toggle.
    if (dusk::getSettings().game.focusedArtsTest.getValue() &&
        !dusk::getSettings().game.focusedArts.getValue())
    {
        dusk::getSettings().game.focusedArts.setValue(true, false);
    }
    dusk::getSettings().game.focusedArtsTest.setValue(false, false);
}

void dusk::config::LoadFromFileName(const char* path) {
    DuskConfigLog.info("Loading config from '{}'", path);

    try {
        LoadFromPath(path);
    } catch (const std::system_error& e) {
        if (e.code() == std::errc::no_such_file_or_directory) {
            DuskConfigLog.info("Config file did not exist, staying with defaults");
        } else {
            DuskConfigLog.error("Failed to load from config! {}", e.what());
        }
    } catch (const nlohmann::json::parse_error& e) {
        DuskConfigLog.error("Failed to parse config JSON, staying with defaults: {}", e.what());
    } catch (const std::exception& e) {
        DuskConfigLog.error("Failed to load from config, staying with defaults: {}", e.what());
    }
}

void dusk::config::Save() {
    const auto configJsonPath = GetConfigJsonPath();
    if (configJsonPath.empty()) {
        return;
    }
    const auto configPathString = io::fs_path_to_string(configJsonPath);

    DuskConfigLog.info("Saving config to '{}'", configPathString);

    json j;

    for (const auto& pair : RegisteredConfigVars) {
        const auto layer = pair.second->getLayer();
        if (layer == ConfigVarLayer::Value || layer == ConfigVarLayer::Speedrun) {
            j[pair.first] = pair.second->getImpl()->dumpToJson(*pair.second);
        }
    }

    // Keys whose owning CVar is not currently registered (unloaded mod, older/newer build)
    // are written back untouched instead of being dropped.
    for (const auto& pair : UnregisteredConfigVars) {
        j[pair.first] = pair.second;
    }

    try {
        const auto tempConfigJsonPath = GetTempConfigJsonPath(configJsonPath);
        io::FileStream::WriteAllText(tempConfigJsonPath, j.dump(4));
        ReplaceFile(tempConfigJsonPath, configJsonPath);
    } catch (const std::exception& e) {
        DuskConfigLog.error("Failed to save config to '{}': {}", configPathString, e.what());
    }
}

void dusk::config::ClearAllActionBindings(int port) {
    for (auto& actionBinding : getActionBinds() | std::views::values) {
        actionBinding.configVars->at(port).setValue(PAD_NATIVE_BUTTON_INVALID);
    }
    Save();
}

ConfigVarBase* dusk::config::GetConfigVar(std::string_view name) {
    const auto configVar = RegisteredConfigVars.find(name);
    if (configVar != RegisteredConfigVars.end()) {
        return configVar->second;
    }

    return nullptr;
}

void dusk::config::EnumerateRegistered(std::function<void(ConfigVarBase&)> callback) {
    for (auto& pair : RegisteredConfigVars) {
        callback(*pair.second);
    }
}
