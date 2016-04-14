# mir2x
client, server, tools for cross-platform mir2. Using SDL, FLTK, libzip, etc..

1. pkgviewer
2. shadowmaker
3. mapeditor
4. client
5. monoserver

Try to make all I learned into practice

global variables:

1. don't use global of build-in type or struct since no multithread control.
2. don't use global of class instanse since confusing construction/distruction.

actually:

1. only use class pointer;
2. only reference it by ``extern g_VarXXX";
3. no local function for operation on global variable only, means:
4. all operations over global variables should be self-contained;

Since there have both log system and exception system

1. log system handle all detailed info
2. exception system only throw/catch std::error_code()

The function who throws always think it's a fatal error so it just throw, but how to handle this ``fatal" error or do catch sub-clause really takes it as fatal is decided not by the thrower, but the catcher.

General rules:

1. only throw std::error_code() and log detailed information.
2. for functions which throws, the throwed type should be specified.
3. for constructor, it may throw different type of exceptions.

So if there are constructors in a normal function, we need catch-rethrow if there do exist exceptions, type rethrowed should only be std::error_code()

For server object asynchronized access, we designed ObjectLockGuard to help, but still which can cause dead lock. Put some rule and assumptions here for safe programming:

1. we can only access an object by check out (nUID, nAddTime).
2. if we already have pObject, that means current thread has already grubbed the lock.
3. so, since inside object we always have ``this", all *public* member function of object should be thread safe.
4. if we already have pObject, we constraint usage of (pObject->UID(), pObject->AddTime()), this should only be used for log info output. If we have this (nUID, nAddTime) via pObject, and need to use it the checkout the object again, do it out of the scope of ObjectLockGuard or put the checkout in g_TaskHub/g_EventTaskHub.

sample code:

    void f(CharObject *pObject)
    {
        nUID     = pObject->UID();
        nAddTime = pObject->AddTime();
    
        //...
    
        // horrible, cause dead lock immediately
        auto pGuard = CheckOut<CharObject>(nUID, nAddTime);
        if(pGuard){
            //...
        }
    
        // should be
        auto fnOperate = [nUID, nAddTime, this](){
            auto pGuard = CheckOut<CharObject>(nUID, nAddTime);
            if(pGuard){
                //...
            }
        };
        g_TaskHub->Add(fnOperate);
    }


this should save us from dead lock, or


    void f()
    {
        // ...
        if(auto pGuard = CheckOut<CharObject>(nUID, nAddTime)){
            nUID     = pGuard->UID();
            nAddTime = pGuard->AddTime();

            // g() use naked pointer of object
            g(pGuard.Get());
        }

        // check out again using nUID, nAddTime, it's OK
        auto pGuard = CheckOut<CharObject>(nUID, nAddTime);
        if(pGuard){
            //...
        }
    }

to make it short: ``in one scope there should be only one lock guard of one object". More than 1 lock guard for one object will cause deadlock immediately.

And, when grubbing an object, try your best don't using its UID and AddTime as pramaters to call another function. use the pointer as prameter instead, or put the logic in current funtion.

    void f()
    {
        // ...
        if(auto pGuard = CheckOut<CharObject>(nUID, nAddTime)){
            nUID     = pGuard->UID();
            nAddTime = pGuard->AddTime();
    
            g(nUID, nAddTime);
        }
    }
    
    void g(uint32_t nUID, uint32_t nAddTime)
    {
        // still a terrible problem
        if(auto pGuard = CheckOut<CharObject>(nUID, nAddTime)){
            // ....
        }
    }

If you really really need to do that, put it in the lambda function and call g_TaskHub, otherwise re-design your logic.
