#include "threads.h"
#include <Windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <iostream>
#include <mutex>
#include <vector>

#include <winnt.h>

std::mutex time_write;

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
    SetThreadpoolThreadMaximum(pool, 10);

    bRet = SetThreadpoolThreadMinimum(pool, 5);
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

void EnumerateThreads(HANDLE hProcess)
{
	//DWORD processId = GetProcessId(hProcess);

	//HANDLE hCurrentProcess = GetCurrentProcess();

	//HANDLE hTargetProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
	//if (hTargetProcess == NULL)
	//{
	//	std::cerr << "OpenProcess failed. Error: " << GetLastError() << std::endl;
	//	return;
	//}

	//DWORD_PTR pThreadList[0x1000];
	//DWORD bytesRead;

	//if (!ReadProcessMemory(hTargetProcess, &NtCurrentTeb()->ProcessEnvironmentBlock->ThreadListHead, pThreadList, sizeof(pThreadList), &bytesRead))
	//{
	//	std::cerr << "ReadProcessMemory failed. Error: " << GetLastError() << std::endl;
	//	CloseHandle(hTargetProcess);
	//	return;
	//}

	//DWORD numThreads = bytesRead / sizeof(DWORD_PTR);

	//for (DWORD i = 0; i < numThreads; i++)
	//{
	//	DWORD_PTR pTibAddress;
	//	if (!ReadProcessMemory(hTargetProcess, &pThreadList[i], &pTibAddress, sizeof(DWORD_PTR), &bytesRead))
	//	{
	//		std::cerr << "ReadProcessMemory failed. Error: " << GetLastError() << std::endl;
	//		break;
	//	}

	//	DWORD threadId;
	//	if (!ReadProcessMemory(hTargetProcess, reinterpret_cast<LPCVOID>(pTibAddress + sizeof(DWORD_PTR)), &threadId, sizeof(DWORD), &bytesRead))
	//	{
	//		std::cerr << "ReadProcessMemory failed. Error: " << GetLastError() << std::endl;
	//		break;
	//	}

	//	std::cout << "Thread ID: " << threadId << std::endl;
	//}

	//CloseHandle(hTargetProcess);
	//CloseHandle(hCurrentProcess);
}

std::vector<HANDLE> WinThreadPool::SuspendThreadsInProcess(DWORD processId)
{

    std::vector<HANDLE> thread_handles;
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    // Create a snapshot of the thread list
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
    {
        std::cerr << "CreateToolhelp32Snapshot failed. Error: " << GetLastError() << std::endl;
        return thread_handles;
    }

    // Set the size of the structure before using it
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread in the snapshot
    if (!Thread32First(hThreadSnap, &te32))
    {
        std::cerr << "Thread32First failed. Error: " << GetLastError() << std::endl;
        CloseHandle(hThreadSnap);
        return thread_handles;
    }

	// Iterate through all threads
	do
	{
		if (te32.th32OwnerProcessID == processId)
		{
			// Suspend Thread
			HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
			if (hThread != NULL) {
                Wow64SuspendThread(hThread);
				thread_handles.push_back(hThread);
			}
		}
	} while (Thread32Next(hThreadSnap, &te32));

	// Close the thread snapshot handle
	CloseHandle(hThreadSnap);

	return thread_handles;
}

void WinThreadPool::ResumeThreadsAndClose(const std::vector<HANDLE>& threads) {
    for (const HANDLE& handle : threads) {
        ResumeThread(handle);
        CloseHandle(handle);
    }
}