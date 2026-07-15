#include "sdk.h"
#include <android/log.h>
#include <arpa/inet.h>
#include <future>
#include <ifaddrs.h>
#include <jni.h>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <thread>

JavaVM *java_vm;
jobject java_object;

void DeferThreadDetach(JNIEnv *env) {
  static pthread_key_t thread_key;

  // Set up a Thread Specific Data key, and a callback that
  // will be executed when a thread is destroyed.
  // This is only done once, across all threads, and the value
  // associated with the key for any given thread will initially
  // be NULL.
  // static auto run_once = [] {
  //   const auto err = pthread_key_create(&thread_key, [](void *ts_env) {
  //     if (ts_env) {
  //       java_vm->DetachCurrentThread();
  //     }
  //   });
  //   if (err) {
  //     // Failed to create TSD key. Throw an exception if you want to.
  //   }
  //   return 0;
  // }();

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
    if (java_vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
      DeferThreadDetach(env);
    } else {
      // Failed to attach thread. Throw an exception if you want to.
    }
  } else if (get_env_result == JNI_EVERSION) {
    // Unsupported JNI version. Throw an exception if you want to.
  }
  return env;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  java_vm = jvm;
  return JNI_VERSION_1_6;
}

jstring string2jstring(JNIEnv *env, const char *str) {
  return (*env).NewStringUTF(str);
}

static jstring ownedCStringToJString(JNIEnv *env, const char *raw) {
  if (raw == nullptr) {
    return env->NewStringUTF("");
  }

  jstring value = env->NewStringUTF(raw);
  opacity_core::opacity_free_string((char *)raw);
  return value;
}

extern "C" const char *get_browser_overlay_bootstrap_script(void)
    __attribute__((weak));

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

extern "C" void android_present_webview(bool shouldIntercept) {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID method = env->GetMethodID(jOpacityCore, "presentBrowser", "(Z)V");

  // Call the method with the necessary parameters
  jboolean jshouldIntercept = shouldIntercept ? JNI_TRUE : JNI_FALSE;
  env->CallVoidMethod(java_object, method, jshouldIntercept);
}

extern "C" void android_set_cookie(const char *url, const char *value) {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);

  jmethodID method =
      env->GetMethodID(jOpacityCore, "setBrowserCookie",
                       "(Ljava/lang/String;Ljava/lang/String;)V");

  jstring jurl = env->NewStringUTF(url);
  jstring jvalue = env->NewStringUTF(value);
  env->CallVoidMethod(java_object, method, jurl, jvalue);
}

extern "C" void android_webview_change_url(const char *url) {
  JNIEnv *env = GetJniEnv();
  // Get the Kotlin class
  jclass jOpacityCore = env->GetObjectClass(java_object);

  // Get the method ID for the method you want to call
  jmethodID method = env->GetMethodID(jOpacityCore, "changeUrlInBrowser",
                                      "(Ljava/lang/String;)V");

  jstring jurl = env->NewStringUTF(url);
  env->CallVoidMethod(java_object, method, jurl);
}

extern "C" const char *get_ip_address() {
  struct ifaddrs *ifAddrStruct = nullptr;
  void *tmpAddrPtr = nullptr;
  std::string ipAddress = "Unavailable";

  if (getifaddrs(&ifAddrStruct) == 0) {
    for (struct ifaddrs *ifa = ifAddrStruct; ifa != nullptr;
         ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr)
        continue;

      if (ifa->ifa_addr->sa_family == AF_INET) { // Check for IPv4
        tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        std::string interfaceName(ifa->ifa_name);

        // Skip loopback interfaces like "lo"
        if (interfaceName != "lo") {
          ipAddress = addressBuffer;
          break;
        }
      }
    }
    freeifaddrs(ifAddrStruct);
  }

  // Allocate memory for the string and copy its content
  char *result = new char[ipAddress.size() + 1];
  std::strcpy(result, ipAddress.c_str());

  // TODO this will leak! The problem is on iOS inet_ntoa is used which returns
  // a static memory address while there on android we need to manage the memory
  // ourselves :( Need to solve this later by creating an android_free_string
  // function that can be called from Rust once the contents have been copied
  return result; // Caller must free this memory
}

