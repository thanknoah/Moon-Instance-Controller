// Imports
#include <algorithm>
#include <random>
#include <tuple>
#include <mutex>
#include <format>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <tlhelp32.h>
#include <cmath>
#undef min

// Mutex
std::mutex clientMutex;

// Main
class Main {
public:
    void intialise()
    {
        activateThreads();
    }

private:
    struct GuiFormat {
        HWND ui;
        bool firstClient;
        bool activeThread;
        int timeElapsed;
        int cpuTries;
        HANDLE hJob;
        HANDLE hProcess;
    };

    int amountOfThreads = 0;
    std::mutex listOfHwndsMutex;
    std::vector<std::shared_ptr<GuiFormat>> listOfHwnds;

    void handleErr(std::string msg)
    {
        std::cout << "[SYSTEM] There has been an error: " << msg << ".\n";
        Sleep(3000);
        exit(1);
    }

    void nonCriticalErr(std::string msg)
    {
        std::cout << "[IGNORE] There has been an error: " << msg << ".\n";
    }

    // Get total amount of Roblox Instances Open
    std::vector<HWND> getRobloxWindows()
    {
        // Searches for Roblox Instances
        std::vector<HWND> instances;
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
            {
                wchar_t title[256];
                GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));

                if (wcsstr(title, L"Roblox"))
                {
                    ((std::vector<HWND>*)lParam)->push_back(hwnd);
                }


