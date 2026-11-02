//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "DpVEncStream_compat"

#include <log/log.h>
#include <shim-helpers.h>

#include "DpFramework.h"

/* DpVEncStream::DpVEncStream(DpVEncStream::ISPStreamType) */
thiz_t* _ZN12DpVEncStreamC1ENS_13ISPStreamTypeE(thiz_t* self, int type)
{
    INNER_CONSTRUCT(self, DP_VENC_STREAM_SIZE, gDp.VEncStream.ctor, type);
    return self;
}

/* DpVEncStream::~DpVEncStream() */
thiz_t* _ZN12DpVEncStreamD1Ev(thiz_t* self)
{
    INNER_DESTRUCT(self, gDp.VEncStream.dtor);
    return self;
}
