#pragma once
#include <Windows.h>
#include <Wbemidl.h>

// Todo figure out this code more

class EventSink : public IWbemObjectSink
{
    LONG m_lRef;
    bool bDone;

public:
    EventSink() { m_lRef = 0; }
    ~EventSink() { bDone = true; }

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT
        STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

    virtual HRESULT Indicate(LONG lObjectCount, IWbemClassObject** apObjArray);

    virtual HRESULT STDMETHODCALLTYPE SetStatus(
        /* [in] */ LONG lFlags,
        /* [in] */ HRESULT hResult,
        /* [in] */ BSTR strParam,
        /* [in] */ IWbemClassObject __RPC_FAR* pObjParam
    );
};

class WMICommunication {
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    IUnsecuredApartment* pUnsecApp = NULL;
    EventSink* pSink = NULL;
    IUnknown* pStubUnk = NULL;
    IWbemObjectSink* pStubSink = NULL;

private:

public:
    long initializeObjects(const char* query);
    long uninitialize();

    WMICommunication(const char* query);
    ~WMICommunication();
};
