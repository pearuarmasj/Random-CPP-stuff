#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>

#pragma comment(lib, "pdh.lib")

int main() {
    PDH_HQUERY cpuQuery;
    PDH_STATUS status = PdhOpenQuery(NULL, 0, &cpuQuery);
    if (status != ERROR_SUCCESS) {
        std::cerr << "PdhOpenQuery failed" << std::endl;
        return 1;
    }
    
    unsigned int numCores = std::thread::hardware_concurrency();
    std::vector<PDH_HCOUNTER> counters;
    for (unsigned int i = 0; i < numCores; ++i) {
        PDH_HCOUNTER counter;
        std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
        status = PdhAddCounterW(cpuQuery, counterPath.c_str(), 0, &counter);
        if (status != ERROR_SUCCESS) {
            std::cerr << "PdhAddCounterW failed for core " << i << std::endl;
        } else {
            counters.push_back(counter);
        }
    }

    // Initial collection to prime the counters
    status = PdhCollectQueryData(cpuQuery);
    if (status != ERROR_SUCCESS) {
        std::cerr << "Initial PdhCollectQueryData failed" << std::endl;
        PdhCloseQuery(cpuQuery);
        return 1;
    }
    Sleep(1000); // Wait a bit after the initial collection

    while (true) {
        status = PdhCollectQueryData(cpuQuery);
        if (status != ERROR_SUCCESS) {
            std::cerr << "PdhCollectQueryData failed" << std::endl;
            break;
        }

        // Wait for a bit to allow counters to update
        Sleep(1000);

        for (size_t i = 0; i < counters.size(); ++i) {
            PDH_FMT_COUNTERVALUE counterVal;
            status = PdhGetFormattedCounterValue(counters[i], PDH_FMT_DOUBLE, NULL, &counterVal);
            if (status != ERROR_SUCCESS) {
                std::cerr << "PdhGetFormattedCounterValue failed for core " << i << std::endl;
            } else {
                std::cout << "Core " << i << ": " << counterVal.doubleValue << "%" << std::endl;
            }
        }

        // Optional: break or continue based on some condition
    }

    PdhCloseQuery(cpuQuery);

    return 0;
}
