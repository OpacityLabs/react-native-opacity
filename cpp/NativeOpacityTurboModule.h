
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
  jsi::Value init(jsi::Runtime &rt, std::string api_key, bool dry_run);
  jsi::Value getUberRiderProfile(jsi::Runtime &rt);
  jsi::Value getUberRiderTripHistory(jsi::Runtime &rt, jsi::String cursor);
  jsi::Value getUberRiderTrip(jsi::Runtime &rt, jsi::String id);
  jsi::Value getUberDriverProfile(jsi::Runtime &rt);
  jsi::Value getUberDriverTrips(jsi::Runtime &rt, jsi::String startDate, jsi::String endDate, jsi::String cursor);
  jsi::Value getUberFareEstimate(jsi::Runtime &rt, double pickupLatitude, double pickupLongitude, double dropoffLatitude, double dropoffLongitude);
  jsi::Value getRedditAccount(jsi::Runtime &rt);
  jsi::Value getRedditFollowedSubreddits(jsi::Runtime &rt);
  jsi::Value getRedditCommets(jsi::Runtime &rt);
  jsi::Value getRedditPosts(jsi::Runtime &rt);
  jsi::Value getZabkaAccount(jsi::Runtime &rt);
  jsi::Value getZabkaPoints(jsi::Runtime &rt);
};
} // namespace facebook::react
