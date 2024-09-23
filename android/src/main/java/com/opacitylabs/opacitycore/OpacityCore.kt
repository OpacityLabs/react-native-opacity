package com.opacitylabs.opacitycore

import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.util.Log
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.mozilla.geckoview.GeckoRuntime

object OpacityCore {

    private lateinit var appContext: Context
    private lateinit var cryptoManager: CryptoManager
    private lateinit var _url: String
    private var headers: Bundle = Bundle()
    private lateinit var sRuntime: GeckoRuntime

    fun initialize(context: Context): Int {
        appContext = context
        sRuntime = GeckoRuntime.create(context.applicationContext)
        cryptoManager = CryptoManager(appContext.applicationContext)
        return init()
    }

    fun getRuntime(): GeckoRuntime {
        if (!::sRuntime.isInitialized) {
            throw IllegalStateException("GeckoRuntime is not initialized. Call initialize() first.")
        }
        return sRuntime
    }

    fun securelySet(key: String, value: String) {
        cryptoManager.set(key, value)
    }

    fun securelyGet(key: String): String? {
        return cryptoManager.get(key)
    }

    fun prepareInAppBrowser(url: String) {
        headers = Bundle()
        _url = url
    }

    fun setBrowserHeader(key: String, value: String) {
        headers.putString(key, value)
    }

    fun presentBrowser() {
        val intent = Intent()
        intent.setClassName(
                appContext.packageName,
                "com.opacitylabs.opacitycore.InAppBrowserActivity"
        )
        intent.putExtra("url", _url)
        intent.putExtra("headers", headers)
        appContext.startActivity(intent)
    }

    fun closeBrowser() {
        val closeIntent = Intent("com.opacitylabs.opacitycore.CLOSE_BROWSER")
        LocalBroadcastManager.getInstance(appContext).sendBroadcast(closeIntent)
        Log.d("OpacityCore", "Intent dispatched")
    }

    private external fun init(): Int

}
