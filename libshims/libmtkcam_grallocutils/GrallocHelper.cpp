//
// SPDX-FileCopyrightText: 2019 MediaTek Inc.
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "MtkCam/GrallocHelper"
//
#include <cutils/properties.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <map>
#include <mtkcam/utils/gralloc/IGrallocHelper.h>
#include <mutex>
#include <stdlib.h>
#include <string.h>
#include <ui/Gralloc.h>
#include <ui/GraphicBufferAllocator.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>
#include <utility>
#include <utils/Timers.h>

#include "MyUtils.h"

using android::GraphicBufferAllocator;
using android::GraphicBufferMapper;
using android::Rect;
using android::String8;
using NSCam::GrallocDynamicInfo;
using NSCam::GrallocRequest;
using NSCam::GrallocStaticInfo;
using NSCam::IGrallocHelper;

/******************************************************************************
 *
 ******************************************************************************/
#define GRALLOC_ALIGN(value, base) (((value) + ((base)-1)) & ~((base)-1))
#define ADDRESS_OFFSET(base1, base2) (intptr_t(base1) - intptr_t(base2))
/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
enum {
  HAL_PIXEL_FORMAT_CAMERA_OPAQUE = 0x111,
};

/**
 *
 */
enum {
  GRALLOC_EXTRA_GET_ION_FD = 1,
  GRALLOC_EXTRA_GET_WIDTH = 10,
  GRALLOC_EXTRA_GET_HEIGHT = 11,
  GRALLOC_EXTRA_GET_STRIDE = 12,
  GRALLOC_EXTRA_GET_VERTICAL_STRIDE = 13,
  GRALLOC_EXTRA_GET_ALLOC_SIZE = 14,
  GRALLOC_EXTRA_GET_FORMAT = 15,
  GRALLOC_EXTRA_GET_USAGE = 17,
  GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO = 100,
};

enum {
  GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO = 100,
};

/**
 *
 */
struct GrallocExtra {
  int (*query)(buffer_handle_t handle, int attribute, void* out_pointer);
  int (*perform)(buffer_handle_t handle, int attribute, void* in_pointer);
  int (*sf_set_status)(void* sf_info, int mask, int value);
};

GrallocExtra* g_extra;

/**
 *
 */
String8 gQueryPixelFormatName(int format) {
#define _ENUM_TO_NAME_(_prefix_, _format_) \
  case _prefix_##_format_: {               \
    static String8 name(#_format_);        \
    return name;                           \
  } break

  switch (format) {
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGBA_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGBX_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGB_888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGB_565);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, BGRA_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YV12);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, Y8);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, Y16);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, BLOB);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, IMPLEMENTATION_DEFINED);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RAW_OPAQUE);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_420_888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_422_SP);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCrCb_420_SP);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_422_I);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RAW16);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, CAMERA_OPAQUE);
    default:
      break;
  }

#undef _ENUM_TO_NAME_
  String8 str;
  //
  return str;
}

/**
 *
 */
String8 gQueryGrallocUsageName(int usage) {
#define _USAGE_TO_NAME_(_prefix_, _usage_) \
  case _prefix_##_usage_: {                \
    str += "|" #_usage_;                   \
  } break

  String8 str("0");
  //
  switch ((usage & GRALLOC_USAGE_SW_READ_MASK)) {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_READ_RARELY);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_READ_OFTEN);
    default:
      break;
  }
  //
  switch ((usage & GRALLOC_USAGE_SW_WRITE_MASK)) {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_WRITE_RARELY);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_WRITE_OFTEN);
    default:
      break;
  }
  //
  switch ((usage & GRALLOC_USAGE_HW_CAMERA_MASK)) {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_WRITE);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_READ);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_ZSL);
    default:
      break;
  }
  //
  if ((usage & GRALLOC_USAGE_HW_MASK) != 0) {
#define _USAGE_TO_NAME_OR_(_prefix_, _usage_) \
  if ((usage & _prefix_##_usage_)) {          \
    str += "|" #_usage_;                      \
  }

    _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_TEXTURE);
    _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_RENDER);
    _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_2D);
    _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_COMPOSER);
    _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_FB);
    _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_VIDEO_ENCODER);

#undef _USAGE_TO_NAME_OR_
  }
  //
