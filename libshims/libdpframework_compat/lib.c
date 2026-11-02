//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "libdpframework_compat"

#include <log/log.h>
#include <shim-helpers.h>

#include "DpFramework.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const char* const sym_names[] = {
/* make MEMBER / STATIC stringify symbol names instead */
#define MEMBER(sym) #sym,
#define STATIC(sym) #sym,
#include "symbols.h"
#undef MEMBER
#undef STATIC
};

__attribute__((visibility("hidden"))) void* func_ptr_tbl[ARRAY_SIZE(sym_names)];
__attribute__((visibility("hidden"))) DpFramework gDp;

__attribute__((constructor)) static void constructor(void)
{
    void* handle = dlopen(DPFRAMEWORK_LIB_NAME, RTLD_NOW);
    LOG_ALWAYS_FATAL_IF(!handle, "dlopen %s failed: %s",
        DPFRAMEWORK_LIB_NAME, dlerror());

    for (size_t i = 0; i < ARRAY_SIZE(sym_names); i++)
    {
        func_ptr_tbl[i] = sdlsym(handle, sym_names[i]);
    }

    /* IspStream */
    gDp.IspStream.ctor = sdlsym(handle, "_ZN11DpIspStreamC1ENS_13ISPStreamTypeE");
    gDp.IspStream.dtor = sdlsym(handle, "_ZN11DpIspStreamD1Ev");
    gDp.IspStream.startStream = sdlsym(handle, "_ZN11DpIspStream11startStreamEP7timevali");

    /* BlitStream */
    gDp.BlitStream.ctor = sdlsym(handle, "_ZN12DpBlitStreamC1Ev");
    gDp.BlitStream.dtor = sdlsym(handle, "_ZN12DpBlitStreamD1Ev");
    gDp.BlitStream.setSrcConfig = sdlsym(handle,
            "_ZN12DpBlitStream12setSrcConfigEiiii13DP_COLOR_ENUM15DP_PROFILE_ENUM17DpInterlaceFormatP6DpRect8DpSecureb");
    gDp.BlitStream.setDstConfig = sdlsym(handle,
            "_ZN12DpBlitStream12setDstConfigEiiii13DP_COLOR_ENUM15DP_PROFILE_ENUM17DpInterlaceFormatP6DpRect8DpSecureb");

    /* VEncStream */
    gDp.VEncStream.ctor = sdlsym(handle, "_ZN12DpVEncStreamC1ENS_13ISPStreamTypeE");
    gDp.VEncStream.dtor = sdlsym(handle, "_ZN12DpVEncStreamD1Ev");
}
