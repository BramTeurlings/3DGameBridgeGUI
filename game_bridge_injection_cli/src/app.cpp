#include "app.h"
#include <iostream>
#include <chrono>

#include "gamebridgeapi.h"
#include "logger.h"
#include "threads.h"
#include "hooks.h"

namespace fs = std::filesystem;

bool RunExternalProgram(const std::string externalProgram);
bool ExitExternalProgram();

namespace game_bridge {

    GameBridgeInjectionCLI::GameBridgeInjectionCLI(): process_detection(WMICommunication(""))
    {

	}

	void TestWithApplication()
    {
		///////////////////////// Performance measurements
		LOG << "Waiting 5 seconds for initialize process events to pass...";
		Sleep(5000);
		LOG << "Start measurement";
		RunExternalProgram("C:/Program Files (x86)/Steam/steamapps/common/ULTRAKILL/ULTRAKILL.exe");
		
    }

    void GameBridgeInjectionCLI::RunAutomaticInjector(std::string sr_binary_path)
    {
        // Load configuration
        InitializeConfiguration();
        supported_games = LoadConfiguration();

        // Initialize Game Bride Api
        game_bridge::init_api();
        game_bridge::subscribe_to_pocess_events();

		// Wack way of doing this, but it was quick
		process_detection.InitializePayload(sr_binary_path);

		// Put supported titles in a separate list
		std::vector<std::string> executable_names;
		std::vector<std::string> config_paths;
		std::vector<std::string> preset_paths;
		//TODO If an exe name doesn't have the .exe extensions, add logic to handle that
		for (GameConfiguration config : supported_games) {
			executable_names.push_back(config.exe_name);
			config_paths.push_back(DetermineGameFixPath(config.exe_name, SuperDepth) + "\\ReShade.ini");
			preset_paths.push_back(DetermineGameFixPath(config.exe_name, SuperDepth) + "\\ReShadePreset.ini");
		}
		WMICommunication::GetDetectionData().supported_titles = executable_names;
		WMICommunication::GetDetectionData().config_paths = config_paths;
		WMICommunication::GetDetectionData().preset_paths = preset_paths;

        // Init WMI here for now
		SetIndicateEventCallback(WmiSearchCallback);
        process_detection.InitializeObjects("");

		// For measuring injection time with an application
		//TestWithApplication();
		//process_detection.GetDetectionData().pr_start_tm = std::chrono::high_resolution_clock::now();
    }

	void GameBridgeInjectionCLI::RunMessageInterceptHooks(HINSTANCE hInstance, std::string sr_binary_path)
	{
		InitializeMSAA(hInstance);
	}
}

PROCESS_INFORMATION processInfo;
bool RunExternalProgram(const std::string externalProgram)
{
	STARTUPINFOA startupInfo;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	ZeroMemory(&processInfo, sizeof(processInfo));

	bool success = true;
	// Start the child process.
	if (!CreateProcessA(NULL,           // No module name (use command line)
		(LPSTR)externalProgram.c_str(),     // Command line
		NULL,                           // Process handle not inheritable
		NULL,                           // Thread handle not inheritable
		TRUE,                           // Set handle inheritance
		0,                              // No creation flags
		NULL,                           // Use parent's environment block
		NULL,                           // Use parent's starting directory
		&startupInfo,                   // Pointer to STARTUPINFO structure
		&processInfo)                   // Pointer to PROCESS_INFORMATION structure
		)
	{
		LOG << "SetHandleInformation failed: " << GetLastError();
		success = false;
	}

    return success;
}

bool ExitExternalProgram()
{
	bool success = false;
	while(!TerminateProcess(processInfo.hProcess, NULL))
	{
		LOG << "Didn't close handle, retrying " << GetLastError();
		Sleep(100);
	}
	WaitForSingleObject(processInfo.hThread, 5000);

	if (!CloseHandle(processInfo.hProcess)) {
		LOG << "Failed closing process handle, error: " << GetLastError();
		success = false;
	}
	if (!CloseHandle(processInfo.hThread)) {
		LOG << "Failed closing thread handle, error: " << GetLastError();
		success = false;
	}

	return success;
}
