package com.opacitylabs.rn

import com.facebook.react.bridge.LifecycleEventListener
import com.facebook.react.bridge.Promise
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReadableMap
import com.facebook.react.module.annotations.ReactModule
import com.opacitylabs.opacitycore.OpacityCore
import com.opacitylabs.opacitycore.OpacityError
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

@ReactModule(name = OpacityModule.NAME)
class OpacityModule(private val reactContext: ReactApplicationContext) :
  NativeOpacitySpec(reactContext), LifecycleEventListener {

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
    val environmentEnum =
      when (environment) {
        0.0 -> OpacityCore.Environment.TEST
        1.0 -> OpacityCore.Environment.LOCAL
        2.0 -> OpacityCore.Environment.STAGING
        3.0 -> OpacityCore.Environment.STAGING
        4.0 -> OpacityCore.Environment.PRODUCTION
        else -> {
          promise.reject("Invalid Arguments", "Invalid environment value")
          return
        }
      }
    try {
      OpacityCore.initialize(apiKey, dryRun, environmentEnum, shouldShowErrorsInWebView)
      promise.resolve(null)
    } catch (e: Exception) {
      promise.reject("Initialization Error", e.message, e)
    }
  }

  override fun getInternal(name: String, params: ReadableMap?, promise: Promise) {
    CoroutineScope(Dispatchers.IO).launch {
      val res = OpacityCore.get(name, params?.toHashMap())
      res.fold(
        onSuccess = { value ->
          run {
            val resMap = mapToWritableMap(value)
            withContext(Dispatchers.Main) {
              promise.resolve(resMap)
            }
          }
        },
        onFailure = {
          when (it) {
            is OpacityError -> {
              withContext(Dispatchers.Main) {
                promise.reject(it.code, it.message, it)
              }
            }

            else -> {
              withContext(Dispatchers.Main) {
                promise.reject("UnknownError", it.message, it)
              }
            }
          }
        }
      )
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
