#include "process_injection.h"

#include <TlHelp32.h>
#include <sddl.h>
#include <AclAPI.h>

#include <iostream>
#include <chrono>

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
	return 0;
}

int SetReshadeConfigPathInPayload(const std::string& reshade_config_path, loading_data& data)
{
	// Convert to wide string
	size_t converted_chars;
	WCHAR w_reshade_config_path[MAX_PATH];
	errno_t err = mbstowcs_s(&converted_chars, w_reshade_config_path, reshade_config_path.size() + 1, reshade_config_path.c_str(), _TRUNCATE);
	if (err == EINVAL)
	{
		wprintf(L"Couldn't convert executable path to wstring");
		return 1;
	}

	SetReshadeConfigPathInPayload(w_reshade_config_path, data);
	return 0;
}

int SetReshadeConfigPathInPayload(const wchar_t* w_reshade_config_path, loading_data& data)
{
	// Set Game Bridge environment variable
	wcscpy(data.gb_env_config_value, w_reshade_config_path);
	return 0;
}

int SetReshadePresetPathInPayload(const std::string& reshade_preset_path, loading_data& data)
{
	// Convert to wide string
	size_t converted_chars;
	WCHAR w__path[MAX_PATH];
	errno_t err = mbstowcs_s(&converted_chars, w__path, reshade_preset_path.size() + 1, reshade_preset_path.c_str(), _TRUNCATE);
	if (err == EINVAL)
	{
		wprintf(L"Couldn't convert executable path to wstring");
		return 1;
	}

	SetReshadePresetPathInPayload(w__path, data);
	return 0;
}

int SetReshadePresetPathInPayload(const wchar_t* w_reshade_config_path, loading_data& data)
{
	// Set Game Bridge environment variable
	wcscpy(data.gb_env_preset_value, w_reshade_config_path);
	return 0;
}
#endif

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
