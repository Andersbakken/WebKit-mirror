#include <config.h>
#include <API/JSContextRef.h>
#include <API/APICast.h>
#include <API/JSStringRef.h>
#include <API/JSValueRef.h>
#include <wtf/MainThread.h>
#include <runtime/InitializeThreading.h>
#include <vector>
// #include "Object.h"

namespace WTF {
void scheduleDispatchFunctionsOnMainThread() {}
void initializeMainThreadPlatform() {}
};

class Str
{
public:
    Str(const char *str)
        : string(JSStringCreateWithUTF8CString(str))
    {}
    Str(const Str &other)
        : string(JSStringRetain(other.string))
    {}
    ~Str()
    {
        JSStringRelease(string);
    }
    Str &operator=(const Str &other)
    {
        JSStringRelease(string);
        string = JSStringRetain(other.string);
        return *this;
    }

    operator JSStringRef() const { return string; }

private:
    JSStringRef string;
};

class EventLoop
{
public:
    EventLoop()
        : mDone(false)
    {}

    static long long mono()
    {
        enum {
            nsecPerMillisecond = (1000 * 1000),
            msecPerSecond = 1000
        };

        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * msecPerSecond) + (ts.tv_nsec / nsecPerMillisecond);
    }

    void post(long long offset, JSContextRef ctx, JSObjectRef object, JSObjectRef thisObject,
              size_t argumentCount, const JSValueRef arguments[])
    {
        // printf("%s:%d size_t argumentCount, const JSValueRef arguments[])\n", __FILE__, __LINE__);
        Call *c = new Call(mono() + offset, ctx, object, thisObject, argumentCount, arguments);
        for (int i=0; i<argumentCount; ++i) {
            JSValueProtect(ctx, arguments[i]);
        }

        mCalls.push_back(c);
    }

    void quit()
    {
        mDone = true;
    }
    void exec()
    {
        while (!mDone) {
            const long long now = mono();
            long long next = -1;
            for (int i=mCalls.size() - 1; i>=0; --i) {
                const Call *c = mCalls[i];
                if (c->time <= now) {
                    JSValueRef ret = JSObjectCallAsFunction(c->ctx, c->object, c->thisObject, c->argumentCount, c->arguments, 0);
                    int s = mCalls.size();
                    mCalls.erase(mCalls.begin() + i);
                    for (int i=0; i<c->argumentCount; ++i) {
                        JSValueUnprotect(c->ctx, c->arguments[i]);
                    }
                    
                    delete c;
                } else if (next == -1 || c->time < next) {
                    next = c->time;
                }
            }
            if (next == -1) {
                usleep(100000);
            } else {
                usleep((next - now) * 1000);
            }
        }
    }
private:
    struct Call {
        Call(long long tm, JSContextRef c, JSObjectRef o,
             JSObjectRef t, size_t a, const JSValueRef *args)
            : time(tm), ctx(c), object(o), thisObject(t),
              argumentCount(a), arguments(a ? new JSValueRef[a] : 0)
        {
            memcpy(arguments, args, sizeof(JSValueRef) * argumentCount);
        }
        ~Call()
        {
            delete[] arguments;
        }

        long long time;
        JSContextRef ctx;
        JSObjectRef object;
        JSObjectRef thisObject;
        size_t argumentCount;
        JSValueRef *arguments;
    };

    std::vector<Call*> mCalls;
    bool mDone;
};

EventLoop *l = 0;

static JSValueRef funcCb(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                         const JSValueRef arguments[], JSValueRef* exception)
{
    printf("func() called\n");
    return JSValueMakeNumber(ctx, 5);
}

static JSValueRef setTimeout(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                             const JSValueRef arguments[], JSValueRef* exception)
{
    // arguments[2] should be optional args for callbac

    if (argumentCount != 2
        || !JSValueIsNumber(ctx, arguments[1])
        || !JSValueIsObject(ctx, arguments[0])
        || !JSObjectIsFunction(ctx, JSValueToObject(ctx, arguments[0], exception))) {
        *exception = JSValueMakeString(ctx, Str("Invalid argument count"));
        return JSValueMakeUndefined(ctx);
    }
    l->post(int(JSValueToNumber(ctx, arguments[1], exception)),
            ctx, JSValueToObject(ctx, arguments[0], exception),
            thisObject, argumentCount, arguments);
    return JSValueMakeUndefined(ctx);
}

