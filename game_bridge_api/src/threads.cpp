#include "threads.h"
#include <windows.h>
#include <tchar.h>
#include <iostream>

//
// This is the thread pool work callback function.
//
void WinThreadPool::DefaultCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work)
{
    // Instance, Parameter, and Work not used in this example.
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(parameter);
    UNREFERENCED_PARAMETER(work);

    //
    // Do something when the work callback is invoked.
    //
    {
        std::cout << "MyWorkCallback: Task performed.\n";
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
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron, cleanupgroup,NULL);
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

bool WinThreadPool::StartWork(void(*work_callback)(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work),const PVOID parameter)
{
    //
   // Create work with the callback environment.
   //
    work = CreateThreadpoolWork(work_callback, parameter, &CallBackEnviron);
    if (NULL == work) {
        std::cout << "CreateThreadpoolWork failed. LastError: %u" << GetLastError();
    }
    else  
    {
        return false;
    }

    //
    // Submit the work to the pool. Because this was a pre-allocated
    // work item (using CreateThreadpoolWork), it is guaranteed to execute.
    //
    SubmitThreadpoolWork(work);
    return true;
}
