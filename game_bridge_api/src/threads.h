#pragma once
#include "Windows.h"
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <Wbemidl.h>

struct ProcessDetectionData
{
    std::vector <std::string> supported_titles;
    std::chrono::high_resolution_clock::time_point pr_start_tm; //process launch time point
};


class WinThreadPool
{
    BOOL bRet = FALSE;
    PTP_WORK work = NULL;
    PTP_POOL pool = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    PTP_CLEANUP_GROUP cleanupgroup = NULL;
    UINT rollback = 0;

public:
	WinThreadPool();
	~WinThreadPool();

	bool StartWork(void(*work_callback)(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work), PVOID parameter);
    static void DefaultCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work);
};
