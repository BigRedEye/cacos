#pragma once

#include <stdexcept>
#include <utility>

/* safe alternative to T* (std::optional<T&>) */

namespace cacos::util {

class bad_optional_ref_access : std::runtime_error {
public:
    bad_optional_ref_access();
};

struct empty_optional_ref_tag {};

inline constexpr empty_optional_ref_tag nullref;

template<typename T>
class optional_ref {
public:
    /* implicit */ optional_ref(T& t)
        : ptr_(std::addressof(t)) {
    }

    /* implicit */ optional_ref(empty_optional_ref_tag)
        : ptr_(nullptr) {
    }

    operator T&() {
        if (!ptr_) {
            throw bad_optional_ref_access();
        }
        return *ptr_;
    }

    operator const T&() const {
        if (!ptr_) {
            throw bad_optional_ref_access();
        }
        return *ptr_;
    }

    operator bool() const {
        return !!ptr_;
    }

    bool operator==(optional_ref<T> other) const {
        return bool(*this) == bool(other) && (!bool(*this) || T(*this) == T(other));
    }

    bool operator!=(optional_ref<T> other) const {
        return !operator==(other);
    }

    T& value() {
        return *this;
    }

    const T& value() const {
        return *this;
    }

private:
    T* ptr_ = nullptr;
};

} // namespace cacos::util
