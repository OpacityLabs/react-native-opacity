#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

@interface ModalWebViewController
    : UIViewController <WKNavigationDelegate, WKUIDelegate, NSURLSessionTaskDelegate, WKScriptMessageHandler>

@property(nonatomic, copy) void (^onDismissCallback)(void);

- (instancetype)initWithRequest:(NSMutableURLRequest *)request
                      userAgent:(NSString *)userAgent
              interceptRequests:(bool)interceptRequests
                 initialCookies:(NSArray<NSHTTPCookie *> *)initialCookies;
- (void)close;
- (void)openRequest:(NSMutableURLRequest *)request;
- (NSDictionary *)getBrowserCookiesForCurrentUrl;
- (NSDictionary *)getBrowserCookiesForDomain:(NSString *)domain;
- (NSString *)evalJs:(NSString *)js timeout:(double)timeoutSeconds;
@end
