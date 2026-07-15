#import "helper_functions.h"
#import "ModalWebViewController.h"
#import "Reachability.h"
#import <CoreLocation/CoreLocation.h>
#import <CoreTelephony/CTCarrier.h>
#import <CoreTelephony/CTTelephonyNetworkInfo.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <string>
#import <sys/utsname.h>

ModalWebViewController *modalWebVC;
NSMutableURLRequest *request;
UINavigationController *navController;
NSString *userAgent;
NSMutableArray<NSHTTPCookie *> *pendingCookies;

// Rust reads the const char* returned across the FFI boundary synchronously,
// immediately after the call, and never frees it. These statics keep the
// backing NSString retained past the function's return so the UTF8String
// pointer isn't dangling by the time Rust reads it (ARC would otherwise
// deallocate a purely local NSString as soon as it goes out of scope).
static NSString *evalJsResultHolder;
static NSString *cookiesForDomainJsonHolder;
static NSString *cookiesForCurrentUrlJsonHolder;
static NSString *deviceCodenameHolder;
static NSString *carrierNameHolder;
static NSString *carrierMccHolder;
static NSString *carrierMncHolder;
static NSString *deviceModelHolder;
static NSString *osNameHolder;
static NSString *osVersionHolder;
static NSString *deviceLocaleHolder;

UIViewController *topMostViewController() {
  // Fetch the key window's root view controller
  UIWindow *keyWindow = [UIApplication sharedApplication].windows.firstObject;
  UIViewController *topController = keyWindow.rootViewController;

  // Traverse the presented view controllers to find the topmost one
  while (topController.presentedViewController) {
    topController = topController.presentedViewController;
  }

  return topController;
}

void ios_prepare_request(const char *url) {
  NSString *urlString = [NSString stringWithUTF8String:url];
  request =
      [NSMutableURLRequest requestWithURL:[NSURL URLWithString:urlString]];
  pendingCookies = [NSMutableArray array];
}

void ios_set_cookie(const char *url_c, const char *value_c) {
  NSString *urlString = [NSString stringWithUTF8String:url_c];
  NSString *cookieString = [NSString stringWithUTF8String:value_c];
  NSURL *url = [NSURL URLWithString:urlString];
  if (!url || !cookieString) {
    return;
  }
  NSArray<NSHTTPCookie *> *cookies =
      [NSHTTPCookie cookiesWithResponseHeaderFields:@{@"Set-Cookie": cookieString}
                                             forURL:url];
  if (!pendingCookies) {
    pendingCookies = [NSMutableArray array];
  }
  [pendingCookies addObjectsFromArray:cookies];
}

void ios_set_request_header(const char *key, const char *value) {
  NSString *nsKey = [NSString stringWithUTF8String:key];
  if ([nsKey caseInsensitiveCompare:@"User-Agent"] == NSOrderedSame) {
    userAgent = [NSString stringWithUTF8String:value];
  }
  NSString *nsValue = [NSString stringWithUTF8String:value];
  [request setValue:nsValue forHTTPHeaderField:nsKey];
}

void ios_close_webview() {
  dispatch_async(dispatch_get_main_queue(), ^{
    userAgent = nil;
    [navController dismissViewControllerAnimated:YES completion:nil];
  });
}

void ios_present_webview(bool intercept_requests) {
  dispatch_async(dispatch_get_main_queue(), ^{
    if (modalWebVC != nil) {
      NSLog(@"Warning: Previous modal web view controller has not been "
            @"dismissed");
    }

    NSArray<NSHTTPCookie *> *cookiesToInject = [pendingCookies copy];
    pendingCookies = [NSMutableArray array];

    modalWebVC = [[ModalWebViewController alloc] initWithRequest:request
                                                       userAgent:userAgent
                                               interceptRequests:intercept_requests
                                                  initialCookies:cookiesToInject];

    // Set an on dismiss callback
    modalWebVC.onDismissCallback = ^{
      modalWebVC = nil;
      navController = nil;
    };

    navController =
        [[UINavigationController alloc] initWithRootViewController:modalWebVC];

    UIViewController *topController = topMostViewController();

    [topController presentViewController:navController
                                animated:YES
                              completion:nil];
  });
}

void ios_webview_change_url(const char *url) {
  if (url == nullptr) {
    NSLog(@"URL is nullptr");
    return;
  }

  NSString *urlString = [NSString stringWithUTF8String:url];
  if (urlString == nil) {
    NSLog(@"Failed to create NSString from URL");
    return;
  }
  NSString *capturedUrlString = [urlString copy];

  dispatch_async(dispatch_get_main_queue(), ^{
    if (modalWebVC == nil) {
      NSLog(@"Warning: Browser is not open");
      return;
    }

    NSURL *nsUrl = [NSURL URLWithString:capturedUrlString];
    if (nsUrl == nil) {
      return;
    }

    NSMutableURLRequest *currentRequest = request;
    NSMutableURLRequest *newRequest = currentRequest != nil
                                          ? [currentRequest mutableCopy]
                                          : nil;
    if (newRequest == nil) {
      newRequest = [NSMutableURLRequest requestWithURL:nsUrl];
    }
    [newRequest setURL:nsUrl];
    if (newRequest == nil) {
      return;
    }

    request = newRequest;
    [modalWebVC openRequest:newRequest];
  });
}