#undef _USAGE_TO_NAME_
  return str;
}

/**
 *
 */
String8 gQueryDataspaceName(int32_t dataspace) {
#define _ENUM_TO_NAME_(_prefix_, _dataspace_) \
  case _prefix_##_dataspace_: {               \
    str = #_dataspace_;                       \
  } break

#define _ENUM_TO_NAME_OR_(_prefix_, _dataspace_) \
  case _prefix_##_dataspace_: {                  \
    str += "|" #_dataspace_;                     \
  } break

  String8 str;
  //
  switch (dataspace) {
    _ENUM_TO_NAME_(HAL_DATASPACE_, UNKNOWN);
    _ENUM_TO_NAME_(HAL_DATASPACE_, ARBITRARY);
    _ENUM_TO_NAME_(HAL_DATASPACE_, JFIF);
    _ENUM_TO_NAME_(HAL_DATASPACE_, BT601_625);
    _ENUM_TO_NAME_(HAL_DATASPACE_, BT601_525);
    _ENUM_TO_NAME_(HAL_DATASPACE_, BT709);
    _ENUM_TO_NAME_(HAL_DATASPACE_, SRGB_LINEAR);
    _ENUM_TO_NAME_(HAL_DATASPACE_, SRGB);
    _ENUM_TO_NAME_(HAL_DATASPACE_, DEPTH);
    _ENUM_TO_NAME_(HAL_DATASPACE_, SENSOR);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_SRGB_LINEAR);
    _ENUM_TO_NAME_(HAL_DATASPACE_, BT2020_LINEAR);
    _ENUM_TO_NAME_(HAL_DATASPACE_, DCI_P3_LINEAR);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_SRGB);
    _ENUM_TO_NAME_(HAL_DATASPACE_, DISPLAY_P3);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_JFIF);
    _ENUM_TO_NAME_(HAL_DATASPACE_, BT2020);
    _ENUM_TO_NAME_(HAL_DATASPACE_, ADOBE_RGB);
    _ENUM_TO_NAME_(HAL_DATASPACE_, DCI_P3);
    _ENUM_TO_NAME_(HAL_DATASPACE_, BT2020_PQ);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_BT709);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_BT601_625);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_BT601_525);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_SCRGB_LINEAR);
    _ENUM_TO_NAME_(HAL_DATASPACE_, V0_SCRGB);
    default:
      str = String8::format("%#x", dataspace);
      break;
  }
  //
  switch ((dataspace & HAL_DATASPACE_STANDARD_MASK)) {
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT709);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT601_625);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT601_625_UNADJUSTED);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT601_525);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT601_525_UNADJUSTED);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT2020);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT2020_CONSTANT_LUMINANCE);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_BT470M);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_FILM);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_DCI_P3);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, STANDARD_ADOBE_RGB);
    default:
      break;
  }
  //
  switch ((dataspace & HAL_DATASPACE_TRANSFER_MASK)) {
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_LINEAR);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_SRGB);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_SMPTE_170M);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_GAMMA2_2);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_GAMMA2_6);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_GAMMA2_8);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_ST2084);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, TRANSFER_HLG);
    default:
      break;
  }
  //
  switch ((dataspace & HAL_DATASPACE_RANGE_MASK)) {
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, RANGE_FULL);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, RANGE_LIMITED);
    _ENUM_TO_NAME_OR_(HAL_DATASPACE_, RANGE_EXTENDED);
    default:
      break;
  }
  //
#undef _ENUM_TO_NAME_OR_
#undef _ENUM_TO_NAME_
  return str;
}

};  // namespace

/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
struct MyStaticInfo : public GrallocStaticInfo {
  int64_t allocTimeNs;
  int usage;
  size_t allocSize;
  //
  MyStaticInfo() {
    format = 0;
    usage = 0;
    widthInPixels = 0;
    heightInPixels = 0;
    allocTimeNs = 0;
    allocSize = 0;
  }
};

/**
 *
 */
struct MyExtraInfo {
  int format;
  int usage;
  int widthInPixels;
  int heightInPixels;
  int stride;
  int verticalStride;
  int allocSize;
};
};  // namespace

