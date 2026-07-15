package com.opacitylabs.opacitycore

class OpacityError(val code: String, override val message: String) : Exception(message)