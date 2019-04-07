#include "cacos/lang/translator.h"

namespace cacos::lang {

Translator::Translator(const cpptoml::table& table) {
    auto exe = table.get_as<std::string>("exe");
    if (!exe) {
        throw std::runtime_error("Cannot find executable");
    }
    fs::path exePath = bp::search_path(*exe).string();
    if (exePath.empty()) {
        throw std::runtime_error("Cannot find executable " + *exe);
    }
    exe_ = executable::Executable(exePath);

    auto flags = table.get_array_of<std::string>("flags");
    if (flags) {
        common_ = *flags;
    } else {
        throw std::runtime_error("Cannot find flags for " + *exe);
    }

    if (auto debug = table.get_array_of<std::string>("debug")) {
        debug_ = *debug;
    }

    if (auto release = table.get_array_of<std::string>("release")) {
        release_ = *release;
    }
}

bool Translator::operator==(const Translator& other) const {
    return std::tie(exe_, common_, debug_, release_) ==
           std::tie(other.exe_, other.common_, other.debug_, other.release_);
}

bool Translator::operator!=(const Translator& other) const {
    return !operator==(other);
}

} // namespace cacos::lang
