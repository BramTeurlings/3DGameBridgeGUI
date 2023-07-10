#include "app.h"
#include <Windows.h>
#include <iostream>
#include <filesystem>

#include "wmicommunication.h"
#include "gamebridgeapi.h"
#include "process_injection.h"
#include "gamebridgeapi.h"
#include "logger.h"
#include "threads.h"

namespace fs = std::filesystem;

bool RunExternalProgram(const std::string externalProgram);
bool ExitExternalProgram();

namespace game_bridge {

    GameBridgeInjectionCLI::GameBridgeInjectionCLI()
    {

	}

    void GameBridgeInjectionCLI::RunAutomaticInjector(std::string sr_binary_path)
    {
        // Load configuration
        InitializeConfiguration();
        supported_games = LoadConfiguration();

        // Initialize Game Bride Api
        game_bridge::init_api();
        game_bridge::subscribe_to_pocess_events();

        // Init WMI here for now
		//SetIndicateEventCallback();
        WMICommunication process_detection("");
        process_detection.InitializeObjects("");

        std::vector<std::string> executable_names;
        for (GameConfiguration config : supported_games) {
            executable_names.push_back(config.exe_name);
        }

        loading_data payload_64_bit;
        CreatePayload(sr_binary_path, payload_64_bit);
        
		///////////////////////// Performance measurements
        LOG << "Waiting 5 seconds for initialize process events to pass...";
        Sleep(5000);
        LOG << "Start measurement";
        RunExternalProgram("C:/Program Files (x86)/Steam/steamapps/common/ULTRAKILL/ULTRAKILL.exe");
        perf_time.a_before = std::chrono::high_resolution_clock::now();
        // Keep the app running
        while (true) {
            // Wait for a process tobe added to the queue
            process_detection.pSink->semaphore_message_queue.wait();

            perf_time.c_after = std::chrono::high_resolution_clock::now();

           perf_time.d_before = std::chrono::high_resolution_clock::now();
            Win32ProcessData process_data = process_detection.pSink->message_queue.front();
            process_detection.pSink->message_queue.pop();
            // LOG << "pid: " << process_data.pid << " path: " << process_data.executable_path;
            perf_time.d_after = std::chrono::high_resolution_clock::now();

            fs::path detected_exe(process_data.executable_path);
            std::string filename = detected_exe.filename().string();

			bool break_true = false;
            std::for_each(executable_names.begin(), executable_names.end(), [&](std::string a) {
				if (filename.compare(a) == 0) {


					// Performance check
					perf_time.b_before = std::chrono::high_resolution_clock::now();
					InjectIntoApplication(process_data.pid, payload_64_bit);
					perf_time.b_after = std::chrono::high_resolution_clock::now();

					

					LOG << "Injected into supported game";
					LOG << "Process detection time: " << std::chrono::duration_cast<std::chrono::milliseconds>(perf_time.c_after - perf_time.a_before).count();
					LOG << "Queue pop time: " << std::chrono::duration_cast<std::chrono::milliseconds>(perf_time.d_after - perf_time.d_before).count();
					LOG << "Injection time: " << std::chrono::duration_cast<std::chrono::milliseconds>(perf_time.b_after - perf_time.b_before).count();
					LOG << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(perf_time.a_after - perf_time.a_before).count();
					break_true = true;
				}
				});

			if (break_true) {
				break;
			}
        }

		Sleep(2000);
		ExitExternalProgram();
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