static JSValueRef print(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                        const JSValueRef arguments[], JSValueRef* exception)
{
    for (int i=0; i<argumentCount; ++i) {
        JSStringRef json = JSValueCreateJSONString(ctx, arguments[i], 2, 0);
        const int max = JSStringGetMaximumUTF8CStringSize(json);
        char *buf = new char[max];
        JSStringGetUTF8CString(json, buf, max);
        printf("%s%c", buf, i + 1 == argumentCount ? '\n' : ' ');
        delete[] buf;
    }
    return JSValueMakeUndefined(ctx);
}


class Obj;
class Func
{
public:
    Func(Obj *o, const char *n)
        : object(o), name(n)
    {}

    Obj *object;
    const char *name;
};

class Obj
{
public:
    static JSValueRef CallAsFunction(JSContextRef ctx, JSObjectRef function,
                                     JSObjectRef thisObject, size_t argumentCount,
                                     const JSValueRef arguments[], JSValueRef* exception)
    {
        const char *name = static_cast<const char*>(JSObjectGetPrivate(function));
        Obj *o = static_cast<Obj*>(JSObjectGetPrivate(thisObject));
        if (name && o) {
            const char **f = o->functions();
            for (int i=0; f[i]; ++i) {
                if (!strcmp(name, f[i])) {
                    return o->invoke(ctx, name, argumentCount, arguments);
                }
            }
        }
        return JSValueMakeUndefined(ctx);
    }

    static JSValueRef GetProperty(JSContextRef ctx, JSObjectRef object,
                                  JSStringRef propertyName, JSValueRef* exception)
    {
        Obj *o = static_cast<Obj*>(JSObjectGetPrivate(object));
        const char **props = o->properties();
        for (int i=0; props[i]; ++i) {
            if (JSStringIsEqualToUTF8CString(propertyName, props[i])) {
                return JSValueMakeNumber(ctx, i);
            }
        }
        const char **f = o->functions();

        for (int i=0; f[i]; ++i) {
            if (JSStringIsEqualToUTF8CString(propertyName, f[i])) {
                const JSClassDefinition def = methodClassDefintion();
                JSClassRef c = JSClassCreate(&def); // cache these
                const void *functionName = f[i];
                JSObjectRef ret = JSObjectMake(ctx, c, const void*>((f[i]))));
                JSClassRelease(c);
                JSC::ExecState *exec = toJS(ctx);
                return toRef(exec, toJS(exec, ret));
            }
        }

        return JSValueMakeUndefined(ctx);
    }
    static bool HasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
    {
        Obj *o = static_cast<Obj*>(JSObjectGetPrivate(object));
        const char **props = o->properties();
        for (int i=0; props[i]; ++i) {
            if (JSStringIsEqualToUTF8CString(propertyName, props[i]))
                return true;
        }
        const char **funcs = o->functions();
        for (int i=0; funcs[i]; ++i) {
            if (JSStringIsEqualToUTF8CString(propertyName, funcs[i])) {
                return true;
            }
        }
        
        return false;
    }

    static void Initialize(JSContextRef ctx, JSObjectRef object)
    {
        printf("%s:%d static void Initialize(JSContextRef ctx, JSObjectRef object)\n", __FILE__, __LINE__);
    }


    static void GetPropertyNames(JSContextRef ctx, JSObjectRef object, JSPropertyNameAccumulatorRef propertyNames)
    {
        printf("GetPropertyNames\n");
        printf("%s:%d static void GetPropertyNames(JSContextRef ctx, JSObjectRef object, JSPropertyNameAccumulatorRef propertyNames)\n", __FILE__, __LINE__);
        Obj *o = static_cast<Obj*>(JSObjectGetPrivate(object));
        const char **props = o->properties();
        for (int i=0; props[i]; ++i) {
            JSPropertyNameAccumulatorAddName(propertyNames, Str(props[i]));
        }
    }

    static JSObjectRef CallAsConstructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
    {
        printf("%s:%d static JSObjectRef CallAsConstructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)\n", __FILE__, __LINE__);
        const JSClassDefinition def = classDefinition();
        JSClassRef c = JSClassCreate(&def);
        JSObjectRef ret = JSObjectMake(ctx, c, new Obj);
        JSClassRelease(c);
        return ret;
    }

    static inline void Finalize(JSObjectRef object)
    {
        printf("%s:%d static inline void Finalize(JSObjectRef object)\n", __FILE__, __LINE__);
        delete static_cast<Obj*>(JSObjectGetPrivate(object));
    }

    static JSClassDefinition classDefinition()
    {
        JSClassDefinition ret = kJSClassDefinitionEmpty;
        ret.className = "Obj";
        ret.hasProperty = Obj::HasProperty;
        ret.getProperty = Obj::GetProperty;
        ret.getPropertyNames = Obj::GetPropertyNames;
        ret.callAsConstructor = Obj::CallAsConstructor;
        ret.initialize = Obj::Initialize;
        ret.finalize = Obj::Finalize;
        return ret;
    }

    static JSClassDefinition methodClassDefintion()
    {
        JSClassDefinition ret = kJSClassDefinitionEmpty;
        ret.callAsFunction = Obj::CallAsFunction;
        return ret;
    }

    const char **properties() const
    {
        static const char *properties[] = { "prop1", "prop2", 0 };
        return properties;
    }

    const char **functions() const
    {
        static const char *functions[] = { "func1", "func2", 0 };
        return functions;
    }

    virtual JSValueRef invoke(JSContextRef ctx, const char *name,
                              size_t argumentCount, const JSValueRef arguments[])
    {
        printf("Calling %s\n", name);
        return argumentCount ? arguments[0] : JSValueMakeUndefined(ctx);
    }

    virtual bool getProperty(JSContextRef ctx, const char *name, JSValueRef *val) const
    {
        if (!strncmp(name, "x")) {
            *val = JSValueMakeNumber(ctx, 12);
            return true;
        }
        *val = JSValueMakeUndefined(ctx);
        return false;
    }

};

