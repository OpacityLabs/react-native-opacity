
#if __has_include(<React-Codegen/RNOpacitySpecJSI.h>)
#include <React-Codegen/RNOpacitySpecJSI.h>
#elif __has_include("RNOpacitySpecJSI.h")
#include "RNOpacitySpecJSI.h"
#endif

#include <ReactCommon/CallInvoker.h>
#include <jsi/jsi.h>

namespace facebook::react {
class NativeOpacityTurboModule
    : public NativeOpacityCxxSpec<NativeOpacityTurboModule> {
public:
  NativeOpacityTurboModule(std::shared_ptr<CallInvoker> jsInvoker);
  void init(jsi::Runtime &rt, std::string api_key, bool dry_run);
      jsi::Value getRiderProfile(jsi::Runtime &rt);
};
} // namespace facebook::react