const char *ios_eval_js(const char *js, double timeout_in_seconds) {
  if (modalWebVC == nil) {
    return "{\"error\":\"browser not open\"}";
  }

  NSString *jsString = [NSString stringWithUTF8String:js];
  if (jsString == nil) {
    return "{\"error\":\"invalid js string\"}";
  }

  NSString *result = [modalWebVC evalJs:jsString timeout:timeout_in_seconds];
  if (result == nil) {
    return "{\"error\":\"nil result\"}";
  }

  evalJsResultHolder = result;
  return [evalJsResultHolder UTF8String];
}

const char *ios_get_browser_cookies_for_domain(const char *domain) {
  if (modalWebVC == nil) {
    return nullptr;
  }
  NSString *domainString = [NSString stringWithUTF8String:domain];
  NSDictionary *cookies = [modalWebVC getBrowserCookiesForDomain:domainString];

  // Convert the dictionary to JSON string
  NSError *error;
  NSData *jsonData = [NSJSONSerialization dataWithJSONObject:cookies
                                                     options:0
                                                       error:&error];
  if (!jsonData) {
    NSLog(@"Error serializing cookies to JSON: %@", error);
    return "";
  }

  // Convert JSON data to C string and return
  cookiesForDomainJsonHolder = [[NSString alloc] initWithData:jsonData
                                                      encoding:NSUTF8StringEncoding];
  return [cookiesForDomainJsonHolder UTF8String];
}

const char *ios_get_browser_cookies_for_current_url() {
  if (modalWebVC == nil) {
    return nullptr;
  }

  NSDictionary *cookies = [modalWebVC getBrowserCookiesForCurrentUrl];

  // Convert the dictionary to JSON string
  NSError *error;
  NSData *jsonData = [NSJSONSerialization dataWithJSONObject:cookies
                                                     options:0
                                                       error:&error];
  if (!jsonData) {
    NSLog(@"Error serializing cookies to JSON: %@", error);
    return "";
  }

  // Convert JSON data to C string and return
  cookiesForCurrentUrlJsonHolder = [[NSString alloc] initWithData:jsonData
                                                          encoding:NSUTF8StringEncoding];
  return [cookiesForCurrentUrlJsonHolder UTF8String];
}

double get_battery_level() {
  UIDevice *device = [UIDevice currentDevice];
  device.batteryMonitoringEnabled = YES;
  double batteryLevel = static_cast<double>(device.batteryLevel);
  return batteryLevel;
}

const char *get_battery_status() {
  UIDevice *device = [UIDevice currentDevice];
  device.batteryMonitoringEnabled = YES;
  UIDeviceBatteryState batteryState = device.batteryState;

  const char *status = "Unknown";
  switch (batteryState) {
  case UIDeviceBatteryStateUnknown:
    status = "Unknown";
    break;
  case UIDeviceBatteryStateUnplugged:
    status = "Unplugged";
    break;
  case UIDeviceBatteryStateCharging:
    status = "Charging";
    break;
  case UIDeviceBatteryStateFull:
    status = "Full";
    break;
  }

  return status;
}

const char *get_carrier_name() {
  CTTelephonyNetworkInfo *networkInfo = [[CTTelephonyNetworkInfo alloc] init];
  NSDictionary<NSString *, CTCarrier *> *carriers =
      [networkInfo serviceSubscriberCellularProviders];
  CTCarrier *carrier = carriers.allValues.firstObject;
  carrierNameHolder = [carrier carrierName];
  return [carrierNameHolder UTF8String];
}

const char *get_carrier_mcc() {
  CTTelephonyNetworkInfo *networkInfo = [[CTTelephonyNetworkInfo alloc] init];
  NSDictionary<NSString *, CTCarrier *> *carriers =
      [networkInfo serviceSubscriberCellularProviders];
  CTCarrier *carrier = carriers.allValues.firstObject;
  carrierMccHolder = [carrier mobileCountryCode];
  return [carrierMccHolder UTF8String];
}

const char *get_carrier_mnc() {
  CTTelephonyNetworkInfo *networkInfo = [[CTTelephonyNetworkInfo alloc] init];
  NSDictionary<NSString *, CTCarrier *> *carriers =
      [networkInfo serviceSubscriberCellularProviders];
  CTCarrier *carrier = carriers.allValues.firstObject;
  carrierMncHolder = [carrier mobileNetworkCode];
  return [carrierMncHolder UTF8String];
}

double get_course() {
  CLLocationManager *locationManager = [[CLLocationManager alloc] init];
  CLLocationDirection course = locationManager.heading.trueHeading;
  return course;
}

