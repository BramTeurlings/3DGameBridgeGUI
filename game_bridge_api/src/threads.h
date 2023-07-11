#pragma once
#include "Windows.h"
#include <chrono>
#include <thread>

struct TimeMeasurements
{
    std::chrono::high_resolution_clock::time_point a_before;
    std::chrono::high_resolution_clock::time_point b_before;
    std::chrono::high_resolution_clock::time_point c_before;
    std::chrono::high_resolution_clock::time_point d_before;

    std::chrono::high_resolution_clock::time_point a_after;
    std::chrono::high_resolution_clock::time_point b_after;
    std::chrono::high_resolution_clock::time_point c_after;
    std::chrono::high_resolution_clock::time_point d_after;
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
	static inline TimeMeasurements* perf_time = nullptr;
};
