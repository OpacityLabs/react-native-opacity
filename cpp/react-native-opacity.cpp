#include "react-native-opacity.h"
#include "macros.h"
#include "opacity.h"
#include <thread>

namespace facebook::react {

// constructor
NativeOpacityTurboModule::NativeOpacityTurboModule(
    std::shared_ptr<CallInvoker> jsinvoker)
    : NativeOpacityCxxSpec(std::move(jsinvoker)) {}

void NativeOpacityTurboModule::init(jsi::Runtime &rt, std::string api_key,
                                    bool dry_run) {
  opacity_core::init(api_key.c_str(), dry_run);
}

jsi::Value NativeOpacityTurboModule::getRiderProfile(jsi::Runtime &rt) {

  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");

  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    std::thread([resolve = std::move(resolve), jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_rider_profile(&json, &proof, &err);

      if (status != opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto js_json = jsi::String::createFromUtf8(rt, json);
          resolve->asObject(rt).asFunction(rt).call(rt, js_json);
        });
        return;
      }
    }).detach();
    return {};
  }));
}

} // namespace facebook::react
