package com.opacitylabs.opacitycore

import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.Gravity
import android.view.ViewGroup
import android.webkit.CookieManager
import android.webkit.JavascriptInterface
import android.webkit.WebResourceRequest
import android.webkit.WebResourceResponse
import android.webkit.WebView
import android.webkit.ConsoleMessage
import android.webkit.WebChromeClient
import android.webkit.WebViewClient
import android.widget.Button
import android.widget.LinearLayout
import androidx.appcompat.app.ActionBar
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import androidx.webkit.WebViewCompat
import androidx.webkit.WebViewFeature
import org.json.JSONObject

class InAppBrowserActivity : AppCompatActivity() {
    private val closeReceiver =
        object : BroadcastReceiver() {
            override fun onReceive(context: Context?, intent: Intent?) {
                if (intent?.action == "com.opacitylabs.opacitycore.CLOSE_BROWSER") {
                    finish()
                }
            }
        }

    private val cookiesForCurrentURLRequestReceiver =
        object : BroadcastReceiver() {
            override fun onReceive(context: Context?, intent: Intent?) {
                if (intent?.action == "com.opacitylabs.opacitycore.GET_COOKIES_FOR_CURRENT_URL") {
                    val receiver = intent.getParcelableExtra<CookieResultReceiver>("receiver")
                    if (currentUrl.isNotEmpty() && currentUrl.startsWith("http")) {
                        updateCookiesFromCookieManager(currentUrl)
                        val domain = java.net.URL(currentUrl).host
                        receiver?.onReceiveResult(getMatchedCookies(domain))
                    } else {
                        receiver?.onReceiveResult(JSONObject())
                    }
                }
            }
        }

