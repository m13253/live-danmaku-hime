/*
  Copyright (c) 2015 StarBrilliant <m13253@hotmail.com>
  All rights reserved.

  Redistribution and use in source and binary forms are permitted
  provided that the above copyright notice and this paragraph are
  duplicated in all such forms and that any documentation,
  advertising materials, and other materials related to such
  distribution and use acknowledge that the software was developed by
  StarBrilliant.
  The name of StarBrilliant may not be used to endorse or promote
  products derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#pragma once
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <utility>

namespace dmhm {

template<typename T>
static inline void unused_arg(T arg) {
    static_cast<void>(arg);
}

/* Available as a standalone library:
   https://github.com/m13253/cpp-proxy-ptr */
template<typename _Tp, typename Allocator = std::allocator<_Tp>>
class proxy_ptr {

public:

    typedef _Tp *pointer;
    typedef _Tp element_type;

    template<typename ...Args>
    explicit proxy_ptr(Args &&...args) :
        _alloc(Allocator()),
        _ptr(_alloc.allocate(1)) {
        try {
            _alloc.construct(_ptr, std::forward<Args>(args)...);
        } catch(...) {
            this->~proxy_ptr();
            throw;
        }
    }

    proxy_ptr(const proxy_ptr &other) :
        _alloc(Allocator()),
        _ptr(_alloc.allocate(1)) {
        try {
            _alloc.construct(_ptr, *other._ptr);
        } catch(...) {
            this->~proxy_ptr();
            throw;
        }
    }

    proxy_ptr(proxy_ptr &&other) :
        _ptr(other._ptr) {
        other._ptr = nullptr;
    }

    proxy_ptr(const element_type &value) :
        _alloc(Allocator()),
        _ptr(_alloc.allocate(1)) {
        try {
            _alloc.construct(_ptr, value);
        } catch(...) {
            this->~proxy_ptr();
            throw;
        }
    }

    proxy_ptr(element_type &&value) :
        _alloc(Allocator()),
        _ptr(_alloc.allocate(1)) {
        try {
            _alloc.construct(_ptr, std::move(value));
        } catch(...) {
            this->~proxy_ptr();
            throw;
        }
    }

    ~proxy_ptr() {
        if(_ptr) {
            _alloc.destroy(_ptr);
            _alloc.deallocate(_ptr, 1);
            _ptr = nullptr;
        }
    }

    proxy_ptr &operator=(const proxy_ptr &that) {
        *_ptr = *that._ptr;
        return *this;
    }

    proxy_ptr &operator=(proxy_ptr &&that) {
        std::swap(_ptr, that._ptr);
        return *this;
    }

    proxy_ptr &operator=(const element_type &value) {
        *_ptr = value;
    }

    proxy_ptr &operator=(element_type &&value) {
        *_ptr = std::move(value);
    }

    element_type &operator*() const {
        return _ptr;
    }

    pointer operator->() const {
        return _ptr;
    }

    explicit operator pointer() const {
        return _ptr;
    }

    pointer get() const {
        return _ptr;
    }

    void swap(proxy_ptr &that) {
        std::swap(_ptr, that._ptr);
    }

    void swap_payload(proxy_ptr &that) {
        std::swap(*_ptr, *that._ptr);
    }

private:

    Allocator _alloc;
    pointer _ptr;

};

class unicode_conversion_error : public std::runtime_error {
public:
    unicode_conversion_error(const char *what) : std::runtime_error(what) {}
    unicode_conversion_error() : std::runtime_error("Can not convert string to Unicode") {}
};

std::wstring utf8_to_wide(const std::string &utf8str, bool strict);

}
