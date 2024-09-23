package com.opacitylabs.opacitycore

import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.util.Log
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

    @SuppressLint("WrongThread")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val localBroadcastManager = LocalBroadcastManager.getInstance(this)
        localBroadcastManager.registerReceiver(
            closeReceiver,
            IntentFilter("com.opacitylabs.opacitycore.CLOSE_BROWSER")
        )

        supportActionBar?.setDisplayShowTitleEnabled(false)

        val closeButton = Button(this)
        closeButton.text = "Close"
        closeButton.setOnClickListener {
            val json =
                "{\"event\": \"close\", \"id\": \"${System.currentTimeMillis().toString()}\"}"
            OpacityCore.emitWebviewEvent(json)
        }

        val layoutParams =
            ActionBar.LayoutParams(
                ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT,
                Gravity.START or Gravity.CENTER_VERTICAL
            )
        supportActionBar?.setCustomView(closeButton, layoutParams)
        supportActionBar?.setDisplayShowCustomEnabled(true)

        OpacityCore.getRuntime().webExtensionController.installBuiltIn("resource://android/assets/extension/")
            .accept { ext ->
                if (ext != null) {
                    Log.d("InAppBrowserActivity", "WebExtension successfully installed")
                    // Set the MessageDelegate or do further initialization here
                } else {
                    Log.e("InAppBrowserActivity", "Failed to install WebExtension")
                }
                ext?.setMessageDelegate(
                    object : WebExtension.MessageDelegate {
                        override fun onMessage(
                            nativeApp: String,
                            message: Any,
                            sender: WebExtension.MessageSender
                        ): GeckoResult<Any>? {
                            val jsonMessage = message as JSONObject
                            val cookies = jsonMessage.getJSONObject("cookies")
                            browserCookies = JsonUtils.mergeJsonObjects(browserCookies, cookies)
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
                        val url = request.uri
                        handleNavigation(url)
                        return super.onLoadRequest(session, request)
                    }

                    override fun onLocationChange(
                        session: GeckoSession,
                        url: String?,
                        perms: MutableList<GeckoSession.PermissionDelegate.ContentPermission>,
                        hasUserGesture: Boolean
                    ) {
                        if (url != null) {
                            handleNavigation(url)
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

    private fun handleNavigation(url: String) {

        val json =
            "{\"event\": \"navigation\"," +
                    "\"url\": \"$url\", \"cookies\": ${browserCookies}, \"id\": \"${
                        System.currentTimeMillis().toString()
                    }\"}"
        OpacityCore.emitWebviewEvent(json)
    }

    override fun onDestroy() {
        super.onDestroy()
        LocalBroadcastManager.getInstance(this).unregisterReceiver(closeReceiver)
        geckoSession.close()
    }
}
