#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Health
PRODUCT_PACKAGES += \
    android.hardware.health@2.1-impl \
    android.hardware.health@2.1-impl.recovery \
    android.hardware.health@2.1-service

# HIDL
PRODUCT_PACKAGES += \
    libhidltransport \
    libhidltransport.vendor \
    libhwbinder \
    libhwbinder.vendor

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/tablet_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/tablet_core_hardware.xml

# Recovery
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/recovery/root/init.recovery.mt8168.rc:recovery/root/init.recovery.mt8168.rc

# Rootdir
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/ueventd.mt8168.rc:$(TARGET_COPY_OUT_VENDOR)/ueventd.rc

PRODUCT_PACKAGES += \
    init.mt8168.rc \
    init.mt8168.usb.rc

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH) \
    hardware/amazon

# Inherit the proprietary files
$(call inherit-product, vendor/amazon/mt8168-common/mt8168-common-vendor.mk)
