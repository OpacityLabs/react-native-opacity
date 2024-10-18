#import "ModalWebViewController.h"
#import "OpacityIOSHelper.h"
#import "opacity.h"

@interface ModalWebViewController ()

@property(nonatomic, strong) WKWebView *webView;
@property(nonatomic, strong) NSMutableURLRequest *request;
@property(nonatomic, strong) WKWebsiteDataStore *websiteDataStore;
@property(nonatomic, strong) NSMutableDictionary *cookies;

@end

@implementation ModalWebViewController

- (void)openRequest:(NSMutableURLRequest *)request {
  _request = request;
  [self.webView loadRequest:_request];
}

- (instancetype)initWithRequest:(NSMutableURLRequest *)request {
  self = [super init];
  if (self) {
    _request = request;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.cookies = [NSMutableDictionary dictionary];

  // Configure the view's background color
  self.view.backgroundColor = [UIColor blackColor];

  // Create a WKWebViewConfiguration
  WKWebViewConfiguration *configuration = [[WKWebViewConfiguration alloc] init];

  // Create a WKProcessPool
  WKProcessPool *processPool = [[WKProcessPool alloc] init];
  configuration.processPool = processPool;

  // Create a WKWebsiteDataStore
  self.websiteDataStore = [WKWebsiteDataStore nonPersistentDataStore];
  configuration.websiteDataStore = self.websiteDataStore;
  //  [self.websiteDataStore.httpCookieStore addObserver:self];

  // Initialize and configure the WKWebView
  self.webView = [[WKWebView alloc] initWithFrame:self.view.bounds
                                    configuration:configuration];

  // Set the configuration to the WKWebView
  self.webView.allowsLinkPreview = true;
  self.webView.autoresizingMask =
      UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
  self.webView.navigationDelegate = self;

  // Add the WKWebView to the view hierarchy
  [self.view addSubview:self.webView];

  // Load the provided URL
  if (self.request) {
    [self.webView loadRequest:self.request];
  }

  // Add a Close button
  UIBarButtonItem *closeButton =
      [[UIBarButtonItem alloc] initWithTitle:@"Close"
                                       style:UIBarButtonItemStylePlain
                                      target:self
                                      action:@selector(close)];
  self.navigationItem.leftBarButtonItem = closeButton;
}

- (void)close {
  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
  [dict setObject:@"close" forKey:@"event"];
  [dict setObject:[NSString stringWithFormat:@"%f", [[NSDate date]
                                                        timeIntervalSince1970]]
           forKey:@"id"];
  NSError *error;
  NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict
                                                     options:0
                                                       error:&error];
  NSString *payload = [[NSString alloc] initWithData:jsonData
                                            encoding:NSUTF8StringEncoding];

  opacity_core::emit_webview_event([payload UTF8String]);

}

#pragma mark - WKNavigationDelegate Methods

// Called when the web view starts to load a page
//- (void)webView:(WKWebView *)webView
//    didStartProvisionalNavigation:(WKNavigation *)navigation {
//  NSLog(@"Started loading: %@", webView.URL.absoluteString);
//}
//
//// Called when the content starts arriving for a page
//- (void)webView:(WKWebView *)webView
//    didCommitNavigation:(WKNavigation *)navigation {
//  NSLog(@"Content started arriving: %@", webView.URL.absoluteString);
//}
//
//// Called when the page finishes loading
- (void)webView:(WKWebView *)webView
    didFinishNavigation:(WKNavigation *)navigation {
  NSURL *url = webView.URL;

  if (url) {
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setObject:url.absoluteString forKey:@"url"];
    [dict setObject:@"navigation" forKey:@"event"];
    [dict
        setObject:[NSString stringWithFormat:@"%f", [[NSDate date]
                                                        timeIntervalSince1970]]
           forKey:@"id"];

    [dict setObject:self.cookies forKey:@"cookies"];

    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict
                                                       options:0
                                                         error:&error];
    NSString *payload = [[NSString alloc] initWithData:jsonData
                                              encoding:NSUTF8StringEncoding];

    opacity_core::emit_webview_event([payload UTF8String]);
  }
}

