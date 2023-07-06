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
static DWORD WINAPI loading_thread_func(loading_data* arg);
#endif


int InjectIntoApplication(uint32_t pid, std::string sr_binary_path, uint32_t sleep_time = 50);
int GetPID(std::string process_name);