/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
MERROR
gQueryExtra(buffer_handle_t const bh, MyExtraInfo* info) {
  int err = 0;
  //
  info->format = 0;
  err |= g_extra->query(bh, GRALLOC_EXTRA_GET_FORMAT, &info->format);
  info->usage = 0;
  err |= g_extra->query(bh, GRALLOC_EXTRA_GET_USAGE, &info->usage);
  info->widthInPixels = 0;
  err |= g_extra->query(bh, GRALLOC_EXTRA_GET_WIDTH, &info->widthInPixels);
  info->heightInPixels = 0;
  err |= g_extra->query(bh, GRALLOC_EXTRA_GET_HEIGHT, &info->heightInPixels);
  info->stride = 0;
  err |= g_extra->query(bh, GRALLOC_EXTRA_GET_STRIDE, &info->stride);
  info->verticalStride = 0;
  err |= g_extra->query(bh, GRALLOC_EXTRA_GET_VERTICAL_STRIDE,
                        &info->verticalStride);
  info->allocSize = 0;
  err |= g_extra->query(bh, GRALLOC_EXTRA_GET_ALLOC_SIZE, &info->allocSize);
  //
  if (err) {
    MY_LOGE("Given buffer_handle_t:%p, gralloc_extra_query returns %d", bh,
            err);
    return UNKNOWN_ERROR;
  }
  //
  return OK;
}

/**
 *
 */
MERROR
gQueryYCbCr(buffer_handle_t const bh,
            MyExtraInfo const& info,
            android_ycbcr* ycbcr) {
  *ycbcr = {};
  int status = GraphicBufferMapper::get().lockYCbCr(
      bh, GRALLOC_USAGE_SW_READ_OFTEN,
      Rect(info.widthInPixels, info.heightInPixels), ycbcr);
  if (status == OK) {
    GraphicBufferMapper::get().unlock(bh);
  }
  //
  MY_LOGD(
      "cr-y:%zu cb-cr:%zu y:%p cb:%p cr:%p ystride:%zu cstride:%zu "
      "chroma_step:%zu status:%d",
      size_t(ADDRESS_OFFSET(ycbcr->cr, ycbcr->y)),
      size_t(ADDRESS_OFFSET(ycbcr->cb, ycbcr->cr)),
      ycbcr->y, ycbcr->cb, ycbcr->cr, ycbcr->ystride, ycbcr->cstride,
      ycbcr->chroma_step, status);
  //
  if (status != OK) {
    MY_LOGF("status:%d(%s)", status, strerror(-status));
  }
  //
  return OK;
}

/**
 *
 */
