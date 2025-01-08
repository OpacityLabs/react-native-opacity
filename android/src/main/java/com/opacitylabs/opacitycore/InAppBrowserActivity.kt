package com.opacitylabs.opacitycore

import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.view.Gravity
import android.widget.Button
import androidx.appcompat.app.ActionBar
import androidx.appcompat.app.AppCompatActivity
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import org.json.JSONObject
import org.mozilla.geckoview.AllowOrDeny
import org.mozilla.geckoview.GeckoResult
import org.mozilla.geckoview.GeckoSession
import org.mozilla.geckoview.GeckoSession.ContentDelegate
import org.mozilla.geckoview.GeckoView
import org.mozilla.geckoview.WebExtension

class InAppBrowserActivity : AppCompatActivity() {
    private val closeReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            if (intent?.action == "com.opacitylabs.opacitycore.CLOSE_BROWSER") {
                finish()
            }
        }
    }

    private lateinit var geckoSession: GeckoSession
    private lateinit var geckoView: GeckoView
    private var browserCookies: JSONObject = JSONObject()
    private var htmlBody: String = ""
    private var currentUrl: String = ""
    private val visitedUrls = mutableListOf<String>()

    @SuppressLint("WrongThread")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val localBroadcastManager = LocalBroadcastManager.getInstance(this)
        localBroadcastManager.registerReceiver(
            closeReceiver,
            IntentFilter("com.opacitylabs.opacitycore.CLOSE_BROWSER")
        )

        supportActionBar?.setDisplayShowTitleEnabled(false)

        val closeButton = Button(this, null, android.R.attr.buttonStyleSmall).apply {
            text = "✕"
            textSize = 18f
            setBackgroundColor(android.graphics.Color.TRANSPARENT)
            setOnClickListener {
            onClose()
            }
        }

        val layoutParams =
            ActionBar.LayoutParams(
            ActionBar.LayoutParams.WRAP_CONTENT,
            ActionBar.LayoutParams.WRAP_CONTENT,
            Gravity.END or Gravity.CENTER_VERTICAL
            )
        supportActionBar?.setCustomView(closeButton, layoutParams)
        supportActionBar?.setDisplayShowCustomEnabled(true)

        OpacityCore.getRuntime().webExtensionController.installBuiltIn("resource://android/assets/extension/")
            .accept { ext ->
                ext?.setMessageDelegate(
                    object : WebExtension.MessageDelegate {
                        override fun onMessage(
                            nativeApp: String,
                            message: Any,
                            sender: WebExtension.MessageSender
                        ): GeckoResult<Any>? {
                            val jsonMessage = message as JSONObject

                            when (jsonMessage.getString("event")) {
                                "html_body" -> {
                                    htmlBody = jsonMessage.getString("html")
                                    emitNavigationEvent()
                                }

                                "cookies" -> {
                                    val cookies = jsonMessage.getJSONObject("cookies")
                                    browserCookies =
                                        JsonUtils.mergeJsonObjects(browserCookies, cookies)
                                }
                                else -> {
                                    // Intentionally left blank
                                    // Log.d("Background Script Event", "${jsonMessage.getString("event")}")
                                }
                            }

                            return super.onMessage(nativeApp, message, sender)
                        }

                    }, "gecko"
                )

                geckoSession = GeckoSession().apply {
                    setContentDelegate(object : ContentDelegate {})
                    open(OpacityCore.getRuntime())
                }

                geckoSession.settings.apply {
                    allowJavascript = true
                }

                geckoSession.navigationDelegate = object : GeckoSession.NavigationDelegate {
                    override fun onLoadRequest(
                        session: GeckoSession,
                        request: GeckoSession.NavigationDelegate.LoadRequest
                    ): GeckoResult<AllowOrDeny>? {
                        currentUrl = request.uri
                        addToVisitedUrls(request.uri)
                        return super.onLoadRequest(session, request)
                    }

                    override fun onLocationChange(
                        session: GeckoSession,
                        url: String?,
                        perms: MutableList<GeckoSession.PermissionDelegate.ContentPermission>,
                        hasUserGesture: Boolean
                    ) {
                        if (url != null) {
                            currentUrl = url
                            addToVisitedUrls(url)
                        }
                    }
                }

                geckoView = GeckoView(this).apply {
                    setSession(geckoSession)
                }

                setContentView(geckoView)
                val url = intent.getStringExtra("url")!!

                geckoSession.loadUri(url)
            }
    }

    private fun emitNavigationEvent() {
            val event: Map<String, Any> = mapOf(
                "event" to "navigation",
                "url" to currentUrl,
                "html_body" to htmlBody,
                "cookies" to browserCookies,
                "visited_urls" to visitedUrls,
                "id" to System.currentTimeMillis().toString()
            )
            val stringifiedObj = JSONObject(event).toString()
            OpacityCore.emitWebviewEvent(stringifiedObj)
            clearVisitedUrls()
    }

    private fun onClose() {
        val event: Map<String, Any> = mapOf(
            "event" to "close",
            "id" to System.currentTimeMillis().toString()
        )
        val stringifiedObj = JSONObject(event).toString()
        OpacityCore.emitWebviewEvent(stringifiedObj)
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
        LocalBroadcastManager.getInstance(this).unregisterReceiver(closeReceiver)
        geckoSession.close()
    }
}