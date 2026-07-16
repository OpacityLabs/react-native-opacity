package com.opacitylabs.opacitycore

import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicInteger
import com.opacitylabs.opacitycore.JsonConverter.Companion.mapToJsonElement
import com.opacitylabs.opacitycore.JsonConverter.Companion.parseJsonElementToAny
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.jsonObject

object OpacityCore {
    enum class Environment(val code: Int) {
        LOCAL(1),
        SANDBOX(2),
        STAGING(3),
        PRODUCTION(4),
    }

    private lateinit var appContext: Context
    private lateinit var cryptoManager: CryptoManager
    private lateinit var _url: String
    private var headers: Bundle = Bundle()
    private var pendingCookies: MutableList<Pair<String, String>> = mutableListOf()
    private var isBrowserActive = false

    // --- eval state ---
    private val pendingEvals = ConcurrentHashMap<String, PendingEval>()
    private val evalIdCounter = AtomicInteger(0)
    private var activeWebViewActivity: java.lang.ref.WeakReference<InAppBrowserActivity>? = null

    private data class PendingEval(
        val latch: CountDownLatch = CountDownLatch(1),
        var result: String = "{\"result\":null}"
    )

    init {
        System.loadLibrary("OpacityCore")
    }

    @JvmStatic
    fun initialize(
        apiKey: String,
        dryRun: Boolean,
        environment: Environment,
        showErrorsInWebView: Boolean
    ): Int {
        return init(apiKey, dryRun, environment.code, showErrorsInWebView)
    }

    @JvmStatic
    fun initializeOpenTelemetry(
        openTelemetryEndpoint: String,
        grafanaInstanceId: String,
        grafanaApiToken: String
    ): Int {
        return nativeInitializeOpenTelemetry(openTelemetryEndpoint, grafanaInstanceId, grafanaApiToken)
    }

    @JvmStatic
    fun setContext(context: Context) {
        appContext = context
        cryptoManager = CryptoManager(appContext.applicationContext)
    }

    fun isAppForegrounded(): Boolean {
        return try {
            val activityManager = appContext.getSystemService(Context.ACTIVITY_SERVICE) as android.app.ActivityManager
            val runningProcesses = activityManager.runningAppProcesses
            if (runningProcesses != null) {
                for (processInfo in runningProcesses) {
                    if (processInfo.processName == appContext.packageName) {
                        return processInfo.importance == android.app.ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND
                    }
                }
            }
            false
        } catch (e: Exception) {
            false
        }
    }

    fun getOsVersion(): String {
        return Build.VERSION.RELEASE
    }

    fun getSdkVersion(): Int {
        return Build.VERSION.SDK_INT
    }

    fun getDeviceManufacturer(): String {
        return Build.MANUFACTURER
    }

    fun getDeviceModel(): String {
        return Build.MODEL
    }

    fun getDeviceLocale(): String {
        return java.util.Locale.getDefault().toLanguageTag()
    }

    fun getScreenWidth(): Int {
        val displayMetrics = appContext.resources.displayMetrics
        return displayMetrics.widthPixels
    }

    fun getScreenHeight(): Int {
        val displayMetrics = appContext.resources.displayMetrics
        return displayMetrics.heightPixels
    }

    fun getScreenDensity(): Float {
        val displayMetrics = appContext.resources.displayMetrics
        return displayMetrics.density
    }

    fun getScreenDpi(): Int {
        val displayMetrics = appContext.resources.displayMetrics
        return displayMetrics.densityDpi
    }

    fun getDeviceCpu(): String {
        return Build.SUPPORTED_ABIS.firstOrNull() ?: Build.CPU_ABI
    }

    fun getDeviceCodename(): String {
        return Build.DEVICE
    }

    fun getBootloader(): String {
        return Build.BOOTLOADER
    }

    fun getRadio(): String {
        return Build.getRadioVersion() ?: ""
    }

    fun getBuildTime(): String {
        return Build.TIME.toString()
    }

    fun securelySet(key: String, value: String) {
        cryptoManager.set(key, value)
    }

    fun securelyGet(key: String): String? {
        return cryptoManager.get(key)
    }

    fun prepareInAppBrowser(url: String) {
        headers = Bundle()
        pendingCookies = mutableListOf()
        _url = url
    }

    fun setBrowserHeader(key: String, value: String) {
        headers.putString(key.lowercase(), value)
    }

    fun setBrowserCookie(url: String, value: String) {
        pendingCookies.add(Pair(url, value))
    }

    fun presentBrowser(shouldIntercept: Boolean) {
        val intent = Intent(appContext, InAppBrowserActivity::class.java)
        intent.putExtra("url", _url)
        intent.putExtra("headers", headers)
        intent.putExtra("enableInterceptRequests", shouldIntercept)
        if (pendingCookies.isNotEmpty()) {
            val cookieUrls = pendingCookies.map { it.first }.toTypedArray()
            val cookieValues = pendingCookies.map { it.second }.toTypedArray()
            intent.putExtra("cookieUrls", cookieUrls)
            intent.putExtra("cookieValues", cookieValues)
        }
        appContext.startActivity(intent)
        isBrowserActive = true
    }

