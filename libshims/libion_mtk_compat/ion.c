//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "libion_mtk_compat"

#include <dlfcn.h>
#include <stdint.h>
#include <string.h>

#include <log/log.h>

#define ION_LIB_NAME "libion_mtk.so"
#define ION_CUSTOM_IOCTL_SYM "ion_custom_ioctl"
#define ION_SYS_PARAM_SIZE 48

enum ION_CMDS {
	ION_CMD_SYSTEM,
	ION_CMD_MULTIMEDIA,
	ION_CMD_MULTIMEDIA_SEC
};

typedef struct ion_sys_data_legacy {
    uint32_t sys_cmd;
    uint8_t param[ION_SYS_PARAM_SIZE];
} ion_sys_data_legacy_t;

typedef struct ion_sys_data {
    uint32_t sys_cmd;
    union {
        uint64_t align;
        uint8_t param[ION_SYS_PARAM_SIZE];
    };
} ion_sys_data_t;

static int (*g_ion_custom_ioctl)(int fd, unsigned int cmd, void* arg);

__attribute__((constructor)) static void constructor(void)
{
    void* handle = dlopen(ION_LIB_NAME, RTLD_NOW);
    LOG_ALWAYS_FATAL_IF(!handle, "dlopen libion_mtk failed: %s",
        dlerror());

    g_ion_custom_ioctl = dlsym(handle, ION_CUSTOM_IOCTL_SYM);
    LOG_ALWAYS_FATAL_IF(g_ion_custom_ioctl == NULL, "failed to dlsym %s: %s",
        ION_CUSTOM_IOCTL_SYM, dlerror());
}

int ion_compat_ioctl(int fd, unsigned int cmd, void* arg)
{
#if !defined(__LP64__)
    if (cmd == ION_CMD_SYSTEM && arg != NULL) {
        ion_sys_data_legacy_t* legacy = arg;
        ion_sys_data_t data = { .sys_cmd = legacy->sys_cmd, };
        int rc;

        memcpy(data.param, legacy->param, sizeof(data.param));
        rc = g_ion_custom_ioctl(fd, cmd, &data);
        memcpy(legacy->param, data.param, sizeof(legacy->param));

        return rc;
    }
#endif

    return g_ion_custom_ioctl(fd, cmd, arg);
}
