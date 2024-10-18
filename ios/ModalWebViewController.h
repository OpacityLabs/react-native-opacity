#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

@interface ModalWebViewController : UIViewController <WKNavigationDelegate, NSURLSessionTaskDelegate>

- (instancetype)initWithRequest:(NSMutableURLRequest *)request;
- (void)close;
- (void)openRequest:(NSMutableURLRequest *)request;
@end
