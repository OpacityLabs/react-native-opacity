package com.opacitylabs.opacitycore

data class OpacityResponse(
    val status: Int,
    val json: String?,
    val proof: String?,
    val err: String?
)
