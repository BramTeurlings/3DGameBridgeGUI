#pragma once

/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 * Modified by srReshade devs
 */

#include <stdio.h>
#include <Windows.h>
#include <sddl.h>
#include <AclAPI.h>
#include <TlHelp32.h>

#include <string>

#include "logger.h"

#define RESHADE_LOADING_THREAD_FUNC 1
#define NUM_DLLS 9
#define SR_REGISTRY_PATH 

struct loading_data
{
	WCHAR load_path[NUM_DLLS][MAX_PATH] = { L"" };
	decltype(&GetLastError) GetLastError = nullptr;
	decltype(&LoadLibraryW) LoadLibraryW = nullptr;
	const WCHAR env_var_name[30] = L"RESHADE_DISABLE_LOADING_CHECK";
	const WCHAR env_var_value[2] = L"1";
	decltype(&SetEnvironmentVariableW) SetEnvironmentVariableW = nullptr;
};

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

static void update_acl_for_uwp(LPWSTR path);

#if RESHADE_LOADING_THREAD_FUNC
static DWORD WINAPI loading_thread_func(loading_data* arg)
{
	arg->SetEnvironmentVariableW(arg->env_var_name, arg->env_var_value);
	int err = 0;
	for (int i = 0; i < NUM_DLLS; i++) {
		if (arg->LoadLibraryW(arg->load_path[i]) == NULL) {
			err = arg->GetLastError();
			return i;
		}
	}
	return err;
}
#endif

int CreatePayload(const std::string& sr_binary_path, loading_data& data, bool use_32_bit = false);
int GetPID(std::string process_name);

inline int InjectIntoApplication(uint32_t pid, const loading_data& payload, uint32_t sleep_time = 50)
{
	///////////////////
	// Wait just a little bit for the application to initialize
	// Sleep(sleep_time);

	// Open target application process
	const scoped_handle remote_process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (remote_process == nullptr)
	{
		wprintf(L"\nFailed to open target application process!\n");
		return GetLastError();
	}

	//Todo You might need a 32 bits exe to inject into a 32 bits app
	// Prepare two payloads, one for 32 and 64 bit, select the right one here.
	// Check process architecture
	BOOL remote_is_wow64 = FALSE;
	IsWow64Process(remote_process, &remote_is_wow64);
#ifndef _WIN64
	if (remote_is_wow64 == FALSE)
#else
	if (remote_is_wow64 != FALSE)
#endif
	{
		wprintf(L"\nProcess architecture does not match injection tool! Cannot continue.\n");
		return ERROR_IMAGE_MACHINE_TYPE_MISMATCH;
	}

#if RESHADE_LOADING_THREAD_FUNC
	const auto loading_thread_func_size = 256; // An estimate of the size of the 'loading_thread_func' function
#else
	const auto loading_thread_func_size = 0;
#endif
	const auto load_param = VirtualAllocEx(remote_process, nullptr, loading_thread_func_size + sizeof(payload), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#if RESHADE_LOADING_THREAD_FUNC
	const auto loading_thread_func_address = static_cast<LPBYTE>(load_param) + sizeof(payload);
#else
	const auto loading_thread_func_address = arg.LoadLibraryW;
#endif

	// Write thread entry point function and 'LoadLibrary' call argument to target application
	if (load_param == nullptr
		|| !WriteProcessMemory(remote_process, load_param, &payload, sizeof(payload), nullptr)
#if RESHADE_LOADING_THREAD_FUNC
		|| !WriteProcessMemory(remote_process, loading_thread_func_address, loading_thread_func, loading_thread_func_size, nullptr)
#endif
		)
	{
		wprintf(L"\nFailed to allocate and write 'LoadLibrary' argument in target application!\n");
		return GetLastError();
	}

	// Execute 'LoadLibrary' in target application
	const scoped_handle load_thread = CreateRemoteThread(remote_process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(loading_thread_func_address), load_param, 0, nullptr);
	if (load_thread == nullptr)
	{
		wprintf(L"\nFailed to execute 'LoadLibrary' in target application!\n");
		return GetLastError();
	}

	// Wait for loading to finish and clean up parameter memory afterwards
	WaitForSingleObject(load_thread, INFINITE);
	VirtualFreeEx(remote_process, load_param, 0, MEM_RELEASE);

	// Thread thread exit code will contain the module handle
	if (DWORD exit_code; GetExitCodeThread(load_thread, &exit_code) &&
#if RESHADE_LOADING_THREAD_FUNC
		exit_code == ERROR_SUCCESS)
#else
		exit_code != NULL)
#endif
	{
		wprintf(L"Succeeded!\n");
	}
	else
	{
#if RESHADE_LOADING_THREAD_FUNC
		wprintf(L"\nFailed to load ReShade in target application! Error code is 0x%x.\n", exit_code);
		return exit_code;
#else
		wprintf(L"\nFailed to load ReShade in target application!\n");
		return ERROR_MOD_NOT_FOUND;
#endif
	}
	return 0;
}