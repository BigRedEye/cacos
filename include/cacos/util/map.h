#pragma once

#include "cacos/util/optional_ref.h"
#include "cacos/util/string.h"

#include <unordered_map>

namespace cacos::util {

class MappingError : std::runtime_error {
public:
    MappingError(const std::string& what);
};

namespace detail {

template<typename K, typename V>
class Map {
public:
    Map(K&& k, V&& v)
        : map_{{k, v}} {
    }

    Map& operator()(K&& k, V&& v) {
        map_.emplace(std::forward<K>(k), std::forward<V>(v));
        return *this;
    }

    const V& map(const K& key) {
        return mapImpl(key);
    }

    const V& map(const K& key, const V& defaultValue) {
        return mapImpl(key, defaultValue);
    }

private:
    struct NoDefaultValue {};

    template<typename T = NoDefaultValue>
    const V& mapImpl(const K& key, const T& defaultValue = T{}) {
        auto it = map_.find(key);

        if (it == map_.end()) {
            if constexpr (!std::is_same_v<std::decay_t<T>, NoDefaultValue>) {
                return defaultValue;
            } else if constexpr (util::string::detail::is_convertible_from_string_v<K>) {
                throw MappingError("Unknown key " + util::string::to(key));
            } else {
                throw MappingError("Unknown key");
            }
        }

        return it->second;
    }

private:
    std::unordered_map<K, V> map_;
};

} // namespace detail

template<typename K, typename V>
auto map(K&& k, V&& v) -> detail::Map<std::decay_t<K>, std::decay_t<V>> {
    return {std::forward<K>(k), std::forward<V>(v)};
}

} // namespace cacos::util
