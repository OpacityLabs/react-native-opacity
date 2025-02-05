package com.opacitylabs.rn

import com.facebook.react.bridge.LifecycleEventListener
import com.facebook.react.bridge.Promise
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReadableMap
import com.facebook.react.module.annotations.ReactModule
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import com.opacitylabs.opacitycore.OpacityCore

@ReactModule(name = OpacityModule.NAME)
class OpacityModule(private val reactContext: ReactApplicationContext) :
  NativeOpacitySpec(reactContext),
  LifecycleEventListener {

  override fun getName(): String {
    return NAME
  }

  init {
    reactContext.addLifecycleEventListener(this)
  }

  override fun init(
    apiKey: String,
    dryRun: Boolean,
    environment: Double,
    shouldShowErrorsInWebView: Boolean,
    promise: Promise
  ) {
    val environmentEnum = when (environment) {
      0.0 -> OpacityCore.Environment.TEST
      1.0 -> OpacityCore.Environment.LOCAL
      2.0 -> OpacityCore.Environment.STAGING
      3.0 -> OpacityCore.Environment.PRODUCTION
      else -> {
        promise.reject("Invalid Arguments", "Invalid environment value")
        return
      }
    }
    val status = OpacityCore.initialize(apiKey, dryRun, environmentEnum, shouldShowErrorsInWebView)
    if (status != 0) {
      promise.reject("SDK Error", "Could not initialize SDK, check the native logs")
    } else {
      promise.resolve(null)
    }
  }

  override fun getInternal(name: String, params: ReadableMap?, promise: Promise) {
    CoroutineScope(Dispatchers.IO).launch {
      try {
        val res = OpacityCore.get(name, params?.toHashMap())
        val resMap = mapToWritableMap(res)
        withContext(Dispatchers.Main) {
          promise.resolve(resMap) // Resolve promise on the main thread
        }
      } catch (e: Exception) {
        withContext(Dispatchers.Main) {
          promise.reject("ERROR", e.message, e) // Reject promise on the main thread
        }
      }
    }
  }

  override fun onHostDestroy() {
//    TODO("Not yet implemented")
  }

  override fun onHostPause() {
//    TODO("Not yet implemented")
  }

  override fun onHostResume() {
    val activity = currentActivity
    if (activity != null) {
      OpacityCore.setContext(activity)
      reactContext.removeLifecycleEventListener(this) // Cleanup listener
    }
  }

  companion object {
    const val NAME = "OpacityModule"
  }
}
