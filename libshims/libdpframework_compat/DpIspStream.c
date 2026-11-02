//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "DpIspStream_compat"

#include <log/log.h>
#include <shim-helpers.h>

#include "DpFramework.h"

/* DpIspStream::DpIspStream(DpIspStream::ISPStreamType) */
thiz_t* _ZN11DpIspStreamC1ENS_13ISPStreamTypeE(thiz_t* self, int type)
{
    INNER_CONSTRUCT(self, DP_ISP_STREAM_SIZE, gDp.IspStream.ctor, type);
    return self;
}

/* DpIspStream::~DpIspStream() */
thiz_t* _ZN11DpIspStreamD1Ev(thiz_t* self)
{
    INNER_DESTRUCT(self, gDp.IspStream.dtor);
    return self;
}

/* DpIspStream::startStream(timeval*) */
int _ZN11DpIspStream11startStreamEP7timeval(thiz_t* self, void* end_time)
{
    /* R callers pass 30 here, startStream(timeval*, int) does not read it */
    return gDp.IspStream.startStream(INNER(self), end_time, 30);
}
