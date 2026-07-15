#import "OpacityObjCWrapper.h"
#import "helper_functions.h"
#import "sdk.h"

NSError *parseOpacityError(NSString *jsonString) {
  if (jsonString == nil || [jsonString length] == 0) {
    return [NSError
        errorWithDomain:@"OpacitySDKUnknownError"
                   code:1001
               userInfo:@{NSLocalizedDescriptionKey : @"Empty error message"}];
  }

  NSData *data = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
  if (data == nil) {
    return [NSError errorWithDomain:@"OpacitySDKUnknownError"
                               code:1001
                           userInfo:@{NSLocalizedDescriptionKey : jsonString}];
  }

  NSError *parsingError = nil;
  NSDictionary *json = [NSJSONSerialization JSONObjectWithData:data
                                                       options:0
                                                         error:&parsingError];
  if (parsingError != nil || json == nil) {
    return [NSError errorWithDomain:@"OpacitySDKUnknownError"
                               code:1001
                           userInfo:@{NSLocalizedDescriptionKey : jsonString}];
  }

  NSString *code = json[@"code"];
  NSString *desc = json[@"description"];
  // We are not using this field for now
  // NSString *translation = json[@"translation_key"];

  return [NSError errorWithDomain:code
                             code:1001
                         userInfo:@{NSLocalizedDescriptionKey : desc}];
}

@implementation OpacityObjCWrapper

+ (int)initialize:(NSString *)api_key
                       andDryRun:(BOOL)dry_run
                  andEnvironment:(OpacityEnvironment)environment
    andShouldShowErrorsInWebview:(BOOL)should_show_errors_in_webview
                        andError:(NSError *__autoreleasing *)error {

  NSString *path = [[[NSBundle mainBundle] privateFrameworksPath]
      stringByAppendingPathComponent:@"sdk.framework"];
  NSBundle *bundle = [NSBundle bundleWithPath:path];
  if (![bundle isLoaded]) {
    BOOL success = [bundle load];
    if (!success) {
      NSString *errorMessage = @"Failed to load framework";
      if (error) {
        *error = [NSError
            errorWithDomain:@"OpacitySDKDylibError"
                       code:1002
                   userInfo:@{NSLocalizedDescriptionKey : errorMessage}];
      }
      return -1;
    }
  }

  opacity_core::register_ios_callbacks(
      ios_prepare_request, ios_set_request_header, ios_set_cookie,
      ios_present_webview, ios_close_webview,
      ios_get_browser_cookies_for_current_url,
      ios_get_browser_cookies_for_domain, get_ip_address, get_battery_level,
      get_battery_status, get_carrier_name, get_carrier_mcc, get_carrier_mnc,
      get_course, get_cpu_abi, get_altitude, get_latitude, get_longitude,
      get_device_model, get_os_name, get_os_version, is_emulator,
      get_horizontal_accuracy, get_vertical_accuracy,
      is_location_services_enabled, is_wifi_connected, is_rooted,
      is_app_foregrounded, get_device_locale, get_screen_width,
      get_screen_height, get_screen_density, get_screen_dpi, get_device_cpu,
      get_device_codename, ios_webview_change_url, ios_eval_js);

  char *err;
  int status = opacity_core::opacity_init([api_key UTF8String], dry_run,
                                          static_cast<int>(environment),
                                          should_show_errors_in_webview, &err);
  if (status != opacity_core::OPACITY_OK && err != nullptr) {
    NSString *errorMessage = [NSString stringWithUTF8String:err];
    if (error) {
      *error = parseOpacityError(errorMessage);
    }
    opacity_core::opacity_free_string(err);
  }

  return status;
}

+ (int)initializeOpenTelemetry:(NSString *)openTelemetryEndpoint
          andGrafanaInstanceId:(NSString *)grafanaInstanceId
            andGrafanaApiToken:(NSString *)grafanaApiToken
                      andError:(NSError *__autoreleasing *)error {
  char *err;
  int status = opacity_core::opacity_initialize_open_telemetry(
      [openTelemetryEndpoint UTF8String], [grafanaInstanceId UTF8String],
      [grafanaApiToken UTF8String], &err);

  if (status != opacity_core::OPACITY_OK && err != nullptr) {
    NSString *errorMessage = [NSString stringWithUTF8String:err];
    if (error) {
      *error = parseOpacityError(errorMessage);
    }
    opacity_core::opacity_free_string(err);
  }

  return status;
}

+ (void)get:(NSString *)name
     andParams:(NSDictionary *)params
    completion:(void (^)(NSDictionary *res, NSError *error))completion {
  NSThread *thread = [[NSThread alloc] initWithBlock:^{
    char *res, *err;
    NSError *error = nil;
    NSString *paramsJSON = nil;

    if (params && [params count] > 0) {
      NSData *jsonData = [NSJSONSerialization dataWithJSONObject:params
                                                         options:0
                                                           error:&error];
      if (!jsonData) {
        if (!error) {
          error = [NSError
              errorWithDomain:@"OpacitySDKError"
                         code:1003
                     userInfo:@{
                       NSLocalizedDescriptionKey : @"Could not parse params"
                     }];
        }

        dispatch_async(dispatch_get_main_queue(), ^{
          completion(nil, error);
        });

        return;
      }

      paramsJSON = [[NSString alloc] initWithData:jsonData
                                         encoding:NSUTF8StringEncoding];
    }

    int status = opacity_core::opacity_get(
        [name UTF8String], paramsJSON ? [paramsJSON UTF8String] : nullptr, &res,
        &err);

    if (status != opacity_core::OPACITY_OK) {
      NSString *error_str = [NSString stringWithUTF8String:err];
      opacity_core::opacity_free_string(err);

      NSError *opacity_error = parseOpacityError(error_str);
      dispatch_async(dispatch_get_main_queue(), ^{
        completion(nil, opacity_error);
      });

      return;
    }

    NSString *final_res = [NSString stringWithUTF8String:res];
    opacity_core::opacity_free_string(res);
    NSData *data = [final_res dataUsingEncoding:NSUTF8StringEncoding];
    NSError *jsonError;
    NSDictionary *jsonDict =
        [NSJSONSerialization JSONObjectWithData:data
                                        options:0
                                          error:&jsonError];
    if (jsonError) {
      dispatch_async(dispatch_get_main_queue(), ^{
        completion(nil, jsonError);
      });
      return;
    }

    dispatch_async(dispatch_get_main_queue(), ^{
      completion(jsonDict, nil);
    });
  }];
  [thread setStackSize:2 * 1024 * 1024]; // 2MB stack size
  [thread start];
}

+ (NSString *)getApiVersion {
  const char *res = opacity_core::get_api_version();
  NSString *final_res = [NSString stringWithUTF8String:res];
  return final_res;
}

@end
