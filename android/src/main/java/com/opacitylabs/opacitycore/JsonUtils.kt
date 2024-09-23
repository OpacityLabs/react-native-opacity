package com.opacitylabs.opacitycore

import org.json.JSONObject

object JsonUtils {
    fun mergeJsonObjects(json1: JSONObject, json2: JSONObject): JSONObject {
        val mergedJson = JSONObject()

        // Add all keys from json1
        json1.keys().forEach { key ->
            mergedJson.put(key, json1.get(key))
        }

        // Add or merge keys from json2
        json2.keys().forEach { key ->
            if (mergedJson.has(key)) {
                // Merge if both values are JSONObjects
                val value1 = mergedJson.get(key)
                val value2 = json2.get(key)

                if (value1 is JSONObject && value2 is JSONObject) {
                    mergedJson.put(key, mergeJsonObjects(value1, value2))
                } else {
                    // Otherwise, just overwrite the value
                    mergedJson.put(key, value2)
                }
            } else {
                // Add key from json2 if it's not present in json1
                mergedJson.put(key, json2.get(key))
            }
        }

        return mergedJson
    }
}