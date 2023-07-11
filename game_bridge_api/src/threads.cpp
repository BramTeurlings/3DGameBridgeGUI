#include "threads.h"
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <mutex>

std::mutex time_write;

//
// This is the thread pool work callback function.
//
#include <TlHelp32.h>
struct scoped_handle
{
    HANDLE handle;

    scoped_handle() :
        handle(INVALID_HANDLE_VALUE) {}
    scoped_handle(HANDLE handle) :
        handle(handle) {}
    scoped_handle(scoped_handle&& other) :
        handle(other.handle) {
        other.handle = NULL;
    }
    ~scoped_handle() { if (handle != NULL && handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }

    operator HANDLE() const { return handle; }

    HANDLE* operator&() { return &handle; }
    const HANDLE* operator&() const { return &handle; }
};

void WinThreadPool::DefaultCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work)
{
    // Instance, Parameter, and Work not used in this example.
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(parameter);
    UNREFERENCED_PARAMETER(work);

    //
    // Do something when the work callback is invoked.
    //
    DWORD pid = 0;

    const scoped_handle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32W process = { sizeof(process) };
    for (BOOL next = Process32FirstW(snapshot, &process); next; next = Process32NextW(snapshot, &process))
    {
        if (wcscmp(process.szExeFile, L"ULTRAKILL.exe") == 0)
        {
            pid = process.th32ProcessID;
        }
    }

    if(pid)
    {
        std::lock_guard<std::mutex> guard(time_write);
        WinThreadPool::perf_time->a_after = std::chrono::high_resolution_clock::now();
        std::cout << "FOUND Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(WinThreadPool::perf_time->a_after - WinThreadPool::perf_time->a_before).count() << std::endl;
    }
    else
    {
        std::cout << "NOT FOUND\n";
    }
}

WinThreadPool::WinThreadPool()
{
    InitializeThreadpoolEnvironment(&CallBackEnviron);

    //
    // Create a custom, dedicated thread pool.
    //
    pool = CreateThreadpool(NULL);
    if (NULL == pool) {
        std::cout << "CreateThreadpool failed. LastError: " << GetLastError();
        throw std::runtime_error("CreateThreadpool failed. LastError: " + GetLastError());
    }

    //
    // The thread pool is made persistent simply by setting
    // both the minimum and maximum threads to 1.
    //
    SetThreadpoolThreadMaximum(pool, 1);

    bRet = SetThreadpoolThreadMinimum(pool, 1);
    if (FALSE == bRet) {
        std::cout << "SetThreadpoolThreadMinimum failed. LastError: " << GetLastError();
        throw std::runtime_error("SetThreadpoolThreadMinimum failed. LastError: " + GetLastError());
    }

    //
    // Create a cleanup group for this thread pool.
    //
    cleanupgroup = CreateThreadpoolCleanupGroup();
    if (NULL == cleanupgroup) {
        std::cout << "CreateThreadpoolCleanupGroup failed. LastError:" << GetLastError();
        throw std::runtime_error("CreateThreadpoolCleanupGroup failed. LastError: " + GetLastError());
    }

    //
    // Associate the callback environment with our thread pool.
    //
    SetThreadpoolCallbackPool(&CallBackEnviron, pool);

    //
    // Associate the cleanup group with our thread pool.
    // Objects created with the same callback environment
    // as the cleanup group become members of the cleanup group.
    //
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron, cleanupgroup, NULL);
}

WinThreadPool::~WinThreadPool()
{
    //
    // Wait for all callbacks to finish.
    // CloseThreadpoolCleanupGroupMembers also releases objects
    // that are members of the cleanup group, so it is not necessary 
    // to call close functions on individual objects 
    // after calling CloseThreadpoolCleanupGroupMembers.
    //
    CloseThreadpoolCleanupGroupMembers(cleanupgroup, FALSE, NULL);

    // Clean up any individual pieces manually
    // Notice the fall-through structure of the switch.
    // Clean up in reverse order.

    // Clean up the cleanup group members.
    CloseThreadpoolCleanupGroupMembers(cleanupgroup, FALSE, NULL);

    // Clean up the cleanup group.
    CloseThreadpoolCleanupGroup(cleanupgroup);

    // Clean up the pool.
    CloseThreadpool(pool);
}

bool WinThreadPool::StartWork(void(*work_callback)(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work), const PVOID parameter)
{
    //
   // Create work with the callback environment.
   //
    work = CreateThreadpoolWork(work_callback, parameter, &CallBackEnviron);
    if (NULL == work) {
        std::cout << "CreateThreadpoolWork failed. LastError: %u" << GetLastError();
        return false;
    }

    //
    // Submit the work to the pool. Because this was a pre-allocated
    // work item (using CreateThreadpoolWork), it is guaranteed to execute.
    //
    SubmitThreadpoolWork(work);
    return true;
}
