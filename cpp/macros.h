#ifndef macros_h
#define macros_h

#define HOSTFN(name)                                                           \
jsi::Function::createFromHostFunction( \
rt, \
jsi::PropNameID::forAscii(rt, name), \
0, \
[=, this](jsi::Runtime &rt, const jsi::Value &thisValue, const jsi::Value *args, size_t count) -> jsi::Value

#endif /* macros_h */
