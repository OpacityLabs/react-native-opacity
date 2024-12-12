#include "NativeOpacityTurboModule.h"
#include "macros.h"
#include "opacity.h"
#include <thread>

namespace facebook::react {

NativeOpacityTurboModule::NativeOpacityTurboModule(
    std::shared_ptr<CallInvoker> jsinvoker)
    : NativeOpacityCxxSpec(std::move(jsinvoker)) {}

jsi::Value NativeOpacityTurboModule::init(jsi::Runtime &rt, std::string api_key,
                                          bool dry_run, double environment) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, api_key, dry_run,
                 environment]() {
      int environment_int = static_cast<int>(environment);
      int status =
          opacity_core::init(api_key.c_str(), dry_run, environment_int);
      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve] {
          resolve->asObject(rt).asFunction(rt).call(rt, {});
        });
      } else {
        auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
        auto error = errorCtr.callAsConstructor(
            rt,
            jsi::String::createFromUtf8(rt, "Failed to initialize the SDK"));
        reject->asObject(rt).asFunction(rt).call(rt, error);
      }
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getUberRiderProfile(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_uber_rider_profile(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
}

jsi::Value
NativeOpacityTurboModule::getUberRiderTripHistory(jsi::Runtime &rt,
                                                  jsi::String cursor) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  auto cursor_str = cursor.utf8(rt);

  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, &cursor_str]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_uber_rider_trip_history(
          cursor_str.c_str(), &json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getUberRiderTrip(jsi::Runtime &rt,
                                                      jsi::String id) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  auto id_str = id.utf8(rt);
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, &id_str]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_uber_rider_trip(id_str.c_str(), &json,
                                                     &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
};

jsi::Value NativeOpacityTurboModule::getUberDriverProfile(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_uber_driver_profile(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
};

jsi::Value NativeOpacityTurboModule::getUberDriverTrips(jsi::Runtime &rt,
                                                        jsi::String startDate,
                                                        jsi::String endDate,
                                                        jsi::String cursor) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  auto startDate_str = startDate.utf8(rt);
  auto endDate_str = endDate.utf8(rt);
  auto cursor_str = cursor.utf8(rt);
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, &startDate_str,
                 &endDate_str, &cursor_str]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_uber_driver_trips(
          startDate_str.c_str(), endDate_str.c_str(), cursor_str.c_str(), &json,
          &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getUberFareEstimate(
    jsi::Runtime &rt, double pickupLatitude, double pickupLongitude,
    double dropoffLatitude, double dropoffLongitude) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
    return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, pickupLatitude,
                 pickupLongitude, dropoffLatitude, dropoffLongitude]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_uber_fare_estimate(
          pickupLatitude, pickupLongitude, dropoffLatitude, dropoffLongitude,
          &json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
                                                                      }));
}

jsi::Value NativeOpacityTurboModule::getRedditAccount(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_reddit_account(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
};

jsi::Value
NativeOpacityTurboModule::getRedditFollowedSubreddits(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status =
          opacity_core::get_reddit_followed_subreddits(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
};

jsi::Value NativeOpacityTurboModule::getRedditComments(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_reddit_comments(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
};

jsi::Value NativeOpacityTurboModule::getRedditPosts(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_reddit_posts(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
};

jsi::Value NativeOpacityTurboModule::getZabkaAccount(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_zabka_account(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
};

jsi::Value NativeOpacityTurboModule::getZabkaPoints(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_zabka_points(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
      return;
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getCartaProfile(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_carta_profile(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getCartaOrganizations(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
    return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_carta_organizations(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}
jsi::Value NativeOpacityTurboModule::getCartaPortfolioInvestments(
    jsi::Runtime &rt, jsi::String firm_id, jsi::String account_id) {
  auto firm_id_str = firm_id.utf8(rt);
  auto account_id_str = account_id.utf8(rt);
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
    return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);

    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, firm_id_str,
                 account_id_str]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_carta_portfolio_investments(
          firm_id_str.c_str(), account_id_str.c_str(), &json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
                                                                      }));
}
jsi::Value
NativeOpacityTurboModule::getCartaHoldingsCompanies(jsi::Runtime &rt,
                                                    jsi::String account_id) {
  auto account_id_str = account_id.utf8(rt);
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
    return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt,
                 account_id_str]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_carta_holdings_companies(
          account_id_str.c_str(), &json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getCartaCorporationSecurities(
    jsi::Runtime &rt, jsi::String account_id, jsi::String corporation_id) {
  auto account_id_str = account_id.utf8(rt);
  auto corporation_id_str = corporation_id.utf8(rt);
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
    return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, account_id_str,
                 corporation_id_str]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_carta_corporation_securities(
          account_id_str.c_str(), corporation_id_str.c_str(), &json, &proof,
          &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getGithubProfile(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_github_profile(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getInstagramProfile(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_instagram_profile(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getInstagramLikes(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_instagram_likes(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getInstagramComments(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_instagram_comments(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getInstagramSavedPosts(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_instagram_saved_posts(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();
    return {};
  }));
}

// Gusto
jsi::Value NativeOpacityTurboModule::getGustoMembersTable(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_gusto_members_table(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();

    return {};
  }));
}

jsi::Value NativeOpacityTurboModule::getGustoPayrollAdminId(jsi::Runtime &rt) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
        return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_gusto_payroll_admin_id(&json, &proof, &err);

      if (status == opacity_core::OPACITY_OK) {
        jsInvoker->invokeAsync([&rt, resolve, json] {
          auto data = jsi::String::createFromUtf8(rt, json);
          auto proof = jsi::String::createFromUtf8(rt, "");
          auto res = jsi::Object(rt);
          res.setProperty(rt, "data", data);
          res.setProperty(rt, "proof", proof);
          resolve->asObject(rt).asFunction(rt).call(rt, res);
        });
      } else {
        jsInvoker->invokeAsync([&rt, reject, err] {
          auto errorCtr = rt.global().getPropertyAsFunction(rt, "Error");
          auto error = errorCtr.callAsConstructor(
              rt, jsi::String::createFromUtf8(rt, err));
          reject->asObject(rt).asFunction(rt).call(rt, error);
        });
      };
    }).detach();

    return {};
  }));
}
} // namespace facebook::react
