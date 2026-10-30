//
// SPDX-FileCopyrightText: 2019 MediaTek Inc.
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef CAMERA_HAL_MEDIATEK_MTKCAM_UTILS_GRALLOC_MYUTILS_H_
#define CAMERA_HAL_MEDIATEK_MTKCAM_UTILS_GRALLOC_MYUTILS_H_

/****************************************************************************
 *
 *****************************************************************************/
//
#include <log/log.h>
#include <utils/Errors.h>
//
typedef int MERROR;

using android::BAD_VALUE;
using android::NAME_NOT_FOUND;
using android::NO_INIT;
using android::NO_MEMORY;
using android::OK;
using android::UNKNOWN_ERROR;

#define MY_LOGD(fmt, arg...) ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...) ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...) ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...) ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...) LOG_ALWAYS_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGE_IF(cond, ...)     \
  do {                            \
    if ((cond)) {                 \
      MY_LOGE(__VA_ARGS__);       \
    }                             \
  } while (0)
//
#define FUNC_START MY_LOGD("+")
#define FUNC_END MY_LOGD("-")

#endif  // CAMERA_HAL_MEDIATEK_MTKCAM_UTILS_GRALLOC_MYUTILS_H_
