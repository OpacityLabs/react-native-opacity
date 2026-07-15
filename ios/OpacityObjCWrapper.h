#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, OpacityEnvironment) {
  Local = 1,
  Sandbox,
  Staging,
  Production
};

@interface OpacityObjCWrapper : NSObject

+ (int)initialize:(NSString *)apiKey
                       andDryRun:(BOOL)dryRun
                  andEnvironment:(OpacityEnvironment)environment
    andShouldShowErrorsInWebview:(BOOL)shouldShowErrorsInWebview
                        andError:(NSError **)error;

+ (int)initializeOpenTelemetry:(NSString *)openTelemetryEndpoint
          andGrafanaInstanceId:(NSString *)grafanaInstanceId
            andGrafanaApiToken:(NSString *)grafanaApiToken
                      andError:(NSError **)error;

+ (void)get:(NSString *)name
     andParams:(NSDictionary *)params
    completion:(void (^)(NSDictionary *res, NSError *error))completion;

+ (NSString *)getApiVersion;

@end
