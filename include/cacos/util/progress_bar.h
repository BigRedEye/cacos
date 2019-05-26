#pragma once

#include "cacos/util/ints.h"
#include "cacos/util/string.h"
#include "cacos/util/terminfo/terminfo.h"

#include <fmt/format.h>

#include <termcolor/termcolor.hpp>

#include <chrono>
#include <iostream>

namespace cacos::util {

template<typename Progress>
class ProgressBar {
public:
    ProgressBar(Progress max = {}, std::ostream& os = std::cerr)
        : progress_{0}
        , max_{max}
        , os_(os)
        , lastRedraw_{std::chrono::high_resolution_clock::now()} {
    }

    void reset(Progress max) {
        max_ = max;
        progress_ = 0;
    }

    void process(Progress delta, std::string_view info = "") {
        progress_ += delta;
        redraw(info);
    }

private:
    void redraw(std::string_view info) {
        auto term = terminfo::get();
        if (!term.tty) {
            return;
        }

        if (getenv("CACOS_DISABLE_PROGRESS_BAR")) {
            return;
        }

        size_t spaceLeft = term.width - 2;
        if constexpr (util::string::detail::is_convertible_from_string_v<Progress>) {
            if (spaceLeft > VERBOSE_WIDTH_TRESHOLD) {
                std::string result = util::string::join("[", progress_, " / ", max_, "] ");
                if (result.size() < spaceLeft) {
                    os_ << result;
                    spaceLeft -= result.size();
                }
            }
        }
        if constexpr (std::is_convertible_v<Progress, float>) {
            if (spaceLeft > PERCENT_WIDTH_TRESHOLD) {
                std::string result = fmt::format("[{:.1f}%] ", 100.f * progress_ / max_);
                if (result.size() < spaceLeft) {
                    os_ << result;
                    spaceLeft -= result.size();
                }
            }
        }
        /* (user info) [=========>.......] */
        if (!info.empty() && spaceLeft > MIN_BAR_WIDTH + 3) {
            size_t infoChars = spaceLeft - MIN_BAR_WIDTH - 3;
            if (infoChars >= info.size()) {
                infoOffset_ = 0;
            } else {
                auto now = std::chrono::high_resolution_clock::now();
                if (now - lastRedraw_ > FLOATING_TIMEOUT) {
                    lastRedraw_ = now;
                    ++infoOffset_;
                }
            }
            auto suffix = info.substr(infoOffset_ % info.size(), infoChars);
            std::string_view delim{""};
            std::string_view prefix{""};
            bool shouldRepeat = suffix.size() < info.size() && suffix.size() != infoChars;
            if (shouldRepeat) {
                delim = " ";
                if (suffix.size() + delim.size() < infoChars) {
                    prefix = info.substr(0, infoChars - suffix.size() - 1);
                }
            }
            std::string result = fmt::format("({}{}{}) ", suffix, delim, prefix);

            os_ << result;
            spaceLeft -= result.size();
        }

        size_t cells = progress_ * spaceLeft / max_;
        std::string res(cells, '=');
        if (progress_ != max_) {
            res.push_back('>');
        }
        std::string empty;
        if (static_cast<i64>(spaceLeft) - static_cast<i64>(cells) - 1 > 0) {
            empty = std::string(spaceLeft - cells - 1, '.');
        }
        os_ << "[" << res << empty << "]" << (progress_ < max_ ? '\r' : '\n');
        os_.flush();
    }

    static constexpr size_t MIN_BAR_WIDTH{10};
    static constexpr size_t PERCENT_WIDTH_TRESHOLD{20};
    static constexpr size_t VERBOSE_WIDTH_TRESHOLD{40};
    static constexpr std::chrono::milliseconds FLOATING_TIMEOUT{500};

private:
    Progress progress_;
    Progress max_;
    std::ostream& os_;

    std::chrono::high_resolution_clock::time_point lastRedraw_;
    size_t infoOffset_{0};
};

} // namespace cacos::util
