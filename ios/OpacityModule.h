#ifdef RCT_NEW_ARCH_ENABLED
#import <RNOpacitySpec/RNOpacitySpec.h>
#else
#import <React/RCTBridge.h>
#endif

@interface OpacityModule : NSObject
#ifdef RCT_NEW_ARCH_ENABLED
<NativeOpacitySpec>
#else
<RCTBridgeModule>
#endif
@end
