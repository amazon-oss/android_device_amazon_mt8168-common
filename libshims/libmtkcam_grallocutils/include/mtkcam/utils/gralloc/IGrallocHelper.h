//
// SPDX-FileCopyrightText: 2019 MediaTek Inc.
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef CAMERA_HAL_MEDIATEK_MTKCAM_INCLUDE_MTKCAM_UTILS_GRALLOC_IGRALLOCHELPER_H_
#define CAMERA_HAL_MEDIATEK_MTKCAM_INCLUDE_MTKCAM_UTILS_GRALLOC_IGRALLOCHELPER_H_
//
#include <system/graphics.h>
#include <hardware/gralloc.h>
#include <utils/String8.h>
#include <utils/Vector.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/**
 *
 */
struct GrallocRequest {
  /**
   * The gralloc usage.
   */
  int usage;

  /**
   * The image format to request.
   */
  int format;

  /**
   * The image width in pixels. For formats where some color channels are
   * subsampled, this is the width of the largest-resolution plane.
   */
  int widthInPixels;

  /**
   * The image height in pixels. For formats where some color channels are
   * subsampled, this is the height of the largest-resolution plane.
   */
  int heightInPixels;
};

/**
 *
 */
struct GrallocStaticInfo {
  struct Plane {
    /**
     * The size for this color plane, in bytes.
     */
    size_t sizeInBytes;

    /**
     * The row stride for this color plane, in bytes.
     *
     * This is the distance between the start of two consecutive rows of
     * pixels in the image. The row stride is always greater than 0.
     */
    size_t rowStrideInBytes;
  };

  /**
   * The resulting image format.
   */
  int format;

  /**
   * The image width in pixels. For formats where some color channels are
   * subsampled, this is the width of the largest-resolution plane.
   */
  int widthInPixels;

  /**
   * The image height in pixels. For formats where some color channels are
   * subsampled, this is the height of the largest-resolution plane.
   */
  int heightInPixels;

  /**
   * A vector of planes.
   */
  android::Vector<Plane> planes;
};

/**
 *
 */
struct GrallocDynamicInfo {
  /**
   * A vector of ion Fds.
   */
  android::Vector<int> ionFds;
};

/**
 *
 */
class IGrallocHelper {
 public:  ////                Instantiation.
  virtual ~IGrallocHelper() {}

  /**
   * Get a singleton instance.
   */
  static IGrallocHelper* singleton();

 public:  ////                Interfaces.
  /**
   * Given a gralloc request, return its information.
   */
  virtual int query(struct GrallocRequest const* pRequest,
                    struct GrallocStaticInfo* pStaticInfo) const = 0;

  /**
   * Given a gralloc buffer handle, return its information.
   */
  virtual int query(buffer_handle_t bufHandle,
                    struct GrallocStaticInfo* pStaticInfo,
                    struct GrallocDynamicInfo* pDynamicInfo) const = 0;

  /**
   * Mark a gralloc buffer as a dirty camera buffer.
   */
  virtual int setDirtyCamera(buffer_handle_t bufHandle) const = 0;

  /**
   * Set the colorspace of a gralloc buffer to BT.601 full range.
   */
  virtual int setColorspace_JFIF(buffer_handle_t bufHandle) const = 0;

  /**
   * Given a HAL_PIXEL_FORMAT_xxx format, return a String8 name.
   */
  virtual android::String8 queryPixelFormatName(int format) const = 0;

  /**
   * Given a combination of usage, return a String8 name.
   */
  virtual android::String8 queryGrallocUsageName(int usage) const = 0;

  /**
   * Given a dataspace, return a String8 name.
   */
  virtual android::String8 queryDataspaceName(int32_t dataspace) const = 0;

  /**
   * Dump to the log for debug.
   */
  virtual void dumpToLog() const = 0;

  /**
   * Import a gralloc buffer handle.
   */
  virtual bool importBuffer(buffer_handle_t& bufHandle) = 0;

  /**
   * Free an imported gralloc buffer handle.
   */
  virtual void freeBuffer(buffer_handle_t bufHandle) = 0;

  /**
   * Lock a gralloc buffer for CPU access.
   */
  virtual int lock(buffer_handle_t bufHandle,
                   unsigned int usage,
                   int width,
                   int height,
                   void** vaddr) = 0;

  /**
   * Unlock a gralloc buffer.
   */
  virtual int unlock(buffer_handle_t bufHandle) = 0;

  /**
   * Update the status bits of a gralloc buffer.
   */
  virtual int setBufferStatus(buffer_handle_t bufHandle,
                              int mask,
                              int value) const = 0;
};

};  // namespace NSCam
/******************************************************************************
 *
 ******************************************************************************/
#endif  // CAMERA_HAL_MEDIATEK_MTKCAM_INCLUDE_MTKCAM_UTILS_GRALLOC_IGRALLOCHELPER_H_