MERROR
queryStaticInfo(buffer_handle_t const bh, MyStaticInfo* info) {
  MyExtraInfo extra;
  if (OK != gQueryExtra(bh, &extra)) {
    return UNKNOWN_ERROR;
  }
  //
  info->format = extra.format;
  info->usage = extra.usage;
  info->widthInPixels = extra.widthInPixels;
  info->heightInPixels = extra.heightInPixels;
  info->allocSize = extra.allocSize;

  switch (info->format) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888: {
      info->planes.resize(1);
      {
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = extra.stride * 4;
        plane.sizeInBytes = info->allocSize;
      }
    } break;
    case HAL_PIXEL_FORMAT_RGB_888: {
      info->planes.resize(1);
      {
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = extra.stride * 3;
        plane.sizeInBytes = info->allocSize;
      }
    } break;
    case HAL_PIXEL_FORMAT_RAW16: {
      info->planes.resize(1);
      {
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = extra.stride * 2;
        plane.sizeInBytes = info->allocSize;
      }
    } break;
    case HAL_PIXEL_FORMAT_Y8:
    case HAL_PIXEL_FORMAT_BLOB:
    case HAL_PIXEL_FORMAT_RAW_OPAQUE: {
      info->planes.resize(1);
      {
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = extra.stride;
        plane.sizeInBytes = info->allocSize;
      }
    } break;
    case HAL_PIXEL_FORMAT_YCbCr_422_I: {
      info->planes.resize(1);
      {
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = extra.stride * 2;
        plane.sizeInBytes = info->allocSize;
        if (plane.rowStrideInBytes % 32 != 0) {
          MY_LOGW("[YCbCr_422_I] rowStrideInBytes:%zu is not a multiple of 32",
                  plane.rowStrideInBytes);
        }
        if (plane.sizeInBytes < plane.rowStrideInBytes * 16) {
          MY_LOGW("[YCbCr_422_I] sizeInBytes:%zu<rowStrideInBytes:%zu*16",
                  plane.sizeInBytes, plane.rowStrideInBytes);
        }
      }
    } break;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP: {
      android_ycbcr ycbcr;
      gQueryYCbCr(bh, extra, &ycbcr);
      info->planes.resize(2);
      {
        // Y
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = ycbcr.ystride;
        plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
      }
      {
        // CrCb
        MyStaticInfo::Plane& plane = info->planes.editItemAt(1);
        plane.rowStrideInBytes = ycbcr.cstride;
        plane.sizeInBytes = info->allocSize - ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
      }
    } break;
    case HAL_PIXEL_FORMAT_YV12: {
      android_ycbcr ycbcr;
      gQueryYCbCr(bh, extra, &ycbcr);
      info->planes.resize(3);
      {
        // Y
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = ycbcr.ystride;
        plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
      }
      {
        // Cr
        MyStaticInfo::Plane& plane = info->planes.editItemAt(1);
        plane.rowStrideInBytes = ycbcr.cstride;
        plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cb, ycbcr.cr);
      }
      {
        // Cb
        MyStaticInfo::Plane& plane = info->planes.editItemAt(2);
        plane.rowStrideInBytes = ycbcr.cstride;
        plane.sizeInBytes = info->allocSize - ADDRESS_OFFSET(ycbcr.cb, ycbcr.y);
      }
    } break;
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
    case HAL_PIXEL_FORMAT_CAMERA_OPAQUE: {
      MY_LOGD("only in zsl mode: %#x(%s)", info->format,
              gQueryPixelFormatName(info->format).c_str());
      info->planes.resize(1);
      {
        MyStaticInfo::Plane& plane = info->planes.editItemAt(0);
        plane.rowStrideInBytes = (extra.stride * 5) >> 2;
        plane.sizeInBytes = info->allocSize;
      }
    } break;
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
      MY_LOGE("format from gralloc_extra_query should not be: %#x(%s)",
              info->format, gQueryPixelFormatName(info->format).c_str());
      return UNKNOWN_ERROR;
    default:
      MY_LOGE("Not support format: %#x(%s)", info->format,
              gQueryPixelFormatName(info->format).c_str());
      return NAME_NOT_FOUND;
  }
  //
  return OK;
}
};  // namespace

/******************************************************************************
 *
 ******************************************************************************/
namespace {
class GrallocHelperImp : public IGrallocHelper {
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  Definitions.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 public:  ////
  /**
   *
   */
  struct MyRequest : public GrallocRequest {
    explicit MyRequest(GrallocRequest const& rhs = GrallocRequest())
        : GrallocRequest(rhs) {}

    bool operator==(MyRequest const& rhs) const {
      return usage == rhs.usage && format == rhs.format &&
             widthInPixels == rhs.widthInPixels &&
             heightInPixels == rhs.heightInPixels;
    }

    bool operator<(MyRequest const& rhs) const {
      /*
          true : if lhs < rhs
          false: if lhs >= rhs
       */
      if (format != rhs.format) {
        return format < rhs.format;
      }
      if (usage != rhs.usage) {
        return usage < rhs.usage;
      }
      if (widthInPixels != rhs.widthInPixels) {
        return widthInPixels < rhs.widthInPixels;
      }
      if (heightInPixels != rhs.heightInPixels) {
        return heightInPixels < rhs.heightInPixels;
      }
      //
      //  here: lhs >= rhs
      return false;
    }
  };
  typedef std::map<MyRequest, MyStaticInfo> Map_t;

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  Implementations.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 protected:  ////                Data Members.
  mutable std::mutex mLock;
  mutable Map_t mMap;
  bool mInitialized;
  int mLogLevel;

 protected:  ////                Operations.
  MERROR initialize();

  static MERROR determine(GrallocRequest const& src, MyRequest* dst);
  static MERROR determine(MyRequest const& src, MyStaticInfo* dst);

  ssize_t addToMapLocked(MyRequest const& k, MyStaticInfo const& v) const;

  void dump(MyRequest const& k, MyStaticInfo const& v) const;

