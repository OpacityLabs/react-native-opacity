#import "OpacityModule.h"
#import "OpacityObjCWrapper.h"

@implementation OpacityModule
RCT_EXPORT_MODULE()

OpacityEnvironment opacityEnvironmentFromDouble(double value) {
  if (value == 0.0) {
    return Test;
  } else if (value == 1.0) {
    return Local;
  } else if (value == 2.0) {
    return Staging;
  } else {
    return Production;
  }
}

- (void)init:(NSString *)apiKey
         dryRun:(BOOL)dryRun
    environment:(double)environment
        resolve:(RCTPromiseResolveBlock)resolve
         reject:(RCTPromiseRejectBlock)reject {

  OpacityEnvironment env = opacityEnvironmentFromDouble(environment);

  int status = [OpacityObjCWrapper initialize:apiKey
                                    andDryRun:dryRun
                               andEnvironment:env];

  if (status == 0) {
    resolve(nil);
  } else {
    reject(@"SDKInitializationError",
           @"Could not initialize SDK, check the native logs", NULL);
  }
}

- (void)getInternal:(NSString *)name
             params:(NSDictionary *_Nullable)params
            resolve:(RCTPromiseResolveBlock)resolve
             reject:(RCTPromiseRejectBlock)reject {
  [OpacityObjCWrapper get:name
                andParams:params
               completion:^(NSDictionary *res, NSError *error) {
                 if (error) {
                   reject(@"FlowError", [error localizedDescription], NULL);
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
