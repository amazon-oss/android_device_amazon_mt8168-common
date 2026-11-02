//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "DpBlitStream_compat"

#include <log/log.h>
#include <shim-helpers.h>

#include "DpFramework.h"

static DpRect* convert_rect(DpRect* dst, const DpRectLegacy* src)
{
    if (src == NULL)
        return NULL;

    *dst = (DpRect){
        .x = src->x,
        .sub_x = src->sub_x,
        .y = src->y,
        .sub_y = src->sub_y,
        .w = src->w,
        .h = src->h,
    };
    return dst;
}

/* DpBlitStream::DpBlitStream() */
thiz_t* _ZN12DpBlitStreamC1Ev(thiz_t* self)
{
    INNER_CONSTRUCT(self, DP_BLIT_STREAM_SIZE, gDp.BlitStream.ctor);
    return self;
}

/* DpBlitStream::~DpBlitStream() */
thiz_t* _ZN12DpBlitStreamD1Ev(thiz_t* self)
{
    INNER_DESTRUCT(self, gDp.BlitStream.dtor);
    return self;
}

/* DpBlitStream::setSrcConfig(int, int, int, int, DP_COLOR_ENUM, DP_PROFILE_ENUM, DpInterlaceFormat, DpRect*, DpSecure, bool) */
int _ZN12DpBlitStream12setSrcConfigEiiii13DP_COLOR_ENUM15DP_PROFILE_ENUM17DpInterlaceFormatP6DpRect8DpSecureb(
        thiz_t* self, int width, int height, int y_pitch, int uv_pitch, int format, int profile,
        int field, const DpRectLegacy* roi, int secure, bool flush)
{
    DpRect rect;
    return gDp.BlitStream.setSrcConfig(INNER(self), width, height, y_pitch, uv_pitch, format,
                                       profile, field, convert_rect(&rect, roi), secure, flush);
}

/* DpBlitStream::setDstConfig(int, int, int, int, DP_COLOR_ENUM, DP_PROFILE_ENUM, DpInterlaceFormat, DpRect*, DpSecure, bool) */
int _ZN12DpBlitStream12setDstConfigEiiii13DP_COLOR_ENUM15DP_PROFILE_ENUM17DpInterlaceFormatP6DpRect8DpSecureb(
        thiz_t* self, int width, int height, int y_pitch, int uv_pitch, int format, int profile,
        int field, const DpRectLegacy* roi, int secure, bool flush)
{
    DpRect rect;
    return gDp.BlitStream.setDstConfig(INNER(self), width, height, y_pitch, uv_pitch, format,
                                       profile, field, convert_rect(&rect, roi), secure, flush);
}