 public:  ////                Operations.
  GrallocHelperImp();
  ~GrallocHelperImp();

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  IGrallocHelper Interfaces.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 public:  ////                Operations.
  virtual int query(struct GrallocRequest const* pRequest,
                    struct GrallocStaticInfo* pStaticInfo) const;

  virtual int query(buffer_handle_t bufHandle,
                    struct GrallocStaticInfo* pStaticInfo,
                    struct GrallocDynamicInfo* pDynamicInfo) const;

  virtual int setDirtyCamera(buffer_handle_t bufHandle) const {
    return setBufferStatus(bufHandle, 0xc03, 0xc03);
  }

  virtual int setColorspace_JFIF(buffer_handle_t bufHandle) const {
    return setBufferStatus(bufHandle, 0xe0000000, 0x40000000);
  }

  virtual String8 queryPixelFormatName(int format) const {
    return gQueryPixelFormatName(format);
  }

  virtual String8 queryGrallocUsageName(int usage) const {
    return gQueryGrallocUsageName(usage);
  }

  virtual String8 queryDataspaceName(int32_t dataspace) const {
    return gQueryDataspaceName(dataspace);
  }

  virtual void dumpToLog() const;

  virtual bool importBuffer(buffer_handle_t& bufHandle);

  virtual void freeBuffer(buffer_handle_t bufHandle);

  virtual int lock(buffer_handle_t bufHandle,
                   unsigned int usage,
                   int width,
                   int height,
                   void** vaddr);

  virtual int unlock(buffer_handle_t bufHandle);

  virtual int setBufferStatus(buffer_handle_t bufHandle,
                              int mask,
                              int value) const;
};

GrallocHelperImp* getSingleton() {
  static GrallocHelperImp* tmp = new GrallocHelperImp();
  return tmp;
}

};  // namespace

/******************************************************************************
 *
 ******************************************************************************/
IGrallocHelper* IGrallocHelper::singleton() {
  return getSingleton();
}

/******************************************************************************
 *
 ******************************************************************************/
