package com.opacitylabs.opacitycore

import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonNull
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.boolean
import kotlinx.serialization.json.booleanOrNull
import kotlinx.serialization.json.double
import kotlinx.serialization.json.doubleOrNull
import kotlinx.serialization.json.int
import kotlinx.serialization.json.intOrNull

class JsonConverter {
    companion object {
        fun mapToJsonElement(map: Map<String, Any?>): JsonElement {
            fun toJsonElement(value: Any?): JsonElement = when (value) {
                null -> JsonNull
                is Boolean -> JsonPrimitive(value)
                is Number -> JsonPrimitive(value)
                is String -> JsonPrimitive(value)
                is Map<*, *> -> {
                    @Suppress("UNCHECKED_CAST")
                    JsonObject((value as Map<String, Any?>).mapValues { toJsonElement(it.value) })
                }

                is List<*> -> JsonArray(value.map { toJsonElement(it) })
                else -> throw IllegalArgumentException("Unsupported type: ${value::class}")
            }

            return JsonObject(map.mapValues { toJsonElement(it.value) })
        }
        
        fun parseJsonElementToAny(jsonElement: JsonElement): Any? {
            return when (jsonElement) {
                is JsonObject -> {
                    jsonElement.toMap().mapValues { parseJsonElementToAny(it.value) }
                }

                is JsonArray -> {
                    jsonElement.map { parseJsonElementToAny(it) }
                }

                is JsonNull -> null

                is JsonPrimitive -> {
                    when {
                        jsonElement.isString -> jsonElement.content
                        jsonElement.intOrNull != null -> jsonElement.int
                        jsonElement.booleanOrNull != null -> jsonElement.boolean
                        jsonElement.doubleOrNull != null -> jsonElement.double
                        else -> throw Exception("Could not convert JSON primitive $jsonElement")
                    }
                }

                else -> throw Exception("Could not convert JSON primitive: $jsonElement")
            }
        }
    }
}