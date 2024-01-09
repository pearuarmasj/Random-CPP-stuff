#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")

void PrintSystemInfo() {
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);  // Retrieves information about the current system to an application running under WOW64

    std::cout << "Processor Architecture: ";
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            std::cout << "x64 (AMD or Intel)";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            std::cout << "ARM";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            std::cout << "Intel Itanium-based";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            std::cout << "x86";
            break;
        default:
            std::cout << "Unknown architecture";
            break;
    }
    std::cout << std::endl;
}

void PrintCPUSpeed() {
    HRESULT hres;

    // Initialize COM for WMI
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        std::cerr << "Failed to initialize COM library. Error code = 0x" << std::hex << hres << std::endl;
        return;
    }

    // Initialize security
    hres = CoInitializeSecurity(
        NULL, 
        -1,                          // COM negotiates service
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
    );

    if (FAILED(hres)) {
        std::cerr << "Failed to initialize security. Error code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return;
    }

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *)&pLoc);
 
    if (FAILED(hres)) {
        std::cerr << "Failed to create IWbemLocator object. Err code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return;
    }

    // Connect to WMI
    IWbemServices *pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // WMI namespace
        NULL,                    // User name
        NULL,                    // User password
        0,                       // Locale 
        NULL,                    // Security flags
        0,                       // Authority 
        0,                       // Context object 
        &pSvc                    // IWbemServices proxy
    );

    if (FAILED(hres)) {
        std::cerr << "Could not connect. Error code = 0x" << std::hex << hres << std::endl;
        pLoc->Release();     
        CoUninitialize();
        return;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres)) {
        std::cerr << "Could not set proxy blanket. Error code = 0x" << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t("SELECT * FROM Win32_Processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if (FAILED(hres)) {
        std::cerr << "Query for operating system name failed. Error code = 0x" << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Get the data from the query
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
   
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        std::wcout << "CPU Name : " << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"MaxClockSpeed", 0, &vtProp, 0, 0);
        std::wcout << "CPU Clock Speed : " << vtProp.uintVal << " MHz" << std::endl;
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}

int main() {
    PrintSystemInfo();
    PrintCPUSpeed();
    Sleep(1000);
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

    }

    PdhCloseQuery(cpuQuery);

    return 0;
}