                return true;
            }, (LPARAM)&instances);

        // Cleans up table
        for (auto it = instances.begin(); it != instances.end(); )
        {
            wchar_t title[256];
            const wchar_t* robloxName = L"Roblox";
            GetWindowText(*it, title, sizeof(title) / sizeof(wchar_t));

            if (wcscmp(title, robloxName) != 0) { it = instances.erase(it); }
            if (wcscmp(title, robloxName) == 0) { ++it; }
        }
        return instances;
    }

    std::vector<DWORD> getPIDs(std::vector<HWND> customCurrentClient)
    {
        // Variables
        std::vector<DWORD> listOfPids;
        if (customCurrentClient.size() == 0) return listOfPids;

        // Looping through PIDs and getting the threads
        DWORD pid = 0;
        for (HWND& robloxClient : customCurrentClient) {
            GetWindowThreadProcessId(robloxClient, &pid);
            if (pid != 0)
                listOfPids.push_back(pid);
        }

        return listOfPids;
    }

    void limitRamConstantly()
    {
        while (true)
        {
            // Varaibles
            std::vector<HWND> listOfClients = getRobloxWindows();
            std::vector<DWORD> pids = getPIDs(listOfClients);
            std::vector<std::shared_ptr<GuiFormat>> snapShot;
            if (listOfClients.size() == 0) continue;

            // Memory Related shi
            MEMORYSTATUSEX memObj;
            memObj.dwLength = sizeof(memObj);
            GlobalMemoryStatusEx(&memObj);

            // Taking a screenshot of vector
            size_t ramForEachClient = (memObj.ullAvailPhys * 0.9) / pids.size();
            {
                std::unique_lock<std::mutex> lock(listOfHwndsMutex);
                snapShot = listOfHwnds;
            }

            for (auto& element : snapShot)
            {
                // Open Process in PID
                HWND gui = element->ui;
                DWORD pid = 0;
                GetWindowThreadProcessId(gui, &pid);

                if (pid == 0)
                    continue;

                // Opening up process
                HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, pid);
                if (hProcess == nullptr)
                {
                    nonCriticalErr("Couldn't open Roblox client PID#" + std::to_string(pid));
                    continue;
                }


                // Assign RAM Limitations
                size_t lowerRamLimit = 1ULL * 1024 * 1024;
                size_t startUpRamLimit = 500ULL * 1024 * 1024;
                size_t activeThreadRamLimit = 250ULL * 1024 * 1024;
                size_t upperRamLimit;

                // Determining Ram
                if (element->activeThread == false)
                    upperRamLimit = activeThreadRamLimit;
                else
                    upperRamLimit = startUpRamLimit;

                // Apply soft RAM Limits to the process
                if (!SetProcessWorkingSetSizeEx(hProcess, lowerRamLimit, upperRamLimit, 0))
                {
                    DWORD err = GetLastError();
                    nonCriticalErr("Failed to assign RAM soft limit to Roblox Client, PID#" + std::to_string(pid) + ", error code: " + std::to_string(err));
                }

                // Close the process handle
                CloseHandle(hProcess);
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    void focusOnHwnd(HWND hwnd)
    {
        ShowWindow(hwnd, SW_RESTORE);
        BringWindowToTop(hwnd);
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);

        while (GetForegroundWindow() != hwnd)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void pressSpace(int durationMs)
    {
        INPUT down = {};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = VK_SPACE; // Spacebar
        down.ki.wScan = MapVirtualKey(VK_SPACE, MAPVK_VK_TO_VSC);
        down.ki.dwFlags = KEYEVENTF_SCANCODE;

        INPUT up = down;
        up.ki.dwFlags |= KEYEVENTF_KEYUP;

        SendInput(1, &down, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        SendInput(1, &up, sizeof(INPUT));
    }


    /*
    void antiActivityClient()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::minutes(10));
            std::vector<std::shared_ptr<GuiFormat>> screenshotOfHwnd;
            size_t chunkSize = 10;
            size_t totalSize;
            size_t totalIterations;

            {
                std::unique_lock<std::mutex> lock(listOfHwndsMutex);
                screenshotOfHwnd = listOfHwnds;

                totalSize = screenshotOfHwnd.size();
                totalIterations = static_cast<size_t>(std::ceil(totalSize / 10.0));

                lock.unlock();
            }

            auto focus = [&screenshotOfHwnd, this](size_t start, size_t end, bool enable)
                {
                    for (size_t j = start; j < end; ++j)
                    {
                        auto& element = screenshotOfHwnd[j];
                        if (element->activeThread == false && element->hJob != nullptr)
                        {
                            JOBOBJECT_CPU_RATE_CONTROL_INFORMATION info = {};
                            info.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;

                            if (enable)
                                info.CpuRate = 300;
                            else
                                info.CpuRate = 50;

                            if (!SetInformationJobObject(element->hJob, JobObjectCpuRateControlInformation, &info, sizeof(info)))
                            {
                                std::cout << "Couldn't assign info for PID";
                                continue;
                            }       
                        }
                    }
                };

            for (size_t i = 0; i < totalIterations; ++i) 
            {
                size_t start = i * chunkSize;
                size_t end = std::min(start + chunkSize, totalSize); 

                focus(start, end, true);
                std::this_thread::sleep_for(std::chrono::seconds(5));

                for (size_t j = start; j < end; ++j)
                {
                    auto& element = screenshotOfHwnd[j];
                    ShowWindow(element->ui, SW_RESTORE);
                    focusOnHwnd(element->ui);
                    pressSpace(20);
                    ShowWindow(element->ui, SW_MINIMIZE);
                }

                focus(start, end, false);
            }
        }
    }
    */
   
    void mananageinternalWorker(std::shared_ptr<GuiFormat> GuiObj)
    {
        while (true)
        {
            // Waiting
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (GuiObj->firstClient == true)
                break;

            if (GuiObj->timeElapsed == 3 || GuiObj->timeElapsed == 4 || GuiObj->timeElapsed == 5 && GuiObj->firstClient == FALSE)
            {
                ShowWindow(GuiObj->ui, SW_MINIMIZE);
                std::cout << "Minimized Window";
            }

            // Updating Variables
            if (GuiObj->timeElapsed < 13)
            {
                GuiObj->timeElapsed += 1;
            }
            else
            {
                // Adding one
                if (GuiObj->cpuTries == 1)
                    break;
                GuiObj->cpuTries += 1;

                // Setting Up Job
                HANDLE hJob = CreateJobObject(nullptr, nullptr);
                JOBOBJECT_CPU_RATE_CONTROL_INFORMATION info = {};
                info.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;
                info.CpuRate = 80;

                // Attempting to apply job
                GuiObj->hJob = hJob;

                if (!SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &info, sizeof(info)))
                {
                    nonCriticalErr("Couldn't assign info for PID");
                    CloseHandle(hJob);
                    CloseHandle(GuiObj->hProcess);
                    GuiObj->hJob = nullptr;
                    return;
                }

                if (!AssignProcessToJobObject(hJob, GuiObj->hProcess))
                {
                    nonCriticalErr("Couldn't assign job to PID#");
                    CloseHandle(hJob);
                    CloseHandle(GuiObj->hProcess);
                    GuiObj->hJob = nullptr;
                    return;
                }
                else
                {
                    std::cout << "\nAssigned CPU Limiter to PID\n";
                }
            }
        }

        GuiObj->activeThread = false;
        CloseHandle(GuiObj->hProcess);

        std::cout << "BROKEN FROM THREAD!";
    }

    // Gets PIDs
    std::vector<DWORD> getPids(std::vector<HWND>& customizableInstances)
    {
        std::vector<DWORD> pids;
        DWORD pid = 0;

        for (HWND instance : customizableInstances)
        {
            GetWindowThreadProcessId(instance, &pid);
            if (pid != 0)
                pids.push_back(pid);
        }

        return pids;
    }

    void logInfo()
    {
        while (true)
        {
            std::cout << "\n";
            for (auto& element : listOfHwnds) {
                DWORD pid;
                GetWindowThreadProcessId(element->ui, &pid);
                std::cout << std::boolalpha;
                std::cout << "PID: " << pid
                    << " | Time Elapsed: " << element->timeElapsed
                    << " | Cpu Tries: " << element->cpuTries
                    << " | First client: " << element->firstClient
                    << " | Active thread: " << element->activeThread
                    << "\n";
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    void acceptNewClients()
    {
        // Current status
        std::vector<HWND> previousAmountOfInstances;

        // Listener
        while (true)
        {
            // Variables
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            std::vector<HWND> instances = getRobloxWindows();

            if (instances.size() == 0) continue;

            // Find added Windows
            for (HWND& hwnd : instances)
            {
                // Checking if the hwnd is in the previous instances
                if (std::find(previousAmountOfInstances.begin(), previousAmountOfInstances.end(), hwnd) != previousAmountOfInstances.end())
                    continue;

                // Open Process && handling errors
                DWORD pid;
                GetWindowThreadProcessId(hwnd, &pid);
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SET_INFORMATION | PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, pid);
                if (hProcess == nullptr)
                {
                    std::cerr << "[FATAL ERROR]: FAILED TO OPEN PROCESS FOR PID #" << pid << ", PLEASE RESTART. CLIENT LIST: " << instances.size() << std::endl;
                    continue;
                }

                // Assigning variable
                auto newClient = std::make_shared<GuiFormat>();
                newClient->ui = hwnd;
                newClient->cpuTries = 0;
                newClient->timeElapsed = 0;
                newClient->activeThread = true;
                newClient->hProcess = hProcess;

                // Handle if its first Client
                if (listOfHwnds.empty())
                {
                    newClient->firstClient = true;
                    CloseHandle(newClient->hProcess);
                    newClient->hProcess = nullptr;
                    std::thread(&Main::mananageinternalWorker, this, newClient).detach();

                    std::cout << "[SYSTEM]: First client joined (kept open, not freezing).\n";
                }
                else
                {
                    newClient->firstClient = false;
                    std::thread(&Main::mananageinternalWorker, this, newClient).detach();
                    amountOfThreads += 1;
                    std::cout << "\n" << amountOfThreads << "THREADS \n";
                }

                // Insert into array
                std::cout << "TESTING...\n";
                std::unique_lock<std::mutex> lock(listOfHwndsMutex);
                listOfHwnds.emplace_back(newClient);
                lock.unlock();
            }

            previousAmountOfInstances = instances;
            if (instances.size() == 0)
                continue;


        }
    }

    void activateThreads()
    {

        std::cout << "Moon Bot Program V2\n";
        std::cout << "CREDIT: Noah\n\n";
        std::cout << "[SYSTEM]: Starting process..\n";
        std::cout << "[SYSTEM]: Please start inserting accounts..\n";

        try {
            std::thread t1(&Main::acceptNewClients, this);
            std::thread t2(&Main::logInfo, this);
            //std::thread t3(&Main::antiActivityClient, this);
            std::thread t4(&Main::limitRamConstantly, this);

            t1.join();
            t2.join();
            //t3.join();
            t4.join();
        }
        catch (...)
        {
            DWORD errCode = GetLastError();
            int e = 0;
            while (e < 6)
            {
                std::cout << "There has been a FATAL ERROR error code: " << errCode;
                e++;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

        }
    }

};

int main()
{

    Main program;
    program.intialise();
    return 0;
}