    private val cookiesForDomainRequestReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            if (intent?.action == "com.opacitylabs.opacitycore.GET_COOKIES_FOR_DOMAIN") {
                val receiver = intent.getParcelableExtra<CookieResultReceiver>("receiver")
                var domain = intent.getStringExtra("domain")
                if (domain?.startsWith(".") == true) {
                    domain = domain.substring(1)
                }

                if (domain != null) {
                    updateCookiesFromCookieManager(currentUrl)
                    updateCookiesFromCookieManager("https://$domain")
                    receiver?.onReceiveResult(getMatchedCookies(domain))
                } else {
                    receiver?.onReceiveResult(JSONObject())
                }
            }
        }
    }

    private lateinit var webView: WebView
    private var cookies: MutableMap<String, JSONObject> = mutableMapOf()

    private fun getMatchedCookies(domain: String): JSONObject {
        val matchedCookies = JSONObject()
        for ((cookieDomain, cookieObject) in cookies) {
            val cleanDomain = cookieDomain.trimStart('.')
            if (domain == cleanDomain || domain.endsWith(".$cleanDomain")) {
                val keys = cookieObject.keys()
                while (keys.hasNext()) {
                    val key = keys.next()
                    matchedCookies.put(key, cookieObject.get(key))
                }
            }
        }
        return matchedCookies
    }

    private var htmlBody: String = ""
    private var currentUrl: String = ""
    private val visitedUrls = mutableListOf<String>()
    private var interceptExtensionEnabled = false
    private val pendingPostBodies = java.util.concurrent.ConcurrentHashMap<String, String>()
    private var overlayEnabled = false
    private var overlayScriptsInstalledAtDocumentStart = false
    private var overlayBootstrapScript: String? = null
    private var overlayObserverScript: String? = null
    private var overlayRendererScript: String? = null

    private val changeUrlReceiver =
        object : BroadcastReceiver() {
            override fun onReceive(context: Context?, intent: Intent?) {
                if (intent?.action == "com.opacitylabs.opacitycore.CHANGE_URL") {
                    val targetUrl = intent.getStringExtra("url")!!
                    currentUrl = targetUrl
                    webView.loadUrl(targetUrl)
                }
            }
        }

    /**
     * Bridge object exposed to JavaScript as `window.OpacityNative`.
     */
    inner class OpacityJsBridge {
        @JavascriptInterface
        fun onInterceptedRequest(json: String) {
            if (!interceptExtensionEnabled) return
            try {
                val requestData = JSONObject(json)
                emitInterceptedRequest(requestData)
            } catch (e: Exception) {
                Log.e("Opacity SDK", "Error parsing intercepted request", e)
            }
        }

        @JavascriptInterface
        fun storePostBody(url: String, body: String) {
            Log.d("Opacity SDK", "storePostBody: url=$url (${body.length} bytes)")
            pendingPostBodies[url] = body
        }

        @JavascriptInterface
        fun notifyEvalResult(id: String, json: String) {
            OpacityCore.notifyWebViewEvalResult(id, json)
        }

        @JavascriptInterface
        fun onRenderedHtmlReady(json: String) {
            try {
                val payload = JSONObject(json)
                val mapperJson = payload.optString("mapper_json")
                if (mapperJson.isBlank()) {
                    return
                }

                Handler(Looper.getMainLooper()).post {
                    presentGeneratedOverlayWithMapperJson(mapperJson)
                }
            } catch (e: Exception) {
                Log.e("Opacity SDK", "Error parsing renderedHtmlReady payload", e)
            }
        }
    }

    private fun installOverlayDocumentStartScriptsIfSupported() {
        if (!overlayEnabled || overlayScriptsInstalledAtDocumentStart) {
            return
        }

        val bootstrap = overlayBootstrapScript ?: return
        val observer = overlayObserverScript ?: return

        if (!WebViewFeature.isFeatureSupported(WebViewFeature.DOCUMENT_START_SCRIPT)) {
            return
        }

        try {
            val overlayOriginRules = setOf("*")
            WebViewCompat.addDocumentStartJavaScript(webView, bootstrap, overlayOriginRules)
            WebViewCompat.addDocumentStartJavaScript(webView, observer, overlayOriginRules)
            overlayScriptsInstalledAtDocumentStart = true
        } catch (e: Exception) {
            Log.e("Opacity SDK", "Failed to install document-start overlay scripts", e)
        }
    }

    private fun injectOverlayScriptsIntoPage(view: WebView?) {
        if (!overlayEnabled || overlayScriptsInstalledAtDocumentStart) {
            return
        }

        val target = view ?: webView
        overlayBootstrapScript?.let { target.evaluateJavascript(it, null) }
        overlayObserverScript?.let { target.evaluateJavascript(it, null) }
    }

    private fun presentGeneratedOverlayWithMapperJson(mapperJson: String) {
        val template = overlayRendererScript
        if (template.isNullOrBlank()) {
            return
        }

        val mapperLiteral = JSONObject.quote(mapperJson)
        val script = template.replace("%@", mapperLiteral).replace("%%", "%")
        dispatchWebViewEval("overlay_${System.currentTimeMillis()}", script, fireAndForget = true)
    }

    @SuppressLint("WrongThread", "SetJavaScriptEnabled")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Handle edge-to-edge display for Android 15+
        window.statusBarColor = android.graphics.Color.TRANSPARENT
        window.navigationBarColor = android.graphics.Color.TRANSPARENT

        ViewCompat.setOnApplyWindowInsetsListener(window.decorView) { view, windowInsets ->
            val insets = windowInsets.getInsets(WindowInsetsCompat.Type.systemBars())
            view.setPadding(0, insets.top, 0, insets.bottom)
            WindowInsetsCompat.CONSUMED
        }

        val localBroadcastManager = LocalBroadcastManager.getInstance(this)
        localBroadcastManager.registerReceiver(
            closeReceiver,
            IntentFilter("com.opacitylabs.opacitycore.CLOSE_BROWSER")
        )

        localBroadcastManager.registerReceiver(
            cookiesForCurrentURLRequestReceiver,
            IntentFilter("com.opacitylabs.opacitycore.GET_COOKIES_FOR_CURRENT_URL")
        )

        localBroadcastManager.registerReceiver(
            cookiesForDomainRequestReceiver,
            IntentFilter("com.opacitylabs.opacitycore.GET_COOKIES_FOR_DOMAIN")
        )

        localBroadcastManager.registerReceiver(
            changeUrlReceiver,
            IntentFilter("com.opacitylabs.opacitycore.CHANGE_URL")
        )

        supportActionBar?.setDisplayShowTitleEnabled(false)

        val closeButton =
            Button(this, null, android.R.attr.buttonStyleSmall).apply {
                text = "✕"
                textSize = 18f
                setBackgroundColor(android.graphics.Color.TRANSPARENT)
                setOnClickListener { onClose() }
            }

        val actionBarLayoutParams =
            ActionBar.LayoutParams(
                ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT,
                Gravity.END or Gravity.CENTER_VERTICAL
            )
        supportActionBar?.setCustomView(closeButton, actionBarLayoutParams)
        supportActionBar?.setDisplayShowCustomEnabled(true)

        OpacityCore.setActiveWebViewActivity(this)
        interceptExtensionEnabled = intent.getBooleanExtra("enableInterceptRequests", false)
        // Clear cookies for private-mode-like behavior
        CookieManager.getInstance().removeAllCookies(null)
        CookieManager.getInstance().setAcceptCookie(true)

        // Create WebView
        webView = WebView(this).apply {
            settings.javaScriptEnabled = true
            settings.domStorageEnabled = true
            settings.databaseEnabled = true
            settings.javaScriptCanOpenWindowsAutomatically = true
            settings.setSupportMultipleWindows(false)

            addJavascriptInterface(OpacityJsBridge(), "OpacityNative")
        }

        overlayEnabled = OpacityCore.isBrowserOverlayEnabled()
        if (overlayEnabled) {
            overlayBootstrapScript = OpacityCore.getBrowserOverlayBootstrapScript()
            overlayObserverScript = OpacityCore.getBrowserOverlayObserverScript()
            overlayRendererScript = OpacityCore.getBrowserOverlayRendererScript()
            installOverlayDocumentStartScriptsIfSupported()
        }

        // Forward JS console messages to logcat when the active Lua flow opts in via
        // open_browser({ debug_logs = true }). Flag is reset to false on close_browser.
        if (OpacityCore.isBrowserDebugLogsEnabled()) {
            webView.webChromeClient = object : WebChromeClient() {
                override fun onConsoleMessage(consoleMessage: ConsoleMessage?): Boolean {
                    Log.d(
                        "Opacity SDK",
                        "JS ${consoleMessage?.messageLevel()}: ${consoleMessage?.message()}"
                    )
                    return true
                }
            }
        }

        val headers: Bundle? = intent.getBundleExtra("headers")
        val customUserAgent = headers?.getString("user-agent")
        if (customUserAgent != null) {
            webView.settings.userAgentString = customUserAgent
        }

        CookieManager.getInstance().setAcceptThirdPartyCookies(webView, true)

        webView.webViewClient = object : WebViewClient() {
            /**
            Uber breaks with the default webview because it automatically adds a X-Requested-With: header
            which they detect, so we use this override which gives us the request before it's executed with
            the option of executing it ourselves, which is where we can strip the header. This specific override
            does not support POSTs as in there is no way to get the request body using this func, which is where
            we use the INTERCEPT_SCRIPT to get the request body, send it back to kotlin, then we can start stripping,
            the header for post requests also. for eg, it is needed for when we actually login (POST v2/submit-form)
            to remove that header.

            Similar to google signins, google have made it clear they do not want webviews to be logging in through OAuth
            https://developers.googleblog.com/upcoming-security-changes-to-googles-oauth-20-authorization-endpoint-in-embedded-webviews/
            They also check for the X-Requested-With header and the User-Agent, from testing they don't enforce the checks for any
            of their POSTs, only GETs so we just intercept that.
             */
            override fun shouldInterceptRequest(
                view: WebView?,
                request: WebResourceRequest?
            ): WebResourceResponse? {

                if (request == null) return null
                val url = request.url?.toString() ?: return null

                val scheme = request.url?.scheme?.lowercase()
                if (scheme != "http" && scheme != "https") return null

                // Only intercept Uber domains
                val host = request.url?.host?.lowercase() ?: ""
                if (!host.endsWith("uber.com") && !host.endsWith("accounts.google.com/v3/signin")) return null


                // For submit-form POST, use the stored body from JS
                val isSubmitForm =
                    request.method == "POST" && url.contains("auth.uber.com/v2/submit-form")
                val storedBody = if (isSubmitForm) pendingPostBodies.remove(url) else null
                if (request.method != "GET" && request.method != "HEAD" && !isSubmitForm) return null
                if (isSubmitForm) {
                    Log.d(
                        "Opacity SDK",
                        "submit-form intercepted: storedBody=${if (storedBody != null) "${storedBody.length} bytes" else "MISSING"}"
                    )
                    if (storedBody == null) return null
                }

                try {
                    val conn = java.net.URL(url).openConnection() as java.net.HttpURLConnection
                    conn.requestMethod = request.method
                    conn.instanceFollowRedirects = true
                    conn.connectTimeout = 15000
                    conn.readTimeout = 15000

                    if (storedBody != null) {
                        conn.doOutput = true
                    }

                    request.requestHeaders?.forEach { (key, value) ->
                        if (!key.equals("x-requested-with", ignoreCase = true) &&
                            !key.equals("accept-encoding", ignoreCase = true)
                        ) {
                            conn.setRequestProperty(key, value)
                        }
                    }

                    val cookieStr = CookieManager.getInstance().getCookie(url)
                    if (cookieStr != null) {
                        conn.setRequestProperty("Cookie", cookieStr)
                    }

                    if (storedBody != null) {
                        val bodyBytes = storedBody.toByteArray(Charsets.UTF_8)
                        conn.setRequestProperty("Content-Length", bodyBytes.size.toString())
                        conn.outputStream.use { it.write(bodyBytes) }
                    }

                    conn.connect()

                    val responseCode = conn.responseCode
                    if (responseCode in 300..399) {
                        conn.disconnect()
                        return null
                    }

                    val reasonPhrase = conn.responseMessage?.ifEmpty { "OK" } ?: "OK"
                    val inputStream = try {
                        conn.inputStream
                    } catch (e: Exception) {
                        conn.errorStream
                    }

                    conn.headerFields?.forEach { (key, values) ->
                        if (key?.equals("Set-Cookie", ignoreCase = true) == true) {
                            values.forEach { value ->
                                CookieManager.getInstance().setCookie(url, value)
                            }
                        }
                    }

                    val contentType = conn.contentType ?: "text/html"
                    val mimeType = contentType.split(";")[0].trim()
                    val charsetMatch = Regex("charset=([^;\\s]+)").find(contentType)
                    val charset = charsetMatch?.groupValues?.get(1)?.trim() ?: "UTF-8"

                    val skipHeaders = setOf(
                        "content-encoding",
                        "transfer-encoding",
                        "content-length",
                        "connection"
                    )
                    val responseHeaders = mutableMapOf<String, String>()
                    conn.headerFields?.forEach { (key, values) ->
                        if (key != null && values.isNotEmpty() && !skipHeaders.contains(key.lowercase())) {
                            responseHeaders[key] = values.last()
                        }
                    }

                    return WebResourceResponse(
                        mimeType,
                        charset,
                        responseCode,
                        reasonPhrase,
                        responseHeaders,
                        inputStream
                    )
                } catch (e: Exception) {
                    Log.e("Opacity SDK", "shouldInterceptRequest error for $url", e)
                    return null
                }
            }

            override fun shouldOverrideUrlLoading(
                view: WebView?,
                request: WebResourceRequest?
            ): Boolean {
                val url = request?.url?.toString() ?: return false
                currentUrl = url
                addToVisitedUrls(url)
                emitNavigationEvent()
                val scheme = request.url.scheme?.lowercase()
                val isHttpLike = scheme == "http" || scheme == "https"
                if (isHttpLike) {
                    return false
                }
                return true
            }

            override fun onPageStarted(
                view: WebView?,
                url: String?,
                favicon: android.graphics.Bitmap?
            ) {
                super.onPageStarted(view, url, favicon)
                if (url != null) {
                    currentUrl = url
                    addToVisitedUrls(url)
                }

                if (interceptExtensionEnabled) {
                    view?.evaluateJavascript(INTERCEPT_SCRIPT, null)
                }

                injectOverlayScriptsIntoPage(view)
            }

            override fun onPageFinished(view: WebView?, url: String?) {
                super.onPageFinished(view, url)
                if (url != null) {
                    currentUrl = url
                    updateCookiesFromCookieManager(url)
                }

                injectOverlayScriptsIntoPage(view)

                view?.evaluateJavascript("document.documentElement.outerHTML") { rawResult ->
                    if (rawResult != null && rawResult != "null") {
                        htmlBody = unescapeJsString(rawResult)
                        emitNavigationEvent()
                        htmlBody = ""
                    } else {
                        emitNavigationEvent()
                    }
                }
            }

            override fun doUpdateVisitedHistory(
                view: WebView?,
                url: String?,
                isReload: Boolean
            ) {
                super.doUpdateVisitedHistory(view, url, isReload)
                if (url != null) {
                    addToVisitedUrls(url)
                    emitLocationEvent(url)
                }
            }
        }

        // Create a container layout to properly handle the action bar spacing
        val container = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )

            ViewCompat.setOnApplyWindowInsetsListener(this) { view, windowInsets ->
                val insets = windowInsets.getInsets(WindowInsetsCompat.Type.systemBars())
                view.setPadding(0, insets.top, 0, insets.bottom)
                windowInsets
            }
        }

        val webViewLayoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.MATCH_PARENT
        ).apply {
            val actionBarHeight = supportActionBar?.height ?: 0
            if (actionBarHeight == 0) {
                val typedArray =
                    theme.obtainStyledAttributes(intArrayOf(android.R.attr.actionBarSize))
                topMargin = typedArray.getDimensionPixelSize(0, 0)
                typedArray.recycle()
            } else {
                topMargin = actionBarHeight
            }
        }

        webView.layoutParams = webViewLayoutParams
        container.addView(webView)

        setContentView(container)

        val url = intent.getStringExtra("url")!!

        // Inject cookies passed via intent extras
        val cookieUrls = intent.getStringArrayExtra("cookieUrls")
        val cookieValues = intent.getStringArrayExtra("cookieValues")
        if (cookieUrls != null && cookieValues != null) {
            val cookieManager = CookieManager.getInstance()
            for (i in cookieUrls.indices) {
                if (i < cookieValues.size) {
                    cookieManager.setCookie(cookieUrls[i], cookieValues[i])
                }
            }
            cookieManager.flush()
        }

        val headerMap = mutableMapOf<String, String>()
        headers?.keySet()?.forEach { key ->
            if (key != "user-agent") {
                headers.getString(key)?.let { headerMap[key] = it }
            }
        }

        webView.loadUrl(url, headerMap)
    }

    /**
     * Parse CookieManager's cookie string for the given URL and merge into the cookies map.
     */
    private fun updateCookiesFromCookieManager(url: String) {
        try {
            if (!url.startsWith("http://") && !url.startsWith("https://")) return
            val domain = java.net.URL(url).host
            val cookieString = CookieManager.getInstance().getCookie(url) ?: return
            val cookieDict = JSONObject()
            cookieString.split(";").forEach { part ->
                val trimmed = part.trim()
                val eqIdx = trimmed.indexOf('=')
                if (eqIdx > 0) {
                    val name = trimmed.substring(0, eqIdx).trim()
                    val value = trimmed.substring(eqIdx + 1).trim()
                    cookieDict.put(name, value)
                }
            }
            cookies[domain] =
                cookies[domain]?.let { existing ->
                    JsonUtils.mergeJsonObjects(existing, cookieDict)
                } ?: cookieDict
        } catch (e: Exception) {
            Log.e("Opacity SDK", "Error updating cookies from CookieManager", e)
        }
    }

    /**
     * evaluateJavascript wraps its result in quotes and escapes inner characters.
     * This helper reverses that escaping.
     */
    private fun unescapeJsString(raw: String): String {
        // Strip surrounding quotes
        var s = raw
        if (s.startsWith("\"") && s.endsWith("\"")) {
            s = s.substring(1, s.length - 1)
        }
        return s
            .replace("\\\\", "\\")
            .replace("\\\"", "\"")
            .replace("\\/", "/")
            .replace("\\n", "\n")
            .replace("\\r", "\r")
            .replace("\\t", "\t")
            .replace("\\u003C", "<")
            .replace("\\u003E", ">")
            .replace("\\u0026", "&")
            .replace("\\u003c", "<")
            .replace("\\u003e", ">")
            .replace("\\u0026", "&")
    }

    private fun emitInterceptedRequest(requestData: JSONObject) {
        val event: MutableMap<String, Any?> =
            mutableMapOf(
                "event" to "intercepted_request",
                "request_type" to requestData.optString("request_type"),
                "data" to requestData.opt("data"),
                "id" to System.currentTimeMillis().toString()
            )
        val json = JSONObject(event).toString()
        // Log.d("Opacity SDK", "emitInterceptedRequest: ${json.take(300)}")
        OpacityCore.emitWebviewEvent(json)
    }

    private fun emitLocationEvent(url: String) {
        val event: Map<String, Any?> =
            mapOf(
                "event" to "location_changed",
                "url" to url,
                "id" to System.currentTimeMillis().toString()
            )
        OpacityCore.emitWebviewEvent(JSONObject(event).toString())
    }

    private fun emitNavigationEvent() {
        val event: MutableMap<String, Any?> =
            mutableMapOf(
                "event" to "navigation",
                "url" to currentUrl,
                "visited_urls" to visitedUrls,
                "id" to System.currentTimeMillis().toString()
            )

        try {
            val domain = java.net.URL(currentUrl).host
            event["cookies"] = cookies[domain]
        } catch (e: Exception) {
            // If the URL is malformed (usually when it is a URI like "uberlogin://blabla")
            // we don't set any cookies
        }

        if (htmlBody != "") {
            event["html_body"] = htmlBody
        }

        OpacityCore.emitWebviewEvent(JSONObject(event).toString())
        clearVisitedUrls()
    }

    private fun onClose() {
        val event: Map<String, Any> =
            mapOf("event" to "close", "id" to System.currentTimeMillis().toString())
        OpacityCore.emitWebviewEvent(JSONObject(event).toString())
        interceptExtensionEnabled = false
        finish()
    }

    private fun addToVisitedUrls(url: String) {
        if (visitedUrls.isNotEmpty() && visitedUrls.last() == url) {
            return
        }
        visitedUrls.add(url)
    }

    private fun clearVisitedUrls() {
        visitedUrls.clear()
    }

    override fun onDestroy() {
        super.onDestroy()
        val lbm = LocalBroadcastManager.getInstance(this)
        lbm.unregisterReceiver(closeReceiver)
        lbm.unregisterReceiver(cookiesForDomainRequestReceiver)
        lbm.unregisterReceiver(cookiesForCurrentURLRequestReceiver)
        lbm.unregisterReceiver(changeUrlReceiver)

        CookieManager.getInstance().removeAllCookies(null)
        webView.destroy()
        OpacityCore.setActiveWebViewActivity(null)
        OpacityCore.onBrowserDestroyed()
    }

    /**
     * Injects [js] into the standard WebView as an AsyncFunction (mirrors the GeckoView eval
     * extension behaviour). When [fireAndForget] is false the result is delivered back via
     * [OpacityJsBridge.notifyEvalResult] → [OpacityCore.notifyWebViewEvalResult].
     */
    fun dispatchWebViewEval(id: String, js: String, fireAndForget: Boolean) {
        val wrappedJs = if (fireAndForget) {
            "(function(){\n$js\n})()"
        } else {
            val escapedJs = org.json.JSONObject.quote(js)
            val escapedId = org.json.JSONObject.quote(id)
            "(function(){" +
                "var AF=Object.getPrototypeOf(async function(){}).constructor;" +
                "new AF($escapedJs)().then(function(__r){" +
                    "var __val=(__r!==undefined&&__r!==null)?__r:null;" +
                    "OpacityNative.notifyEvalResult($escapedId,JSON.stringify({result:__val}));" +
                "}).catch(function(e){" +
                    "OpacityNative.notifyEvalResult($escapedId,JSON.stringify({error:String(e)}));" +
                "});" +
            "})()"
        }
        Handler(Looper.getMainLooper()).post {
            webView.evaluateJavascript(wrappedJs, null)
        }
    }

    companion object {
        private const val INTERCEPT_SCRIPT = """
(function() {
    const log = (requestType, data) => { try { OpacityNative.onInterceptedRequest(JSON.stringify({ request_type: requestType, data })); } catch(e) {} };

    const nativeToString = Function.prototype.toString;
    const nativeCallToString = Function.prototype.call.bind(nativeToString);
    const wrappedFns = new WeakMap();

    Function.prototype.toString = function() {
        if (wrappedFns.has(this)) {
            return wrappedFns.get(this);
        }
        return nativeCallToString(this);
    };
    wrappedFns.set(Function.prototype.toString, 'function toString() { [native code] }');

    const originalFetch = window.fetch;
    const wrappedFetch = function fetch(input, init) {
        const method = (init && init.method) || (typeof input === 'string' ? 'GET' : input.method || 'GET');
        const url = typeof input === 'string' ? input : input.url;
        if (method.toUpperCase() === 'POST' && init?.body && url.indexOf('/v2/submit-form') !== -1) {
            var bodyStr = typeof init.body === 'string' ? init.body : JSON.stringify(init.body);
            var fullUrl = new URL(url, location.href).href;
            try { OpacityNative.storePostBody(fullUrl, bodyStr); } catch(e) {}
        }
        let requestHeaders = init?.headers || {};
        if (requestHeaders instanceof Headers) requestHeaders = Object.fromEntries(requestHeaders.entries());
        log('fetch_request', { url, method, headers: requestHeaders, body: init?.body });
        return originalFetch.apply(this, arguments).then(function(response) {
            const cloned = response.clone();
            let responseHeaders = cloned.headers || {};
            if (responseHeaders instanceof Headers) responseHeaders = Object.fromEntries(responseHeaders.entries());
            cloned.text().then(function(body) {
                log('fetch_response', { url, method, headers: responseHeaders, body, status: cloned.status });
            });
            return response;
        });
    };
    wrappedFns.set(wrappedFetch, 'function fetch() { [native code] }');
    Object.defineProperty(window, 'fetch', { value: wrappedFetch, writable: true, configurable: true });

    const OriginalXHR = window.XMLHttpRequest;
    const xhrProto = OriginalXHR.prototype;
    const originalOpen = xhrProto.open;
    const originalSend = xhrProto.send;
    const originalSetHeader = xhrProto.setRequestHeader;
    const xhrData = new WeakMap();

    xhrProto.open = function(method, url) {
        xhrData.set(this, { method, url, headers: {} });
        return originalOpen.apply(this, arguments);
    };
    wrappedFns.set(xhrProto.open, 'function open() { [native code] }');

    xhrProto.setRequestHeader = function(name, value) {
        const data = xhrData.get(this);
        if (data) data.headers[name] = value;
        return originalSetHeader.apply(this, arguments);
    };
    wrappedFns.set(xhrProto.setRequestHeader, 'function setRequestHeader() { [native code] }');

    xhrProto.send = function(body) {
        const data = xhrData.get(this);
        if (data) {
            log('xhr_request', { method: data.method, url: data.url, headers: data.headers, body });
            this.addEventListener('loadend', () => {
                log('xhr_response', { method: data.method, url: data.url, headers: data.headers, body: this.responseText || this.response, status: this.status });
            });
        }
        return originalSend.apply(this, arguments);
    };
    wrappedFns.set(xhrProto.send, 'function send() { [native code] }');
})();
"""
    }

}
