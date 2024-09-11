#include <jni.h>
#include "react-native-opacity.h"

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_opacity_OpacityModule_nativeMultiply(JNIEnv *env, jclass type, jdouble a, jdouble b) {
    return opacity::multiply(a, b);
}
