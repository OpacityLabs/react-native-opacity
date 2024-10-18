#ifdef __OBJC__
#import <Foundation/Foundation.h>
#else
#ifndef OBJC_EXTERN
#if defined(__cplusplus)
#define OBJC_EXTERN extern "C"
#else
#define OBJC_EXTERN extern
#endif
#endif
#endif

#ifdef __cplusplus
namespace opacity {
#endif

OBJC_EXTERN void ios_prepare_request(const char *url);
OBJC_EXTERN void ios_set_request_header(const char *key, const char *value);
OBJC_EXTERN void ios_present_webview();
OBJC_EXTERN void ios_close_webview();

#ifdef __cplusplus
} // namespace opacity
#endif