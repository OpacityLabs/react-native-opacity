#import "ModalWebViewController.h"
#import "OpacityIOSHelper.h"
#import "opacity.h"

@interface ModalWebViewController ()

@property(nonatomic, strong) WKWebView *webView;
@property(nonatomic, strong) NSMutableURLRequest *request;
@property(nonatomic, strong) WKWebsiteDataStore *websiteDataStore;
@property(nonatomic, strong) NSMutableDictionary *cookies;
@property(nonatomic, strong) NSMutableArray<NSString *> *visitedUrls;

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
  self.visitedUrls = [NSMutableArray array];

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

  [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)addToVisitedUrls:(NSString *)urlToAdd {
  if (self.visitedUrls.count == 0 ||
      ![self.visitedUrls.lastObject isEqualToString:urlToAdd]) {
    [self.visitedUrls addObject:urlToAdd];
  }
}

- (void)resetVisitedUrls {
  [self.visitedUrls removeAllObjects];
}

#pragma mark - WKNavigationDelegate Methods

// Called when the web view starts to load a page
- (void)webView:(WKWebView *)webView
    didStartProvisionalNavigation:(WKNavigation *)navigation {
  if (webView.URL) {
    [self addToVisitedUrls:webView.URL.absoluteString];
  }
}

/// Called when the content starts arriving for a page
//- (void)webView:(WKWebView *)webView
//    didCommitNavigation:(WKNavigation *)navigation {
//  NSLog(@"Content started arriving: %@", webView.URL.absoluteString);
//}

/// Called when the page finishes loading
- (void)webView:(WKWebView *)webView
    didFinishNavigation:(WKNavigation *)navigation {
  NSURL *url = webView.URL;

  if (url) {
    [self addToVisitedUrls:webView.URL.absoluteString];
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setObject:url.absoluteString forKey:@"url"];
    [dict setObject:@"navigation" forKey:@"event"];
    [dict
        setObject:[NSString stringWithFormat:@"%f", [[NSDate date]
                                                        timeIntervalSince1970]]
           forKey:@"id"];

    // Insert the HTML body inside of 'html_body'
    // And also append ALL the cookies
    [webView evaluateJavaScript:@"document.documentElement.outerHTML.toString()"
              completionHandler:^(NSString *html, NSError *error) {
                if (!error) {
                  [dict setObject:html forKey:@"html_body"];
                } else {
                  NSLog(@"Error fetching the body of html: %@", error);
                }

                // Use WKHTTPCookieStore to get all cookies after JavaScript
                // evaluation
                WKHTTPCookieStore *cookieStore =
                    self.webView.configuration.websiteDataStore.httpCookieStore;
                [cookieStore getAllCookies:^(NSArray<NSHTTPCookie *> *cookies) {
                  for (NSHTTPCookie *cookie in cookies) {
                    // Update self.cookies with the latest cookies
                    [self.cookies setObject:cookie.value forKey:cookie.name];
                  }
                  [dict setObject:self.cookies forKey:@"cookies"];
                  [dict setObject:self.visitedUrls forKey:@"visitedUrls"];

                  NSError *error;
                  NSData *jsonData =
                      [NSJSONSerialization dataWithJSONObject:dict
                                                      options:0
                                                        error:&error];
                  NSString *payload =
                      [[NSString alloc] initWithData:jsonData
                                            encoding:NSUTF8StringEncoding];

                  opacity_core::emit_webview_event([payload UTF8String]);
                  [self resetVisitedUrls];
                }];
              }];
  }
}
- (void)webView:(WKWebView *)webView
    didReceiveServerRedirectForProvisionalNavigation:
        (WKNavigation *)navigation {
  NSURL *url = webView.URL;

  if (url) {
    [self addToVisitedUrls:url.absoluteString];
  }
}

// Called if an error occurs during navigation
- (void)webView:(WKWebView *)webView
    didFailProvisionalNavigation:(WKNavigation *)navigation
                       withError:(NSError *)error {
  NSString *url = error.userInfo[NSURLErrorFailingURLStringErrorKey];
  if (url) {
    [self addToVisitedUrls:webView.URL.absoluteString];
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setObject:url forKey:@"url"];
    [dict setObject:@"navigation" forKey:@"event"];
    [dict
        setObject:[NSString stringWithFormat:@"%f", [[NSDate date]
                                                        timeIntervalSince1970]]
           forKey:@"id"];

    [dict setObject:self.cookies forKey:@"cookies"];
    [dict setObject:self.visitedUrls forKey:@"visitedUrls"];

    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict
                                                       options:0
                                                         error:&error];
    NSString *payload = [[NSString alloc] initWithData:jsonData
                                              encoding:NSUTF8StringEncoding];

    opacity_core::emit_webview_event([payload UTF8String]);
    [self resetVisitedUrls];
  }

  NSLog(@"Failed to load: %@, Error: %@",
        error.userInfo[NSURLErrorFailingURLStringErrorKey],
        error.localizedDescription);
}

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

  if (request.URL) {
    [self addToVisitedUrls:request.URL.absoluteString];
  }
  if (response.URL) {
    [self addToVisitedUrls:response.URL.absoluteString];
  }
  completionHandler(request);
}

// This method is called before the web view starts loading a page
- (void)webView:(WKWebView *)webView
    decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction
                    decisionHandler:
                        (void (^)(WKNavigationActionPolicy))decisionHandler {

  /// We potentially want to intercept navigation requests with deeplinks
  /// A deeplink might take you out of the current app and into the service app
  /// The problem is by canceling the redirection none of the other handlers are
  /// triggered. Which means the cookies at the moment of the redirection are
  /// not sent to Rust. We could potentially move the code of
  /// didFailProvisionalNavigation here and it might work... I don't know, this
  /// needs testing. For now allowing all redirections causes the deeplinks we
  /// need to fail which then triggeres didFailProvisionalNavigation and
  /// extracts and sends the requests to Rust and then to Lua
  //  NSURLRequest *request = navigationAction.request;
  //  NSURL *url = request.URL;
  //  if (![url.scheme isEqualToString:@"http"] &&
  //      ![url.scheme isEqualToString:@"https"]) {
  //    decisionHandler(WKNavigationActionPolicyCancel);
  //    return;
  //  }

  if (webView.URL) {
    [self addToVisitedUrls:webView.URL.absoluteString];
  }

  decisionHandler(WKNavigationActionPolicyAllow);
}

@end
