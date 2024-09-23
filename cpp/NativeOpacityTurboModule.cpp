#include "NativeOpacityTurboModule.h"
#include "macros.h"
#include "opacity.h"
#include <thread>

namespace facebook::react {

NativeOpacityTurboModule::NativeOpacityTurboModule(
    std::shared_ptr<CallInvoker> jsinvoker)
    : NativeOpacityCxxSpec(std::move(jsinvoker)) {}

jsi::Value NativeOpacityTurboModule::init(jsi::Runtime &rt, std::string api_key,
                                          bool dry_run) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");
  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, api_key,
                 dry_run]() {
      int status = opacity_core::init(api_key.c_str(), dry_run);
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

jsi::Value NativeOpacityTurboModule::getUberRiderTripHistory(jsi::Runtime &rt,
                                                             double limit,
                                                             double offset) {
  jsi::Function promiseConstructor =
      rt.global().getPropertyAsFunction(rt, "Promise");

  return promiseConstructor.callAsConstructor(rt, HOSTFN("promise") {
    auto resolve = std::make_shared<jsi::Value>(rt, args[0]);
    auto reject = std::make_shared<jsi::Value>(rt, args[1]);
    std::thread([resolve, reject, jsInvoker = jsInvoker_, &rt, &limit,
                 &offset]() {
      char *json;
      char *proof;
      char *err;

      int status = opacity_core::get_uber_rider_trip_history(
          limit, offset, &json, &proof, &err);

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

jsi::Value NativeOpacityTurboModule::getRedditCommets(jsi::Runtime &rt) {
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
};

} // namespace facebook::react
