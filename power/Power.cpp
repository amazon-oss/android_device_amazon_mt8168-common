/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "mtkpower-tap2wake"

#include <aidl/android/hardware/power/BnPower.h>
#include <android-base/file.h>
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <dlfcn.h>

#include <string>

using ::aidl::android::hardware::power::BnPower;
using ::aidl::android::hardware::power::Boost;
using ::aidl::android::hardware::power::IPower;
using ::aidl::android::hardware::power::Mode;
using ::ndk::ScopedAStatus;
using ::ndk::SharedRefBase;
using ::ndk::SpAIBinder;

namespace {

constexpr const char* kTapToWakeNode = "/sys/touchscreen/link_input_dev/gesture_wakeup";

class Power : public BnPower {
  public:
    explicit Power(std::shared_ptr<IPower> impl) : mImpl(std::move(impl)) {}

    ScopedAStatus setMode(Mode type, bool enabled) override {
        if (type == Mode::DOUBLE_TAP_TO_WAKE) {
            if (!android::base::WriteStringToFile(enabled ? "1" : "0", kTapToWakeNode)) {
                PLOG(ERROR) << "Failed to write " << kTapToWakeNode;
            }
            return ScopedAStatus::ok();
        }
        return mImpl->setMode(type, enabled);
    }

    ScopedAStatus isModeSupported(Mode type, bool* _aidl_return) override {
        if (type == Mode::DOUBLE_TAP_TO_WAKE) {
            *_aidl_return = true;
            return ScopedAStatus::ok();
        }
        return mImpl->isModeSupported(type, _aidl_return);
    }

    ScopedAStatus setBoost(Boost type, int32_t durationMs) override {
        return mImpl->setBoost(type, durationMs);
    }

    ScopedAStatus isBoostSupported(Boost type, bool* _aidl_return) override {
        return mImpl->isBoostSupported(type, _aidl_return);
    }

  private:
    std::shared_ptr<IPower> mImpl;
};

std::shared_ptr<Power> gPower;
SpAIBinder gBinder;

}  // namespace

extern "C" binder_status_t AServiceManager_addService(AIBinder* binder, const char* instance) {
    static const auto addService = reinterpret_cast<binder_status_t (*)(AIBinder*, const char*)>(
            dlsym(RTLD_NEXT, "AServiceManager_addService"));

    if (std::string(IPower::descriptor) + "/default" == instance) {
        AIBinder_incStrong(binder);
        std::shared_ptr<IPower> impl = IPower::fromBinder(SpAIBinder(binder));
        if (impl != nullptr) {
            gPower = SharedRefBase::make<Power>(impl);
            gBinder = gPower->asBinder();
            binder = gBinder.get();
        } else {
            LOG(ERROR) << "Failed to wrap " << instance;
        }
    }

    return addService(binder, instance);
}
