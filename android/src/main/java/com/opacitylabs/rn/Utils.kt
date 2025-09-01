package com.opacitylabs.rn

import com.facebook.react.bridge.Arguments
import com.facebook.react.bridge.WritableArray
import com.facebook.react.bridge.WritableMap

fun mapToWritableMap(map: Map<String, Any?>): WritableMap {
    val writableMap = Arguments.createMap()
    for ((key, value) in map) {
        when (value) {
            is String -> writableMap.putString(key, value)
            is Int -> writableMap.putInt(key, value)
            is Double -> writableMap.putDouble(key, value)
            is Boolean -> writableMap.putBoolean(key, value)
            is Map<*, *> -> writableMap.putMap(key, mapToWritableMap(value as Map<String, Any?>))
            is List<*> -> writableMap.putArray(key, listToWritableArray(value))
            null -> writableMap.putNull(key)
            else -> throw IllegalArgumentException("Unsupported type for key: $key value: $value")
        }
    }
    return writableMap
}

fun listToWritableArray(list: List<*>): WritableArray {
    val writableArray = Arguments.createArray()
    for (item in list) {
        when (item) {
            is String -> writableArray.pushString(item)
            is Int -> writableArray.pushInt(item)
            is Double -> writableArray.pushDouble(item)
            is Boolean -> writableArray.pushBoolean(item)
            is Map<*, *> -> writableArray.pushMap(mapToWritableMap(item as Map<String, Any?>))
            is List<*> -> writableArray.pushArray(listToWritableArray(item))
            null -> writableArray.pushNull()
            else -> throw IllegalArgumentException("Unsupported list item type")
        }
    }
    return writableArray
}