// -(void)webView:(WKWebView *)webView
// didReceiveServerRedirectForProvisionalNavigation:(WKNavigation *)navigation {
//     NSLog(@"🟥 didReceiveServerRedirectForProvisionalNavigation");
//     NSURL *url = webView.URL;
//
//     if (url) {
//       NSMutableDictionary *dict = [NSMutableDictionary dictionary];
//       [dict setObject:url.absoluteString forKey:@"url"];
//       [dict setObject:@"navigation" forKey:@"event"];
//
//       dispatch_after(
//           dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)),
//           dispatch_get_main_queue(), ^{
//             WKHTTPCookieStore *cookieStore =
//                 self.webView.configuration.websiteDataStore.httpCookieStore;
//
//             [cookieStore getAllCookies:^(NSArray<NSHTTPCookie *> *cookies) {
//               NSMutableDictionary *cookies_dict =
//                   [NSMutableDictionary dictionary];
//
//               for (NSHTTPCookie *cookie in cookies) {
//                 [cookies_dict setObject:cookie.value forKey:cookie.name];
//               }
//
//               [dict setObject:cookies_dict forKey:@"cookies"];
//
//               // Convert the dictionary to a JSON string
//               NSError *error;
//               NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict
//                                                                  options:0
//                                                                    error:&error];
//               NSString *payload =
//                   [[NSString alloc] initWithData:jsonData
//                                         encoding:NSUTF8StringEncoding];
//
//               opacity_core::emit_webview_event([payload UTF8String]);
//             }];
//           });
//     }
// }

// Called if an error occurs during navigation
- (void)webView:(WKWebView *)webView
    didFailProvisionalNavigation:(WKNavigation *)navigation
                       withError:(NSError *)error {
    NSString* url = error.userInfo[NSURLErrorFailingURLStringErrorKey];
    if(url) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:url forKey:@"url"];
        [dict setObject:@"navigation" forKey:@"event"];
        [dict
            setObject:[NSString stringWithFormat:@"%f", [[NSDate date]
                                                            timeIntervalSince1970]]
               forKey:@"id"];

        [dict setObject:self.cookies forKey:@"cookies"];

        NSError *error;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict
                                                           options:0
                                                             error:&error];
        NSString *payload = [[NSString alloc] initWithData:jsonData
                                                  encoding:NSUTF8StringEncoding];

        opacity_core::emit_webview_event([payload UTF8String]);
    }
    
    NSLog(@"Failed to load: %@, Error: %@", error.userInfo[NSURLErrorFailingURLStringErrorKey],
        error.localizedDescription);
}

//// Called if an error occurs after the navigation has started
//- (void)webView:(WKWebView *)webView
//    didFailNavigation:(WKNavigation *)navigation
//            withError:(NSError *)error {
//  NSLog(@"Failed during navigation: %@, Error: %@",
//  webView.URL.absoluteString,
//        error.localizedDescription);
//}

- (void)URLSession:(NSURLSession *)session
                          task:(NSURLSessionTask *)task
    willPerformHTTPRedirection:(NSHTTPURLResponse *)response
                    newRequest:(NSURLRequest *)request
             completionHandler:
                 (void (^)(NSURLRequest *_Nullable))completionHandler {

  NSDictionary *headers = [response allHeaderFields];

  NSArray *cookies =
      [NSHTTPCookie cookiesWithResponseHeaderFields:headers
                                             forURL:[response URL]];

  for (NSHTTPCookie *cookie in cookies) {
    [self.cookies setObject:cookie.value forKey:cookie.name];
  }

  completionHandler(request);
}

// This method is called before the web view starts loading a page
- (void)webView:(WKWebView *)webView
    decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction
                    decisionHandler:
                        (void (^)(WKNavigationActionPolicy))decisionHandler {

  NSURLRequest *request = navigationAction.request;
  NSURLSession *session =
      [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration
                                                 defaultSessionConfiguration]
                                    delegate:self
                               delegateQueue:nil];
  NSURLSessionDataTask *task =
      [session dataTaskWithRequest:request
                 completionHandler:^(NSData *data, NSURLResponse *response,
                                     NSError *error) {
                   if (error) {
                     NSLog(@"Could not extract cookies from url: %@, with error: %@",
                           request.URL, error.localizedDescription);
                   }

                   if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
                     NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;

                     NSDictionary *diction = [resp allHeaderFields];

                     NSArray *cookies = [NSHTTPCookie
                         cookiesWithResponseHeaderFields:diction
                                                  forURL:[resp URL]];

                     for (NSHTTPCookie *cookie in cookies) {
                       [self.cookies setObject:cookie.value forKey:cookie.name];
                     }
                   }

                   decisionHandler(WKNavigationActionPolicyAllow);
                 }];
  [task resume];
}

@end
