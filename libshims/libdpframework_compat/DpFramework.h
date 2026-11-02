//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _DPFRAMEWORK_H
#define _DPFRAMEWORK_H

#include <stdbool.h>
#include <stdint.h>

#define DPFRAMEWORK_LIB_NAME "libdpframework.so"
#define DP_ISP_STREAM_SIZE 0x15698
#define DP_BLIT_STREAM_SIZE 0xae8
#define DP_VENC_STREAM_SIZE 0xab68

/* DpRect as the P camera passes it */
typedef struct DpRectLegacy {
    int32_t x;
    int32_t sub_x;
    int32_t y;
    int32_t sub_y;
    int32_t w;
    int32_t h;
} DpRectLegacy;

typedef struct DpRect {
    int32_t x;
    int32_t sub_x;
    int32_t y;
    int32_t sub_y;
    int32_t w;
    int32_t sub_w;
    int32_t h;
    int32_t sub_h;
} DpRect;

/* dpframework methods */
typedef struct DpFramework {
    struct {
        void (*ctor)(void*, int);
        void (*dtor)(void*);
        int  (*startStream)(void*, void*, int);
    } IspStream;

    struct {
        void (*ctor)(void*);
        void (*dtor)(void*);
        int  (*setSrcConfig)(void*, int, int, int, int, int, int, int, DpRect*, int, bool);
        int  (*setDstConfig)(void*, int, int, int, int, int, int, int, DpRect*, int, bool);
    } BlitStream;

    struct {
        void (*ctor)(void*, int);
        void (*dtor)(void*);
    } VEncStream;
} DpFramework;

extern DpFramework gDp;

#endif
