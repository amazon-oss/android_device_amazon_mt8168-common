//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <dlfcn.h>
#include <stdlib.h>

#ifndef LOG_ALWAYS_FATAL_IF
#error "Please include <log/log.h> first."
#endif

static inline void* sdlsym(void* handle, const char* name)
{
    void* p = dlsym(handle, name);
    LOG_ALWAYS_FATAL_IF(!p, "failed to dlsym %s: %s", name, dlerror());
    return p;
}

typedef struct thiz {
    void *inner;
} thiz_t;

#define INNER(_x) ((_x)->inner)

#define INNER_CONSTRUCT(_self, _size, _ctor, ...)        \
    do {                                                 \
        thiz_t *_obj_ptr = (_self);                      \
        void *_inner = malloc(_size);                    \
        LOG_ALWAYS_FATAL_IF(_inner == NULL,              \
                            "failed to allocate inner"); \
        _ctor(_inner, ##__VA_ARGS__);                    \
        _obj_ptr->inner = _inner;                        \
    } while (0)

#define INNER_DESTRUCT(_self, _dtor)                    \
    do {                                                \
        thiz_t *_obj_ptr = (_self);                     \
        if (_obj_ptr && _obj_ptr->inner) {              \
            _dtor(_obj_ptr->inner);                     \
            free(_obj_ptr->inner);                      \
            _obj_ptr->inner = NULL;                     \
        }                                               \
    } while (0)
