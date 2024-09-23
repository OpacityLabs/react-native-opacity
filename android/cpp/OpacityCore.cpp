#include "opacity.h"
#include <android/log.h>
#include <future>
#include <jni.h>
#include <thread>

JavaVM *java_vm;
jobject java_object;

struct OpacityResponse {
  int status;
  const char *json;
  const char *proof;
  const char *err;
};

void DeferThreadDetach(JNIEnv *env) {
  static pthread_key_t thread_key;

  // Set up a Thread Specific Data key, and a callback that
  // will be executed when a thread is destroyed.
  // This is only done once, across all threads, and the value
  // associated with the key for any given thread will initially
  // be NULL.
  static auto run_once = [] {
    const auto err = pthread_key_create(&thread_key, [](void *ts_env) {
      if (ts_env) {
        java_vm->DetachCurrentThread();
      }
    });
    if (err) {
      // Failed to create TSD key. Throw an exception if you want to.
    }
    return 0;
  }();

  // For the callback to actually be executed when a thread exits
  // we need to associate a non-NULL value with the key on that thread.
  // We can use the JNIEnv* as that value.
  const auto ts_env = pthread_getspecific(thread_key);
  if (!ts_env) {
    if (pthread_setspecific(thread_key, env)) {
      // Failed to set thread-specific value for key. Throw an exception if you
      // want to.
    }
  }
}

JNIEnv *GetJniEnv() {
  JNIEnv *env = nullptr;
  // We still call GetEnv first to detect if the thread already
  // is attached. This is done to avoid setting up a DetachCurrentThread
  // call on a Java thread.

  // g_vm is a global.
  auto get_env_result = java_vm->GetEnv((void **)&env, JNI_VERSION_1_6);
  if (get_env_result == JNI_EDETACHED) {
    if (java_vm->AttachCurrentThread(&env, NULL) == JNI_OK) {
      DeferThreadDetach(env);
    } else {
      // Failed to attach thread. Throw an exception if you want to.
    }
  } else if (get_env_result == JNI_EVERSION) {
    // Unsupported JNI version. Throw an exception if you want to.
  }
  return env;
}

jstring string2jstring(JNIEnv *env, const char *str) {
  return (*env).NewStringUTF(str);
}

extern "C" void secure_set(const char *key, const char *value) {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID set_method = env->GetMethodID(
      jOpacityCore, "securelySet", "(Ljava/lang/String;Ljava/lang/String;)V");

  env->CallVoidMethod(java_object, set_method, string2jstring(env, key),
                      string2jstring(env, value));
}

extern "C" const char *secure_get(const char *key) {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID set_method = env->GetMethodID(
      jOpacityCore, "securelyGet", "(Ljava/lang/String;)Ljava/lang/String;");

  auto res = (jstring)env->CallObjectMethod(java_object, set_method,
                                            string2jstring(env, key));

  if (res == nullptr) {
    return nullptr;
  }

  const char *val_str = env->GetStringUTFChars(res, nullptr);
  return val_str;
}

extern "C" void android_prepare_request(const char *url) {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID openBrowserMethod = env->GetMethodID(
      jOpacityCore, "prepareInAppBrowser", "(Ljava/lang/String;)V");

  // Call the method with the necessary parameters
  jstring jurl = env->NewStringUTF(url);
  env->CallVoidMethod(java_object, openBrowserMethod, jurl);
}

extern "C" void android_set_request_header(const char *key, const char *value) {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID method =
      env->GetMethodID(jOpacityCore, "setBrowserHeader",
                       "(Ljava/lang/String;Ljava/lang/String;)V");

  // Call the method with the necessary parameters
  jstring jkey = env->NewStringUTF(key);
  jstring jvalue = env->NewStringUTF(value);
  env->CallVoidMethod(java_object, method, jkey, jvalue);
}

extern "C" void android_present_webview() {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID method = env->GetMethodID(jOpacityCore, "presentBrowser", "()V");

  // Call the method with the necessary parameters
  env->CallVoidMethod(java_object, method);
}

extern "C" void android_close_webview() {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID method = env->GetMethodID(jOpacityCore, "closeBrowser", "()V");

  // Call the method with the necessary parameters
  env->CallVoidMethod(java_object, method);
}

JavaVM *getJavaVM(JNIEnv *env) {
  JavaVM *vm;
  if (env->GetJavaVM(&vm) != JNI_OK) {
    throw std::runtime_error("Failed to retrieve JavaVM");
  }
  return vm;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_init(JNIEnv *env, jobject thiz) {
  JavaVM *vm = getJavaVM(env);
  java_vm = vm;
  java_object = env->NewGlobalRef(thiz);
  return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_emitWebviewEvent(
    JNIEnv *env, jobject thiz, jstring event_json) {
  const char *json = env->GetStringUTFChars(event_json, nullptr);
  opacity_core::emit_webview_event(json);
}
