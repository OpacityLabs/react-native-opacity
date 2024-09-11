
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
  jsi::Value getRiderTripHistory(jsi::Runtime &rt, double limit, double offset);
  jsi::Value getRiderTrip(jsi::Runtime &rt, jsi::String id);
  jsi::Value getDriverProfile(jsi::Runtime &rt);
  jsi::Value getDriverTrips(jsi::Runtime &rt, jsi::String startDate, jsi::String endDate, jsi::String cursor);
  jsi::Value getRedditAccount(jsi::Runtime &rt);
  jsi::Value getRedditFollowedSubreddits(jsi::Runtime &rt);
  jsi::Value getRedditCommets(jsi::Runtime &rt);
  jsi::Value getRedditPosts(jsi::Runtime &rt);
  jsi::Value zabkaGetAccount(jsi::Runtime &rt);
  jsi::Value zabkaGetPoints(jsi::Runtime &rt);
};
} // namespace facebook::react
