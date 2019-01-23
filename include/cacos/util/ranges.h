#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

namespace cacos::util {

template<typename It>
struct Range {
    It begin() const {
        return begin_;
    }

    It end() const {
        return end_;
    }

    It begin_;
    It end_;
};

template<typename C, typename It = decltype(std::declval<C>().begin())>
Range<It> skip(C&& c, std::size_t steps) {
    auto it = c.begin();
    while (it != c.end() && steps) {
        ++it;
        --steps;
    }

    return {it, c.end()};
}

template<typename C, typename It = decltype(std::declval<C>().begin())>
Range<It> select(C&& c, std::size_t pos) {
    /*
    auto it = std::next(c.begin(), pos);
    auto next = std::next(it, 1);
    return {it, next};
    */

    auto it = c.begin();
    while (pos--) {
        ++it;
    }

    auto next = it;
    ++next;
    return {it, next};
}

}
