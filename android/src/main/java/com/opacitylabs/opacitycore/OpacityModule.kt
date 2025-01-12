package com.opacitylabs.opacitycore

import com.facebook.react.bridge.Promise
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.module.annotations.ReactModule
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

@ReactModule(name = OpacityModule.NAME)
class OpacityModule(reactContext: ReactApplicationContext) : NativeOpacitySpec(reactContext) {

  override fun getName(): String {
    return NAME
  }

  override fun init(apiKey: String, dryRun: Boolean, environment: Double, promise: Promise) {
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
    val status = OpacityCore.initialize(apiKey, dryRun, environmentEnum)
    if(status != 0) {
      promise.reject("SDK Error", "Could not initialize SDK, check the native logs")
    } else {
      promise.resolve(null)
    }
  }

  override fun getInternal(name: String, params: String?, promise: Promise) {
    CoroutineScope(Dispatchers.IO).launch {
      try {
        val res = OpacityCore.get(name, params) // Assuming this is a blocking call
        val profileMap: Map<String, Any?> = mapOf(
          "json" to res.json,
          // "proof" to res.proof
        )
        withContext(Dispatchers.Main) {
          promise.resolve(profileMap) // Resolve promise on the main thread
        }
      } catch (e: Exception) {
        withContext(Dispatchers.Main) {
          promise.reject("ERROR", e.message, e) // Reject promise on the main thread
        }
      }
    }
  }

  companion object {
    const val NAME = "OpacityModule"
  }
}
