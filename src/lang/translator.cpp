#include "cacos/lang/translator.h"

namespace cacos::lang {

Translator::Translator(const cpptoml::table& table) {
    auto exe = table.get_as<std::string>("exe");
    if (!exe) {
        throw std::runtime_error("Cannot find executable");
    }
    exe_ = executable::Executable(bp::search_path(*exe).string());

    auto flags = table.get_array_of<std::string>("flags");
    if (flags) {
        common_ = *flags;
    }
}

} // namespace cacos::lang
