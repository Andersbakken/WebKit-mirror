#include <config.h>
#include <API/JSContextRef.h>
#include <API/APICast.h>
#include <API/JSStringRef.h>
#include <API/JSValueRef.h>
#include <wtf/MainThread.h>
#include <runtime/InitializeThreading.h>
#include <vector>
#include "Object.h"

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

int main(int argc, char **argv)
{
    JSC::initializeThreading();
    WTF::initializeMainThread();
    JSContextGroupRef group = JSContextGroupCreate();
    JSGlobalContextRef globalCtx = JSGlobalContextCreateInGroup(group, 0);
    JSC::ExecState *exec = toJS(globalCtx);
    JSContextRef ctx = toRef(exec);
    JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx, Str("func"), funcCb);
    JSObjectRef setTimeout = JSObjectMakeFunctionWithCallback(ctx, Str("setTimeout"), ::setTimeout);
    JSObjectRef global = JSContextGetGlobalObject(ctx);
    JSClassDefinition def = classDefinition();
    JSClassRef c = JSClassCreate(&def);
    JSObjectRef test = JSObjectMake(ctx, c, 0);
    JSObjectSetProperty(ctx, global, Str("test"), test, kJSPropertyAttributeNone, NULL);
    JSEvaluateScript(ctx, Str("test.foo = 12;"), 0, 0, 0, 0);
    JSEvaluateScript(ctx, Str("var b = new test();"), 0, 0, 0, 0);

    JSObjectSetProperty(ctx, global, Str("func"), func, kJSPropertyAttributeNone, NULL);
    JSObjectSetProperty(ctx, global, Str("setTimeout"), setTimeout, kJSPropertyAttributeNone, NULL);
    EventLoop loop;
    l = &loop;
    JSValueRef val = JSEvaluateScript(ctx, Str("setTimeout(func, 1000);"), 0, 0, 0, 0);
    loop.exec();
    return 0;
}
