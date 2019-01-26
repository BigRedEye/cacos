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

template<typename It>
class EnumeratedIterator {
public:
    EnumeratedIterator(It&& it, size_t pos = 0)
        : it_(std::forward<It>(it))
        , pos_(pos)
    {}

    auto operator*() const {
        return std::pair{*it_, pos_};
    }

    auto operator*() {
        return std::pair{*it_, pos_};
    }

    EnumeratedIterator& operator++() {
        ++it_;
        ++pos_;
        return *this;
    }

    EnumeratedIterator operator++(int) {
        EnumeratedIterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator!=(const EnumeratedIterator& other) {
        return it_ != other.it_;
    }

private:
    It it_;
    size_t pos_;
};

template<typename C, typename It = decltype(std::declval<C>().begin())>
Range<EnumeratedIterator<It>> enumerate(C&& c) {
    return {EnumeratedIterator{c.begin(), 0}, EnumeratedIterator{c.end(), 0}};
}

}