const char *get_cpu_abi() {
  const char *cpuAbi = "arm64-v8a";
  return cpuAbi;
}

double get_altitude() {
  CLLocationManager *locationManager = [[CLLocationManager alloc] init];

  CLLocation *location = locationManager.location;
  double altitude = location.altitude;
  return altitude;
}

double get_latitude() {
  CLLocationManager *locationManager = [[CLLocationManager alloc] init];

  CLLocation *location = locationManager.location;
  double latitude = location.coordinate.latitude;
  return latitude;
}

double get_longitude() {
  CLLocationManager *locationManager = [[CLLocationManager alloc] init];

  CLLocation *location = locationManager.location;
  double longitude = location.coordinate.longitude;
  return longitude;
}

const char *get_device_model() {
  struct utsname systemInfo;
  uname(&systemInfo);
  deviceModelHolder = [NSString stringWithCString:systemInfo.machine
                                          encoding:NSUTF8StringEncoding];
  return [deviceModelHolder UTF8String];
}

const char *get_os_name() {
  osNameHolder = [UIDevice currentDevice].systemName;
  return [osNameHolder UTF8String];
}

const char *get_os_version() {
  osVersionHolder = [UIDevice currentDevice].systemVersion;
  return [osVersionHolder UTF8String];
}

const char *get_device_locale() {
  deviceLocaleHolder = [[NSLocale currentLocale] localeIdentifier];
  const char *locale = [deviceLocaleHolder UTF8String];
  return locale;
}

int get_screen_width() {
  CGRect screenBounds = [[UIScreen mainScreen] bounds];
  CGFloat scale = [[UIScreen mainScreen] scale];
  return (int)(screenBounds.size.width * scale);
}

int get_screen_height() {
  CGRect screenBounds = [[UIScreen mainScreen] bounds];
  CGFloat scale = [[UIScreen mainScreen] scale];
  return (int)(screenBounds.size.height * scale);
}

float get_screen_density() { return [[UIScreen mainScreen] scale]; }

int get_screen_dpi() {
  CGFloat scale = [[UIScreen mainScreen] scale];
  return (int)(163 * scale);
}

const char *get_device_cpu() {
#if defined(__aarch64__) || defined(__arm64__)
  static const char *cpu_arch = "arm64";
#elif defined(__arm__)
  static const char *cpu_arch = "arm";
#elif defined(__x86_64__)
  static const char *cpu_arch = "x86_64";
#elif defined(__i386__)
  static const char *cpu_arch = "x86";
#else
  static const char *cpu_arch = "unknown";
#endif
  return cpu_arch;
}

const char *get_device_codename() {
  struct utsname systemInfo;
  uname(&systemInfo);

  deviceCodenameHolder = [NSString stringWithCString:systemInfo.machine
                                             encoding:NSUTF8StringEncoding];
  return [deviceCodenameHolder UTF8String];
}

bool is_emulator() {
  bool isSimulator = false;

#if TARGET_IPHONE_SIMULATOR
  isSimulator = true;
#endif

  return isSimulator;
}

double get_horizontal_accuracy() {
  CLLocationManager *locationManager = [[CLLocationManager alloc] init];

  CLLocation *location = locationManager.location;
  double horizontalAccuracy = location.horizontalAccuracy;
  return horizontalAccuracy;
}

double get_vertical_accuracy() {
  CLLocationManager *locationManager = [[CLLocationManager alloc] init];

  CLLocation *location = locationManager.location;
  double verticalAccuracy = location.verticalAccuracy;
  return verticalAccuracy;
}

const char *get_ip_address() {
  struct ifaddrs *interfaces = NULL;
  struct ifaddrs *temp_addr = NULL;
  const char *ipAddress = "Unknown";

  // Retrieve the current interfaces - returns 0 on success
  if (getifaddrs(&interfaces) == 0) {
    // Loop through linked list of interfaces
    temp_addr = interfaces;
    while (temp_addr != NULL) {
      if (temp_addr->ifa_addr == NULL) {
        temp_addr = temp_addr->ifa_next;
        continue;
      }
      // Check if the interface is IPv4 or IPv6
      if (temp_addr->ifa_addr->sa_family == AF_INET) {
        if (strcmp(temp_addr->ifa_name, "lo0") != 0) {
          // Convert the IP address from a binary to a string representation
          ipAddress =
              inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr);
          break;
        }
      }
      temp_addr = temp_addr->ifa_next;
    }
  }

  // Free memory
  freeifaddrs(interfaces);

  return ipAddress;
}

bool is_location_services_enabled() {
  return CLLocationManager.locationServicesEnabled;
}

bool is_wifi_connected() {
  Reachability *reachability = [Reachability reachabilityForInternetConnection];
  NetworkStatus netStatus = [reachability currentReachabilityStatus];
  return netStatus == ReachableViaWiFi;
}

bool is_rooted() { return false; }

bool is_app_foregrounded() {
  UIApplicationState state =
      [[UIApplication sharedApplication] applicationState];
  return state == UIApplicationStateActive;
}
