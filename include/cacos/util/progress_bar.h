#pragma once

#include "cacos/util/ints.h"
#include "cacos/util/string.h"
#include "cacos/util/terminfo/terminfo.h"

#include <fmt/format.h>

#include <iostream>

#include <termcolor/termcolor.hpp>

namespace cacos::util {

template<typename Progress>
class ProgressBar {
public:
    ProgressBar(Progress max = {}, std::ostream& os = std::cerr)
        : progress_{0}
        , max_{max}
        , os_(os)
    {}

    void reset(Progress progress) {
        max_ = progress;
        progress_ = 0;
    }

    void process(Progress delta) {
        progress_ += delta;
        redraw();
    }

    void finish() {
        os_ << '\n';
    }

private:
    void redraw() {
        auto term = terminfo::get();
        if (!term.tty) {
            return;
        }

        size_t width = term.width - 2;
        if constexpr (util::string::detail::is_convertible_from_string_v<Progress>) {
            if (width > VERBOSE_WIDTH_TRESHOLD) {
                std::string result = util::string::join("[", progress_, " / ", max_, "] ");
                if (result.size() < width) {
                    os_ << result;
                    width -= result.size();
                }
            }
        }
        if constexpr (std::is_convertible_v<Progress, float>) {
            if (width > PERCENT_WIDTH_TRESHOLD) {
                std::string result = fmt::format("[{:.1f}%] ", 100.f * progress_ / max_);
                if (result.size() < width) {
                    os_ << result;
                    width -= result.size();
                }
            }
        }
        size_t cells = progress_ * width / max_;
        std::string res(cells, '=');
        res.push_back('>');
        std::string empty;
        if (static_cast<i64>(width) - static_cast<i64>(cells) - 1 > 0) {
            empty = std::string(width - cells - 1, '.');
        }
        os_ << "[" << res << empty << "]\r";
        os_.flush();
    }

    static constexpr size_t PERCENT_WIDTH_TRESHOLD = 20;
    static constexpr size_t VERBOSE_WIDTH_TRESHOLD = 40;

private:
    Progress progress_;
    Progress max_;
    std::ostream& os_;
};

} // namespace cacos::util
