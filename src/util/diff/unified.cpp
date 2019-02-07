#include "cacos/util/diff/unified.h"

#include "cacos/util/split.h"

#include <termcolor/termcolor.hpp>

#include <fmt/ostream.h>

#include <string>

namespace cacos::util::diff {

Unified::Unified(const std::string& left, const std::string& right) {
    auto lsplit = util::split<std::string>(left, "\n");
    auto rsplit = util::split<std::string>(right, "\n");

    dtl::Diff<std::string> diff(lsplit, rsplit);
    diff.compose();
    diff.composeUnifiedHunks();

    hunks_ = diff.getUniHunks();
}

void Unified::print(std::ostream& os) const {
    for (auto&& hunk : hunks_) {
        os << termcolor::bold << termcolor::cyan;
        fmt::print(os, "@@ -{},{} +{},{} @@\n", hunk.a, hunk.b, hunk.c, hunk.d);
        os << termcolor::reset;
        for (auto&& line : hunk.common[0]) {
            fmt::print(os, " {}\n", line.first);
        }
        for (auto&& line : hunk.change) {
            switch (line.second.type) {
            case dtl::SES_ADD:
                std::cout << termcolor::green << "+";
                break;
            case dtl::SES_COMMON:
                std::cout << termcolor::reset << " ";
                break;
            case dtl::SES_DELETE:
                std::cout << termcolor::red << "-";
                break;
            }
            fmt::print(os, "{}\n", line.first);
        }
        os << termcolor::reset;
        for (auto&& line : hunk.common[1]) {
            fmt::print(os, " {}\n", line.first);
        }
    }
}

bool Unified::empty() const {
    return hunks_.empty();
}

} // namespace cacos::util::diff