int main(int argc, char **argv)
{
    JSC::initializeThreading();
    WTF::initializeMainThread();
    JSContextGroupRef group = JSContextGroupCreate();
    JSGlobalContextRef globalCtx = JSGlobalContextCreateInGroup(group, 0);
    JSC::ExecState *exec = toJS(globalCtx);
    JSContextRef ctx = toRef(exec);
    // JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx, Str("func"), funcCb);
    // JSObjectRef setTimeout = JSObjectMakeFunctionWithCallback(ctx, Str("setTimeout"), ::setTimeout);
    JSObjectRef print = JSObjectMakeFunctionWithCallback(ctx, Str("print"), ::print);
    JSObjectRef global = JSContextGetGlobalObject(ctx);
    // JSEvaluateScript(ctx, Str("b.foo(12);"), 0, 0, 0, 0);

    // JSEvaluateScript(ctx, Str("Obj();"), 0, 0, 0, 0);
    JSClassDefinition def = Obj::classDefinition();
    JSClassRef c = JSClassCreate(&def);
    JSObjectRef test = JSObjectMake(ctx, c, 0);
    JSObjectSetProperty(ctx, global, Str("Obj"), test, kJSPropertyAttributeNone, NULL);
    JSObjectSetProperty(ctx, global, Str("print"), print, kJSPropertyAttributeNone, NULL);

    const char *script =
        "var f = new Obj();\n"
        "f.foo = 12;";
    // "var ff = new test;\n"
    // "if (ff.balle == f['t'])\n"
    // "    delete ff.balle;\n";
    // "for (var p in f) {}\n;";
    // "var b = f.prop1;";
    // JSEvaluateScript(ctx, Str("test();"), 0, 0, 0, 0);
    JSEvaluateScript(ctx, Str(argc > 1 ? argv[1] : script), 0, 0, 0, 0);

    // JSEvaluateScript(ctx, Str("test.foo = 12;"), 0, 0, 0, 0);
    // JSEvaluateScript(ctx, Str("var b = new test();"), 0, 0, 0, 0);
    // JSEvaluateScript(ctx, Str("b.foo(12);"), 0, 0, 0, 0);


    // JSObjectSetProperty(ctx, global, Str("func"), func, kJSPropertyAttributeNone, NULL);
    // JSObjectSetProperty(ctx, global, Str("setTimeout"), setTimeout, kJSPropertyAttributeNone, NULL);
    // EventLoop loop;
    // l = &loop;
    // JSValueRef val = JSEvaluateScript(ctx, Str("setTimeout(func, 1000);"), 0, 0, 0, 0);
    // loop.exec();
    JSGarbageCollect(ctx);
    return 0;
}
