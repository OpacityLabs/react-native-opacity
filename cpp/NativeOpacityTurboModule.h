
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
  // uber
  jsi::Value getUberRiderProfile(jsi::Runtime &rt);
  jsi::Value getUberRiderTripHistory(jsi::Runtime &rt, jsi::String cursor);
  jsi::Value getUberRiderTrip(jsi::Runtime &rt, jsi::String id);
  jsi::Value getUberDriverProfile(jsi::Runtime &rt);
  jsi::Value getUberDriverTrips(jsi::Runtime &rt, jsi::String startDate, jsi::String endDate, jsi::String cursor);
  jsi::Value getUberFareEstimate(jsi::Runtime &rt, double pickupLatitude, double pickupLongitude, double dropoffLatitude, double dropoffLongitude);
  // reddit
  jsi::Value getRedditAccount(jsi::Runtime &rt);
  jsi::Value getRedditFollowedSubreddits(jsi::Runtime &rt);
  jsi::Value getRedditComments(jsi::Runtime &rt);
  jsi::Value getRedditPosts(jsi::Runtime &rt);
  // zabka
  jsi::Value getZabkaAccount(jsi::Runtime &rt);
  jsi::Value getZabkaPoints(jsi::Runtime &rt);
  // carta
  jsi::Value getCartaProfile(jsi::Runtime &rt);
  jsi::Value getCartaOrganizations(jsi::Runtime &rt);
  jsi::Value getCartaPortfolioInvestments(jsi::Runtime &rt, jsi::String firm_id, jsi::String account_id);
  jsi::Value getCartaHoldingsCompanies(jsi::Runtime &rt, jsi::String account_id);
  jsi::Value getCartaCorporationSecurities(jsi::Runtime &rt, jsi::String account_id, jsi::String corporation_id);
};
} // namespace facebook::react
