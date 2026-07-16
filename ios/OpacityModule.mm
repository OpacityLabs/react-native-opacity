#import "OpacityModule.h"
#import "OpacityObjCWrapper.h"

@implementation OpacityModule
RCT_EXPORT_MODULE()

OpacityEnvironment opacityEnvironmentFromDouble(double value) {
  if (value == 1.0) {
    return Local;
  } else if (value == 2.0) {
    return Sandbox;
  } else if (value == 3.0) {
    return Staging;
  } else {
    return Production;
  }
}

RCT_EXPORT_METHOD(
    init : (NSString *)apiKey dryRun : (BOOL)dryRun environment : (double)
        environment shouldShowErrorsInWebView : (BOOL)
            shouldShowErrorsInWebView resolve : (RCTPromiseResolveBlock)
                resolve reject : (RCTPromiseRejectBlock)reject) {

  OpacityEnvironment env = opacityEnvironmentFromDouble(environment);
  NSError *error;

  int status = [OpacityObjCWrapper initialize:apiKey
                                    andDryRun:dryRun
                               andEnvironment:env
                 andShouldShowErrorsInWebview:shouldShowErrorsInWebView
                                     andError:&error];

  if (status == 0) {
    resolve(nil);
  } else {
    reject([NSString stringWithFormat:@"%d", status],
           [error localizedDescription], NULL);
  }
}

// Argument labels must match the selector codegen emits from the TurboModule
// spec -- `@selector(initializeOpenTelemetry:grafanaInstanceId:grafanaApiToken:
// resolve:reject:)`. An `and`-prefixed label builds a different selector, and
// the mismatch only surfaces as an unrecognized-selector crash when JS actually
// calls the method.
RCT_EXPORT_METHOD(initializeOpenTelemetry : (NSString *)
                       openTelemetryEndpoint grafanaInstanceId : (NSString *)
                           grafanaInstanceId grafanaApiToken : (NSString *)
                               grafanaApiToken resolve : (RCTPromiseResolveBlock)
                                   resolve reject : (RCTPromiseRejectBlock)reject) {

  NSError *error = nil;

  int status = [OpacityObjCWrapper initializeOpenTelemetry:openTelemetryEndpoint
                                        andGrafanaInstanceId:grafanaInstanceId
                                          andGrafanaApiToken:grafanaApiToken
                                                    andError:&error];

  if (status == 0) {
    resolve(nil);
  } else {
    NSString *message = error ? [error localizedDescription] : @"Unknown error";
    reject([NSString stringWithFormat:@"%d", status], message, error);
  }
}

RCT_EXPORT_METHOD(getInternal : (NSString *)name params : (NSDictionary *)
                      params resolve : (RCTPromiseResolveBlock)
                          resolve reject : (RCTPromiseRejectBlock)reject) {
  [OpacityObjCWrapper get:name
                andParams:params
               completion:^(NSDictionary *res, NSError *error) {
                 if (error) {
                   reject([error domain], [error localizedDescription], error);
                 } else {
                   resolve(res);
                 }
               }];
}

RCT_EXPORT_METHOD(getInternalWithContext : (NSString *)name traceparent : (
    NSString *)traceparent params : (NSDictionary *)params tracestate : (
    NSString *)tracestate resolve : (RCTPromiseResolveBlock)
                      resolve reject : (RCTPromiseRejectBlock)reject) {
  [OpacityObjCWrapper getWithContext:name
                           andParams:params
                      andTraceparent:traceparent
                       andTracestate:tracestate
                          completion:^(NSDictionary *res, NSError *error) {
                            if (error) {
                              reject([error domain],
                                     [error localizedDescription], error);
                            } else {
                              resolve(res);
                            }
                          }];
}

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params {
  return std::make_shared<facebook::react::NativeOpacitySpecJSI>(params);
}

@end
