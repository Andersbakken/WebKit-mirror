#ifndef Object_h
#define Object_h

#include <JSObjectRef.h>

static inline void Initialize(JSContextRef ctx, JSObjectRef object)
{
    printf("%s:%d void Initialize(JSContextRef ctx, JSObjectRef object)\n", __FILE__, __LINE__);
}
static inline void Finalize(JSObjectRef object)
{
    printf("%s:%d void Finalize(JSObjectRef object)\n", __FILE__, __LINE__);
}

static inline bool HasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
{
    printf("%s:%d bool HasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)\n", __FILE__, __LINE__);
}
static inline JSValueRef GetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    printf("%s:%d JSValueRef GetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)\n", __FILE__, __LINE__);
}

static inline bool SetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    printf("%s:%d bool SetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)\n", __FILE__, __LINE__);
}
static inline bool DeleteProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    printf("%s:%d bool DeleteProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)\n", __FILE__, __LINE__);
}
static inline void GetPropertyNames(JSContextRef ctx, JSObjectRef object, JSPropertyNameAccumulatorRef propertyNames)
{

}
static inline JSValueRef CallAsFunction(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{

}
static inline JSObjectRef CallAsConstructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{

}
static inline bool HasInstance(JSContextRef ctx, JSObjectRef constructor, JSValueRef possibleInstance, JSValueRef* exception)
{

}
static inline JSValueRef ConvertToType(JSContextRef ctx, JSObjectRef object, JSType type, JSValueRef* exception)
{

}

static inline JSClassDefinition classDefinition()
{
    JSClassDefinition ret;
    memset(&ret, 0, sizeof(JSClassDefinition));
    ret.version = 0;
    ret.attributes = kJSClassAttributeNone;
    ret.className = "Test";
    ret.parentClass = 0;
    ret.staticValues = 0;
    ret.staticFunctions = 0;
    ret.initialize = Initialize;
    ret.finalize = Finalize;
    ret.hasProperty = HasProperty;
    ret.getProperty = GetProperty;
    ret.setProperty = SetProperty;
    ret.deleteProperty = DeleteProperty;
    ret.getPropertyNames = GetPropertyNames;
    ret.callAsFunction = CallAsFunction;
    ret.callAsConstructor = CallAsConstructor;
    ret.convertToType = ConvertToType;
    return ret;
}

#endif