extern "C" bool android_is_app_foregrounded() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "isAppForegrounded", "()Z");
  return env->CallBooleanMethod(java_object, method);
}

extern "C" const char *android_get_os_version() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method =
      env->GetMethodID(jOpacityCore, "getOsVersion", "()Ljava/lang/String;");
  auto res = (jstring)env->CallObjectMethod(java_object, method);

  if (res == nullptr) {
    return "";
  }

  const char *val_str = env->GetStringUTFChars(res, nullptr);
  return val_str;
}

extern "C" const char *android_get_device_manufacturer() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getDeviceManufacturer",
                                      "()Ljava/lang/String;");
  auto res = (jstring)env->CallObjectMethod(java_object, method);

  if (res == nullptr) {
    return "";
  }

  const char *val_str = env->GetStringUTFChars(res, nullptr);
  return val_str;
}

extern "C" const char *android_get_device_model() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method =
      env->GetMethodID(jOpacityCore, "getDeviceModel", "()Ljava/lang/String;");
  auto res = (jstring)env->CallObjectMethod(java_object, method);

  if (res == nullptr) {
    return "";
  }

  const char *val_str = env->GetStringUTFChars(res, nullptr);
  return val_str;
}

extern "C" const char *android_get_device_locale() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method =
      env->GetMethodID(jOpacityCore, "getDeviceLocale", "()Ljava/lang/String;");
  auto res = (jstring)env->CallObjectMethod(java_object, method);

  if (res == nullptr) {
    return "";
  }

  const char *val_str = env->GetStringUTFChars(res, nullptr);
  return val_str;
}

extern "C" int android_get_sdk_version() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getSdkVersion", "()I");
  return env->CallIntMethod(java_object, method);
}

extern "C" int android_get_screen_width() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getScreenWidth", "()I");
  return env->CallIntMethod(java_object, method);
}

extern "C" int android_get_screen_height() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getScreenHeight", "()I");
  return env->CallIntMethod(java_object, method);
}

extern "C" float android_get_screen_density() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getScreenDensity", "()F");
  return env->CallFloatMethod(java_object, method);
}

extern "C" int android_get_screen_dpi() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getScreenDpi", "()I");
  return env->CallIntMethod(java_object, method);
}

extern "C" const char *android_get_device_cpu() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method =
      env->GetMethodID(jOpacityCore, "getDeviceCpu", "()Ljava/lang/String;");
  auto jCpu = (jstring)env->CallObjectMethod(java_object, method);
  if (jCpu == nullptr) {
    return strdup("");
  }
  const char *cpu = env->GetStringUTFChars(jCpu, nullptr);
  char *result = strdup(cpu);
  env->ReleaseStringUTFChars(jCpu, cpu);
  env->DeleteLocalRef(jCpu);
  return result;
}

extern "C" const char *android_get_device_codename() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getDeviceCodename",
                                      "()Ljava/lang/String;");
  auto jCodename = (jstring)env->CallObjectMethod(java_object, method);
  if (jCodename == nullptr) {
    return strdup("");
  }
  const char *codename = env->GetStringUTFChars(jCodename, nullptr);
  char *result = strdup(codename);
  env->ReleaseStringUTFChars(jCodename, codename);
  env->DeleteLocalRef(jCodename);
  return result;
}

extern "C" const char *android_get_bootloader() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getBootloader",
                                      "()Ljava/lang/String;");
  auto jBootloader = (jstring)env->CallObjectMethod(java_object, method);
  if (jBootloader == nullptr) {
    return strdup("");
  }
  const char *bootloader = env->GetStringUTFChars(jBootloader, nullptr);
  char *result = strdup(bootloader);
  env->ReleaseStringUTFChars(jBootloader, bootloader);
  env->DeleteLocalRef(jBootloader);
  return result;
}

