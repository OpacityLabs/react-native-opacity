package com.opacitylabs.opacitycore

import android.content.Context
import android.content.SharedPreferences
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey

class CryptoManager(context: Context) {
    private val masterKey: MasterKey

    private val encryptedPrefs: SharedPreferences

    init {
        // Create a regular master key and a biometrics secured master key
        masterKey = generateMasterKey(context)

        // Create a regular encryptedSharedPreferences and a biometrics one
        encryptedPrefs = EncryptedSharedPreferences.create(
            context,
            SHARED_PREFS_FILENAME,
            masterKey,
            EncryptedSharedPreferences.PrefKeyEncryptionScheme.AES256_SIV,
            EncryptedSharedPreferences.PrefValueEncryptionScheme.AES256_GCM
        )
    }

    //    By Choosing AES256_GCM the default settings for the key are
    //    KeyGenParameterSpec.Builder(
    //            mKeyAlias,
    //            KeyProperties.PURPOSE_ENCRYPT | KeyProperties.PURPOSE_DECRYPT)
    //            .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
    //            .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
    //            .setKeySize(DEFAULT_AES_GCM_MASTER_KEY_SIZE)
    private fun generateMasterKey(context: Context): MasterKey {

        return MasterKey
            .Builder(context)
            .setKeyScheme(MasterKey.KeyScheme.AES256_GCM)
            .setUserAuthenticationRequired(false, 10)
            .build()
    }

    fun set(key: String, value: String) {
        encryptedPrefs.edit().putString(key, value).apply()
    }

    fun get(key: String): String? {
        return encryptedPrefs.getString(key, null)
    }

    fun delete(key: String) {
        encryptedPrefs.edit().putString(key, null).apply()
    }

    companion object {
        private const val SHARED_PREFS_FILENAME = "OpacityCoreEncryptedSharedPrefs"
    }
}
