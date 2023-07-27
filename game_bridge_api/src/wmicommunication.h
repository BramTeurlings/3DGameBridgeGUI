#pragma once
#define _WIN32_DCOM
#include <Wbemidl.h>

#include <string>
#include <queue>
#include "threads.h"
#include <TlHelp32.h>

// Todo figure out this code more

#include <mutex>
#include <condition_variable>

#ifdef GAME_BRIDGE_API_EXPORTS
#define GAME_BRIDGE_API __declspec(dllexport)
#else
#define GAME_BRIDGE_API __declspec(dllimport)
#endif

GAME_BRIDGE_API void SetIndicateEventCallback(void(*work_callback)(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work));
GAME_BRIDGE_API void WmiSearchCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work);
GAME_BRIDGE_API void ProcessEnumerationCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work);

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

struct ProcessDetectionData
{
    std::vector <std::string> supported_titles;
    std::vector <std::string> config_paths;
    std::vector <std::string> preset_paths;
    std::vector<uint32_t> game_fixes;
    std::chrono::high_resolution_clock::time_point pr_start_tm; //process launch time point
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

    void InitializePayload(const std::string& sr_binary_path);
    void SetReshadeConfigPath(const std::string& reshade_config_path);
    long InitializeObjects(const char* query);
    long Deinitialize();

    static void SetDetectionData(const ProcessDetectionData& val);
    static ProcessDetectionData& GetDetectionData();

    WMICommunication(const char* query);
    ~WMICommunication();
};