extern "C" const char *android_get_radio() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getRadio",
                                      "()Ljava/lang/String;");
  auto jRadio = (jstring)env->CallObjectMethod(java_object, method);
  if (jRadio == nullptr) {
    return strdup("");
  }
  const char *radio = env->GetStringUTFChars(jRadio, nullptr);
  char *result = strdup(radio);
  env->ReleaseStringUTFChars(jRadio, radio);
  env->DeleteLocalRef(jRadio);
  return result;
}

extern "C" const char *android_get_build_time() {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(jOpacityCore, "getBuildTime",
                                      "()Ljava/lang/String;");
  auto jBuildTime = (jstring)env->CallObjectMethod(java_object, method);
  if (jBuildTime == nullptr) {
    return strdup("");
  }
  const char *buildTime = env->GetStringUTFChars(jBuildTime, nullptr);
  char *result = strdup(buildTime);
  env->ReleaseStringUTFChars(jBuildTime, buildTime);
  env->DeleteLocalRef(jBuildTime);
  return result;
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

extern "C" const char *android_get_browser_cookies_for_current_url() {
  JNIEnv *env = GetJniEnv();

  jclass jOpacityCore = env->GetObjectClass(java_object);

  jmethodID method = env->GetMethodID(
      jOpacityCore, "getBrowserCookiesForCurrentUrl", "()Ljava/lang/String;");
  auto res = (jstring)env->CallObjectMethod(java_object, method);

  if (res == nullptr) {
    return nullptr;
  }

  const char *val_str = env->GetStringUTFChars(res, nullptr);
  return val_str;
}

extern "C" const char *android_eval_js(const char *js,
                                       double timeout_in_seconds) {
  JNIEnv *env = GetJniEnv();
  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method = env->GetMethodID(
      jOpacityCore, "evalJs", "(Ljava/lang/String;J)Ljava/lang/String;");
  jstring jjs = env->NewStringUTF(js);
  auto timeout_ms = (jlong)(timeout_in_seconds * 1000.0);
  auto result =
      (jstring)env->CallObjectMethod(java_object, method, jjs, timeout_ms);
  env->DeleteLocalRef(jjs);
  if (result == nullptr) {
    return strdup("{\"result\":null}");
  }
  const char *str = env->GetStringUTFChars(result, nullptr);
  char *copy = strdup(str);
  env->ReleaseStringUTFChars(result, str);
  env->DeleteLocalRef(result);
  return copy;
}

extern "C" const char *
android_get_browser_cookies_for_domain(const char *domain) {
  JNIEnv *env = GetJniEnv();

  jclass jOpacityCore = env->GetObjectClass(java_object);
  jmethodID method =
      env->GetMethodID(jOpacityCore, "getBrowserCookiesForDomain",
                       "(Ljava/lang/String;)Ljava/lang/String;");
  jstring jdomain = env->NewStringUTF(domain);
  auto res = (jstring)env->CallObjectMethod(java_object, method, jdomain);
  if (res == nullptr) {
    return nullptr;
  }
  const char *val_str = env->GetStringUTFChars(res, nullptr);
  //    opacity_core::free_string(domain); // Free the domain string
  return val_str; // Caller must free this memory
}

extern "C" JNIEXPORT jint JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_init(
    JNIEnv *env, jobject thiz, jstring api_key, jboolean dry_run,
    jint environment_enum, jboolean show_errors_in_webview) {
  java_object = env->NewGlobalRef(thiz);
  char *err;
  const char *api_key_str = env->GetStringUTFChars(api_key, nullptr);
  int result = opacity_core::opacity_init(api_key_str, dry_run,
                                  static_cast<int>(environment_enum),
                                  show_errors_in_webview, &err);
  if (result != opacity_core::OPACITY_OK) {
    jclass exceptionClass = env->FindClass("java/lang/Exception");
    env->ThrowNew(exceptionClass, err);
  }

  return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_nativeInitializeOpenTelemetry(
        JNIEnv *env,
        jobject thiz,
        jstring j_open_telemetry_endpoint,
        jstring j_grafana_instance_id,
        jstring j_grafana_api_token
        ) {
  java_object = env->NewGlobalRef(thiz);
  char *err;
  const char *open_telemetry_endpoint = env->GetStringUTFChars(j_open_telemetry_endpoint, nullptr);
  const char *grafana_instance_id = env->GetStringUTFChars(j_grafana_instance_id, nullptr);
  const char *grafana_api_token = env->GetStringUTFChars(j_grafana_api_token, nullptr);

  int result = opacity_core::opacity_initialize_open_telemetry(
          open_telemetry_endpoint,
          grafana_instance_id,
          grafana_api_token, &err);

  if (result != opacity_core::OPACITY_OK) {
    jclass exceptionClass = env->FindClass("java/lang/Exception");
    env->ThrowNew(exceptionClass, err);
  }

  return result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_emitWebviewEvent(
    JNIEnv *env, jobject thiz, jstring event_json) {
  const char *json = env->GetStringUTFChars(event_json, nullptr);
  opacity_core::emit_webview_event(json);
}

jobject createOpacityResponse(JNIEnv *env, int status, char *res, char *err) {
  jclass opacityResponseClass =
      env->FindClass("com/opacitylabs/opacitycore/OpacityResponse");

  jmethodID constructor =
      env->GetMethodID(opacityResponseClass, "<init>",
                       "(ILjava/lang/String;Ljava/lang/String;)V");

  jobject opacityResponse;
  jstring jres, jerr;
  if (status == opacity_core::OPACITY_OK) {
    jres = env->NewStringUTF(res);
    jerr = env->NewStringUTF(nullptr);
    opacity_core::opacity_free_string(res);
  } else {
    jres = env->NewStringUTF(nullptr);
    jerr = env->NewStringUTF(err);
    opacity_core::opacity_free_string(err);
  }

  opacityResponse =
      env->NewObject(opacityResponseClass, constructor, status, jres, jerr);

  return opacityResponse;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_getNative(JNIEnv *env,
                                                       jobject thiz,
                                                       jstring name,
                                                       jstring params) {
  char *res, *err;
  const char *name_str = env->GetStringUTFChars(name, nullptr);
  const char *params_str =
      params != nullptr ? env->GetStringUTFChars(params, nullptr) : nullptr;
  int status = opacity_core::opacity_get(name_str, params_str, &res, &err);
  return createOpacityResponse(env, status, res, err);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_getSdkVersions(JNIEnv *env,
                                                            jobject thiz) {
  const char *res = opacity_core::get_api_version();
  jstring jres = env->NewStringUTF(res);
  //    opacity_core::free_string(res);
  return jres;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_isBrowserOverlayEnabled(
    JNIEnv *env, jobject thiz) {
  return opacity_core::is_browser_overlay_enabled() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_getBrowserOverlayObserverScript(
    JNIEnv *env, jobject thiz) {
  return ownedCStringToJString(
      env, opacity_core::get_browser_overlay_observer_script());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_getBrowserOverlayBootstrapScript(
    JNIEnv *env, jobject thiz) {
  if (get_browser_overlay_bootstrap_script == nullptr) {
    return env->NewStringUTF("");
  }

  return ownedCStringToJString(env, get_browser_overlay_bootstrap_script());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_getBrowserOverlayRendererScript(
    JNIEnv *env, jobject thiz) {
  return ownedCStringToJString(
      env, opacity_core::get_browser_overlay_renderer_script());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_opacitylabs_opacitycore_OpacityCore_isBrowserDebugLogsEnabled(
    JNIEnv *env, jobject thiz) {
  return opacity_core::is_browser_debug_logs_enabled() ? JNI_TRUE : JNI_FALSE;
}
