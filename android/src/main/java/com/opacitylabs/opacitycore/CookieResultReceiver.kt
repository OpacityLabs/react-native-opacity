package com.opacitylabs.opacitycore


import android.os.Parcel
import android.os.Parcelable
import org.json.JSONObject
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit


class CookieResultReceiver() : Parcelable {
    private var cookies: JSONObject? = null
    private val latch = CountDownLatch(1)

    constructor(parcel: Parcel) : this()

    fun onReceiveResult(cookies: JSONObject?) {
        this.cookies = cookies
        latch.countDown()
    }

    fun waitForResult(timeoutMs: Long): JSONObject? {
        latch.await(timeoutMs, TimeUnit.MILLISECONDS)
        return cookies
    }

    override fun writeToParcel(parcel: Parcel, flags: Int) {}

    override fun describeContents(): Int = 0

    companion object CREATOR : Parcelable.Creator<CookieResultReceiver> {
        override fun createFromParcel(parcel: Parcel): CookieResultReceiver {
            return CookieResultReceiver(parcel)
        }

        override fun newArray(size: Int): Array<CookieResultReceiver?> {
            return arrayOfNulls(size)
        }
    }
}