    fun getBrowserCookiesForCurrentUrl(): String? {
        if (!isBrowserActive) {
            return null
        }

        val cookiesIntent = Intent("com.opacitylabs.opacitycore.GET_COOKIES_FOR_CURRENT_URL")
        val resultReceiver = CookieResultReceiver()
        cookiesIntent.putExtra("receiver", resultReceiver)
        LocalBroadcastManager.getInstance(appContext).sendBroadcast(cookiesIntent)
        val json = resultReceiver.waitForResult(1000) // Wait up to 1 second for the result
        return json?.toString()
    }

    fun getBrowserCookiesForDomain(domain: String): String? {
        if (!isBrowserActive) {
            return null
        }

        val cookiesIntent = Intent("com.opacitylabs.opacitycore.GET_COOKIES_FOR_DOMAIN")
        val resultsReceiver = CookieResultReceiver()
        cookiesIntent.putExtra("receiver", resultsReceiver)
        cookiesIntent.putExtra("domain", domain)
        LocalBroadcastManager.getInstance(appContext).sendBroadcast(cookiesIntent)
        val json = resultsReceiver.waitForResult(1000)
        return json?.toString()
    }

    fun closeBrowser() {
        val closeIntent = Intent("com.opacitylabs.opacitycore.CLOSE_BROWSER")
        LocalBroadcastManager.getInstance(appContext).sendBroadcast(closeIntent)
    }

    fun changeUrlInBrowser(url: String) {
        val changeUrlIntent = Intent("com.opacitylabs.opacitycore.CHANGE_URL")
        changeUrlIntent.putExtra("url", url)
        LocalBroadcastManager.getInstance(appContext).sendBroadcast(changeUrlIntent)
    }

    fun onBrowserDestroyed() {
        isBrowserActive = false
    }

    fun setActiveWebViewActivity(activity: InAppBrowserActivity?) {
        activeWebViewActivity = activity?.let { java.lang.ref.WeakReference(it) }
    }

    fun notifyWebViewEvalResult(id: String, json: String) {
        val pending = pendingEvals[id] ?: return
        pending.result = json
        pending.latch.countDown()
    }

    fun evalJs(js: String, timeoutMs: Long): String {
        val activity = activeWebViewActivity?.get()
        if (activity == null || activity.isFinishing || activity.isDestroyed) {
            return "{\"error\":\"no active webview\"}"
        }

        val id = "eval_${evalIdCounter.incrementAndGet()}"

        if (timeoutMs == 0L) {
            activity.dispatchWebViewEval(id, js, fireAndForget = true)
            return "{\"result\":null}"
        }

        val pending = PendingEval()
        pendingEvals[id] = pending
        activity.dispatchWebViewEval(id, js, fireAndForget = false)
        pending.latch.await(timeoutMs, TimeUnit.MILLISECONDS)
        pendingEvals.remove(id)
        return pending.result
    }

    private fun parseOpacityError(error: String?): OpacityError {
        if (error == null) {
            return OpacityError("UnknownError", "No Message")
        }
        return try {
            val json = Json.parseToJsonElement(error).jsonObject
            val code = json["code"]?.toString()?.replace("\"", "") ?: "unknown"
            val description = json["description"]?.toString()?.replace("\"", "") ?: "unknown"
            OpacityError(code, description)
        } catch (e: Exception) {
            OpacityError("UnknownError", error)
        }
    }

    @JvmStatic
    suspend fun get(
        name: String,
        params: Map<String, Any?>?
    ): Result<Map<String, Any?>> {
        return runGet(name, params, null, null)
    }

    /**
     * Runs the flow joined to the caller's W3C trace. [traceparent] is required;
     * [tracestate] is optional (W3C allows it to be absent).
     */
    @JvmStatic
    suspend fun getWithContext(
        name: String,
        params: Map<String, Any?>?,
        traceparent: String,
        tracestate: String? = null
    ): Result<Map<String, Any?>> {
        return runGet(name, params, traceparent, tracestate)
    }

    private suspend fun runGet(
        name: String,
        params: Map<String, Any?>?,
        traceparent: String?,
        tracestate: String?
    ): Result<Map<String, Any?>> {
        return withContext(Dispatchers.IO) {
            val paramsString = params?.let {
                val jsonElement = mapToJsonElement(it)
                Json.encodeToString(jsonElement)
            }

            val res = getNative(name, paramsString, traceparent, tracestate)
            if (res.status != 0) {
                return@withContext Result.failure(parseOpacityError(res.err))
            }

            val map: Map<String, Any?> =
                Json.parseToJsonElement(res.data!!).jsonObject.mapValues {
                    parseJsonElementToAny(it.value)
                }

            return@withContext Result.success(map)
        }
    }

    private external fun init(
        apiKey: String,
        dryRun: Boolean,
        environment: Int,
        showErrorsInWebView: Boolean
    ): Int

    private external fun nativeInitializeOpenTelemetry(openTelemetryEndpoint: String, grafanaInstanceId: String, grafanaApiToken: String): Int

    private external fun getNative(
        name: String,
        params: String?,
        traceparent: String?,
        tracestate: String?
    ): OpacityResponse
    external fun getSdkVersions(): String
    external fun emitWebviewEvent(eventJson: String)
    external fun isBrowserOverlayEnabled(): Boolean
    external fun getBrowserOverlayObserverScript(): String
    external fun getBrowserOverlayBootstrapScript(): String
    external fun getBrowserOverlayRendererScript(): String
    external fun isBrowserDebugLogsEnabled(): Boolean
}
