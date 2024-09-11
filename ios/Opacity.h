#ifdef __cplusplus
#import "react-native-opacity.h"
#endif

#ifdef RCT_NEW_ARCH_ENABLED
#import "RNOpacitySpec.h"

@interface Opacity : NSObject <NativeOpacitySpec>
#else
#import <React/RCTBridgeModule.h>

@interface Opacity : NSObject <RCTBridgeModule>
#endif

@end
