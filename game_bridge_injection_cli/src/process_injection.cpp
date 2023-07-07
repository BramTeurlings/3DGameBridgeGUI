#include "process_injection.h"
#include <string>
#include <iostream>

loading_data x32_injection_data;
loading_data x64_injection_data;

static void update_acl_for_uwp(LPWSTR path)
{
	OSVERSIONINFOEX verinfo_windows7 = { sizeof(OSVERSIONINFOEX), 6, 1 };
	const bool is_windows7 = VerifyVersionInfo(&verinfo_windows7, VER_MAJORVERSION | VER_MINORVERSION,
		VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL), VER_MINORVERSION, VER_EQUAL)) != FALSE;
	if (is_windows7)
		return; // There is no UWP on Windows 7, so no need to update DACL

	PACL old_acl = nullptr, new_acl = nullptr;
	PSECURITY_DESCRIPTOR sd = nullptr;
	SECURITY_INFORMATION siInfo = DACL_SECURITY_INFORMATION;

	// Get the existing DACL for the file
	if (GetNamedSecurityInfoW(path, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &old_acl, nullptr, &sd) != ERROR_SUCCESS)
		return;
	LocalFree(sd);

	// Get the SID for 'ALL_APPLICATION_PACKAGES'
	PSID sid = nullptr;
	if (!ConvertStringSidToSid(TEXT("S-1-15-2-1"), &sid))
		return;

	EXPLICIT_ACCESS access = {};
	access.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
	access.grfAccessMode = SET_ACCESS;
	access.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	access.Trustee.ptstrName = reinterpret_cast<LPTCH>(sid);

	// Update the DACL with the new entry
	if (SetEntriesInAcl(1, &access, old_acl, &new_acl) == ERROR_SUCCESS)
	{
		SetNamedSecurityInfoW(path, SE_FILE_OBJECT, siInfo, NULL, NULL, new_acl, NULL);
		LocalFree(new_acl);
	}

	LocalFree(sid);
}

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

int CreatePayload(const std::string& sr_binary_path, loading_data& data, bool use_32_bit)
{
	size_t num_dlls = NUM_DLLS;
	std::wstring reshade_dll_name;

	// Select 32 or 64 bit dll
	if(use_32_bit)
	{
		num_dlls = 1;
		reshade_dll_name = L"ReShade32.dll";
	}
	else
	{
		num_dlls = NUM_DLLS;
		reshade_dll_name = L"ReShade64.dll";
	}

	// Required SR dll list
	WCHAR dlls[NUM_DLLS][MAX_PATH] = {
		// List must be in load order
		L"opencv_world343.dll",
		L"glog.dll",
		L"SimulatedReality.dll",
		L"DimencoWeaving.dll",
		L"SimulatedRealityCore.dll",
		L"SimulatedRealityFaceTrackers.dll",
		L"SimulatedRealityDisplays.dll",
		L"SimulatedRealityDirectX.dll",
	};

	// Get wide string from binary path
	// Todo Catch error here
	WCHAR w_sr_binary_path[MAX_PATH];
	size_t converted_chars;
	errno_t err = mbstowcs_s(&converted_chars, w_sr_binary_path, sr_binary_path.size() + 1, sr_binary_path.c_str(), _TRUNCATE);
	if (err == EINVAL)
	{
		wprintf(L"Couldn't convert executable path to wstring");
		return 1;
	}

	// Don't include SR dlls for 32 bits because we SR doesn't support it
	if (!use_32_bit) {
		for (int i = 0; i < num_dlls - 1; i++) {
			// First copy sr binary path in the dll list, then directory divide and finally the binary name
			wcscat_s(data.load_path[i], w_sr_binary_path);
			wcscat_s(data.load_path[i], L"\\");
			wcscat_s(data.load_path[i], dlls[i]);

			if (GetFileAttributesW(data.load_path[i]) == INVALID_FILE_ATTRIBUTES)
			{
				wprintf(L"\nFailed to find dll at \"%s\"!\n", data.load_path[i]);
				return ERROR_FILE_NOT_FOUND;
			}
		}
	}
	
	// Add Reshade dll separately in the last index for 64 bits
	uint32_t reshade_dll_index = num_dlls - 1;
	GetCurrentDirectoryW(MAX_PATH, data.load_path[reshade_dll_index]);
	wcscat_s(data.load_path[reshade_dll_index], L"\\");
	wcscat_s(data.load_path[reshade_dll_index], reshade_dll_name.c_str());
	if (GetFileAttributesW(data.load_path[reshade_dll_index]) == INVALID_FILE_ATTRIBUTES)
	{
		wprintf(L"\nFailed to find dll at \"%s\"!\n", data.load_path[reshade_dll_index]);
		return ERROR_FILE_NOT_FOUND;
	}

	// Only for debugging, log dlls inside the payload
	for (int i = 0; i < num_dlls; i++)
	{
		std::wcout << data.load_path[i] << "\n";
	}

	// Make sure the DLL has permissions set up for 'ALL_APPLICATION_PACKAGES'
	for (int i = 0; i < num_dlls; i++) {
		update_acl_for_uwp(data.load_path[i]);
	}

	// This happens to work because kernel32.dll is always loaded to the same base address, so the address of 'LoadLibrary' is the same in the target application and the current one
	data.GetLastError = GetLastError;
	data.LoadLibraryW = LoadLibraryW;
	data.SetEnvironmentVariableW = SetEnvironmentVariableW;
}
#endif

int InjectIntoApplication(uint32_t pid, const loading_data& payload, uint32_t sleep_time)
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

int GetPID(std::string process_name)
{
    DWORD pid = 0;

    // Wait for a process with the target name to spawn
    while (!pid)
    {
        const scoped_handle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        PROCESSENTRY32W process = { sizeof(process) };
        for (BOOL next = Process32FirstW(snapshot, &process); next; next = Process32NextW(snapshot, &process))
        {
            if (wcscmp(process.szExeFile, L"Journey.exe") == 0)
            {
                pid = process.th32ProcessID;
                break;
            }
        }

        Sleep(1); // Sleep a bit to not overburden the CPU
    }

    wprintf(L"Found a matching process with PID %lu! Injecting ReShade ... ", pid);
	wprintf(L"\n");
	return pid;
}