GrallocHelperImp::GrallocHelperImp() : mInitialized(false), mLogLevel(0) {
  FUNC_START;
  //
  mLogLevel = property_get_int32("vendor.debug.camera.log", 0);
  if (mLogLevel == 0) {
    mLogLevel = property_get_int32("debug.mtkcam.utils.gralloc.loglevel", 0);
  }
  //
  MERROR err = initialize();
  MY_LOGE_IF(err, "%s initialize fail", __FUNCTION__);
  FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
GrallocHelperImp::~GrallocHelperImp() {
  FUNC_START;
  g_extra = nullptr;
  FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
GrallocHelperImp::initialize() {
  FUNC_START;

  void* handle = dlopen("libgralloc_extra.so", RTLD_NOW);
  if (handle == nullptr) {
    MY_LOGE("dlopen libgralloc_extra.so failed: %s", dlerror());
    return NO_INIT;
  }
  //
  static GrallocExtra extra;
  extra.query = reinterpret_cast<decltype(extra.query)>(
      dlsym(handle, "gralloc_extra_query"));
  extra.perform = reinterpret_cast<decltype(extra.perform)>(
      dlsym(handle, "gralloc_extra_perform"));
  extra.sf_set_status = reinterpret_cast<decltype(extra.sf_set_status)>(
      dlsym(handle, "gralloc_extra_sf_set_status"));
  if (extra.query == nullptr || extra.perform == nullptr ||
      extra.sf_set_status == nullptr) {
    MY_LOGE("dlsym libgralloc_extra.so failed: %s", dlerror());
    return NO_INIT;
  }
  g_extra = &extra;
  //
  mInitialized = true;
  FUNC_END;
  //
  return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int GrallocHelperImp::query(struct GrallocRequest const* pRequest,
                            struct GrallocStaticInfo* pStaticInfo) const {
  MERROR status = OK;
  //
  if (!pRequest) {
    MY_LOGE("NULL pRequest");
    return BAD_VALUE;
  }
  //
  if (!pStaticInfo) {
    MY_LOGE("NULL pStaticInfo");
    return BAD_VALUE;
  }
  //
  MY_LOGD("Grallc Request: u(0x%x) f(0x%x) w(%d) h(%d)", pRequest->usage,
          pRequest->format, pRequest->widthInPixels, pRequest->heightInPixels);
  MyRequest request;
  determine(*pRequest, &request);
  //
  std::lock_guard<std::mutex> _l(mLock);
  Map_t::iterator it;
  it = mMap.find(request);
  //
  if (it == mMap.end()) {
    MyStaticInfo staticInfo;
    status = determine(request, &staticInfo);
    if (OK != status) {
      return status;
    }

    addToMapLocked(request, staticInfo);
    *pStaticInfo = staticInfo;
    switch (request.format) {
      case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
      case HAL_PIXEL_FORMAT_YCbCr_420_888: {
        request.format = staticInfo.format;
        request.usage = 0;
        auto iter = mMap.find(request);
        if (iter == mMap.end()) {
          addToMapLocked(request, staticInfo);
        }
      } break;
      default:
        break;
    }
  } else {
    *pStaticInfo = it->second;
  }
  //
  return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int GrallocHelperImp::query(buffer_handle_t bufHandle,
                            struct GrallocStaticInfo* pStaticInfo,
                            struct GrallocDynamicInfo* pDynamicInfo) const {
  MERROR status = OK;
  //
  if (!bufHandle) {
    MY_LOGE("NULL buffer_handle_t");
    return BAD_VALUE;
  }
  //
  MyExtraInfo extra;
  if (OK != gQueryExtra(bufHandle, &extra)) {
    return UNKNOWN_ERROR;
  }
  //
  {
    MyRequest request;
    request.usage = 0;
    request.format = extra.format;
    request.widthInPixels = extra.widthInPixels;
    request.heightInPixels = extra.heightInPixels;
    //
    std::lock_guard<std::mutex> _l(mLock);
    Map_t::iterator it;
    it = mMap.find(request);
    if (it != mMap.end()) {
      if (NULL != pStaticInfo) {
        *pStaticInfo = it->second;
      }
    } else {
      MY_LOGW("Not found: %dx%d %s", request.widthInPixels,
              request.heightInPixels,
              gQueryPixelFormatName(request.format).c_str());
      MyStaticInfo staticInfo;
      status = queryStaticInfo(bufHandle, &staticInfo);
      if (OK != status) {
        return status;
      }
      //
      //  Add it to map if not existing.
      MyRequest request;
      request.usage = 0;
      request.format = staticInfo.format;
      request.widthInPixels = staticInfo.widthInPixels;
      request.heightInPixels = staticInfo.heightInPixels;

      //
      Map_t::iterator it;
      it = mMap.find(request);
      if (it == mMap.end()) {
        addToMapLocked(request, staticInfo);
      }
      //
      if (NULL != pStaticInfo) {
        *pStaticInfo = staticInfo;
      }
    }
  }
  //
  if (NULL != pDynamicInfo) {
    int ionFd = -1;
    int err =
        g_extra->query(bufHandle, GRALLOC_EXTRA_GET_ION_FD, &ionFd);
    if (err || ionFd < 0) {
      MY_LOGE("[buffer_handle_t:%p] err:%d ionFd:%d", bufHandle, err, ionFd);
      return UNKNOWN_ERROR;
    }
    pDynamicInfo->ionFds.push_back(ionFd);
  }
  //
  return status;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
GrallocHelperImp::determine(GrallocRequest const& src, MyRequest* dst) {
  dst->usage = src.usage;
  dst->format = src.format;
  dst->widthInPixels = src.widthInPixels;
  dst->heightInPixels = src.heightInPixels;

  //
  switch (src.format) {
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
      dst->usage = src.usage;
      break;
    default:
      dst->usage = 0;
      break;
  }

  //
  return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
GrallocHelperImp::determine(MyRequest const& src, MyStaticInfo* dst) {
  //
  // already protected by mLock.
  // allocate null buffer for query static info.
  // free buffer handle before end of this function.
  nsecs_t startTime = systemTime(SYSTEM_TIME_MONOTONIC);
  uint32_t outStride = 0;
  buffer_handle_t handle = nullptr;
  int err = GraphicBufferAllocator::get().allocate(
      src.widthInPixels ?: 1, src.heightInPixels ?: 1, src.format, 1,
      static_cast<uint64_t>(static_cast<int64_t>(src.usage)),
      &handle, &outStride, "determine");
  if (err != OK || handle == nullptr) {
    MY_LOGE("Bad allcation handle:%p %dx%d format:%#x(%s) usage:%#x(%s)",
            handle, src.widthInPixels, src.heightInPixels, src.format,
            gQueryPixelFormatName(src.format).c_str(), src.usage,
            gQueryGrallocUsageName(src.usage).c_str());
    return NO_MEMORY;
  }
  dst->allocTimeNs = systemTime(SYSTEM_TIME_MONOTONIC) - startTime;
  err = queryStaticInfo(handle, dst);
  //
  if (handle) {
    GraphicBufferAllocator::get().free(handle);
  }

  return err;
}

/******************************************************************************
 *
 ******************************************************************************/
ssize_t GrallocHelperImp::addToMapLocked(MyRequest const& k,
                                         MyStaticInfo const& v) const {
  dump(k, v);
  mMap.insert(std::pair<MyRequest, MyStaticInfo>(k, v));
  return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
void GrallocHelperImp::dumpToLog() const {
  MY_LOGD("+");
  std::lock_guard<std::mutex> _l(mLock);
  for (auto& i : mMap) {
    dump(i.first, i.second);
  }
  MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
void GrallocHelperImp::dump(MyRequest const& k, MyStaticInfo const& v) const {
  MY_LOGD(
      "************************************************************************"
      "*****");
  MY_LOGD("gralloc time: %" PRId64 " us", v.allocTimeNs / 1000);
  MY_LOGD("%dx%d usage:0x%08x(%s) format:0x%08x(%s) -->", k.widthInPixels,
          k.heightInPixels, k.usage, queryGrallocUsageName(k.usage).c_str(),
          k.format, queryPixelFormatName(k.format).c_str());
  MY_LOGD("%dx%d usage:0x%08x(%s) format:0x%08x(%s)", v.widthInPixels,
          v.heightInPixels, v.usage, queryGrallocUsageName(v.usage).c_str(),
          v.format, queryPixelFormatName(v.format).c_str());
  for (size_t i = 0; i < v.planes.size(); i++) {
    MY_LOGD("  [%zu] sizeInBytes:%zu rowStrideInBytes:%zu", i,
            v.planes[i].sizeInBytes, v.planes[i].rowStrideInBytes);
  }
}

/******************************************************************************
 *
 ******************************************************************************/
bool GrallocHelperImp::importBuffer(buffer_handle_t& bufHandle) {
  if (bufHandle->numFds == 0 && bufHandle->numInts == 0) {
    bufHandle = nullptr;
    return true;
  }
  //
  buffer_handle_t outHandle = nullptr;
  int err = GraphicBufferMapper::get().getGrallocMapper().importBuffer(
      android::hardware::hidl_handle(bufHandle), &outHandle);
  if (err != OK) {
    MY_LOGE("importBuffer(%p) failed: %d", bufHandle, err);
    return false;
  }
  //
  bufHandle = outHandle;
  return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void GrallocHelperImp::freeBuffer(buffer_handle_t bufHandle) {
  if (bufHandle) {
    GraphicBufferMapper::get().freeBuffer(bufHandle);
  }
}

/******************************************************************************
 *
 ******************************************************************************/
int GrallocHelperImp::lock(buffer_handle_t bufHandle,
                           unsigned int usage,
                           int width,
                           int height,
                           void** vaddr) {
  int err = GraphicBufferMapper::get().lock(bufHandle, usage,
                                            Rect(width, height), vaddr);
  if (err != OK) {
    MY_LOGE("lock(...) failed %d (%s)", err, strerror(-err));
  }
  //
  return err;
}

/******************************************************************************
 *
 ******************************************************************************/
int GrallocHelperImp::unlock(buffer_handle_t bufHandle) {
  GraphicBufferMapper::get().unlock(bufHandle);
  return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int GrallocHelperImp::setBufferStatus(buffer_handle_t bufHandle,
                                      int mask,
                                      int value) const {
  uint32_t sfInfo[16] = {};
  //
  g_extra->query(bufHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, sfInfo);
  g_extra->sf_set_status(sfInfo, mask, value);
  return g_extra->perform(bufHandle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO,
                          sfInfo);
}
