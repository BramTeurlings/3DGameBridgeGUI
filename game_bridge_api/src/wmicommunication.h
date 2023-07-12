#pragma once
#define _WIN32_DCOM
#include <Wbemidl.h>

#include <string>
#include <queue>
#include "threads.h"

// Todo figure out this code more

#include <mutex>
#include <condition_variable>

#ifdef GAME_BRIDGE_API_EXPORTS
#define GAME_BRIDGE_API __declspec(dllexport)
#else
#define GAME_BRIDGE_API __declspec(dllimport)
#endif

void SetIndicateEventCallback(void(*work_callback)(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work));

class Semaphore {
public:
    Semaphore(int count_ = 0)
        : count(count_) {}

    inline void notify()
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }

    inline void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);

        while (count == 0) {
            cv.wait(lock);
        }
        count--;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};

struct Win32ProcessData
{
    uint32_t pid;
    std::string executable_path;
};

class GAME_BRIDGE_API EventSink : public IWbemObjectSink
{
    LONG m_lRef;
    bool bDone;

public:
    // Todo move these sometime
    Semaphore semaphore_message_queue;
    std::queue<Win32ProcessData> message_queue;

    EventSink() { m_lRef = 0; }
    ~EventSink() { bDone = true; }

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

    virtual HRESULT Indicate(LONG lObjectCount, IWbemClassObject** apObjArray);

    virtual HRESULT STDMETHODCALLTYPE SetStatus(
        /* [in] */ LONG lFlags,
        /* [in] */ HRESULT hResult,
        /* [in] */ BSTR strParam,
        /* [in] */ IWbemClassObject __RPC_FAR* pObjParam
    );
};

class GAME_BRIDGE_API WMICommunication {
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    IUnsecuredApartment* pUnsecApp = NULL;
    IUnknown* pStubUnk = NULL;
    IWbemObjectSink* pStubSink = NULL;

public:
    // Todo make this less bad
    EventSink* pSink = NULL;

    long InitializeObjects(const char* query);
    long Deinitialize();
    void SetTime(const TimeMeasurements& val);
    TimeMeasurements& GetTime();

    WMICommunication(const char* query);
    ~WMICommunication();
};
