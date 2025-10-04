#pragma once

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <Windows.h>
#include <dwmapi.h>

/// @brief Provides utility functions for handling window operations 
///        such as focusing, restoring UI, and retrieving PIDs.
/// 
/// NOTE: This class stores pointers to variables defined in the main program.
///       Be careful to ensure those variables outlive this class instance.
class MoonHwndOperations
{
private:
    bool* WAIT_FOR_MS = nullptr;   // Pointer to wait timing mode (true = ms, false = µs)
    HWND* g_MainWindow = nullptr;   // Pointer to the main program's window handle

public:
    /// @brief Set reference to a flag that controls wait timing.
    /// @param waitFlag Reference to a bool (true = sleep in ms, false = sleep in µs).
    void setWaitForMs(bool& waitFlag)
    {
        WAIT_FOR_MS = &waitFlag;
    }

    /// @brief Set reference to the main program's window handle.
    /// @param globalWindow Reference to HWND variable.
    void setGlobalMainWindow(HWND& globalWindow)
    {
        g_MainWindow = &globalWindow;
    }

    /// @brief Restore and bring back the UI of the global main window.
    void bringBackUI()
    {
        if (g_MainWindow && IsWindow(*g_MainWindow))
        {
            ShowWindow(*g_MainWindow, SW_RESTORE);
            SetForegroundWindow(*g_MainWindow);
        }
    }

    /// @brief Focus on a specific window until it is the foreground window.
    /// @param hwnd Reference to the target window handle.
    void focusOnHwnd(HWND& hwnd)
    {
        if (!IsWindow(hwnd)) return;

        // Restore and bring window to the foreground
        ShowWindow(hwnd, SW_RESTORE);
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);

        // Busy wait until window is in foreground
        while (true)
        {
            if (GetForegroundWindow() == hwnd)
                break;

            (*WAIT_FOR_MS) ? std::this_thread::sleep_for(std::chrono::milliseconds(1)) : std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

    /// @brief Get process IDs for a list of window handles.
    /// @param customizableInstances Vector of HWNDs (window handles).
    /// @return Vector of process IDs corresponding to the given windows.
    std::vector<DWORD> getPids(const std::vector<HWND>& customizableInstances)
    {
        std::vector<DWORD> pids;

        for (HWND instance : customizableInstances)
        {
            DWORD pid = 0;
            GetWindowThreadProcessId(instance, &pid);

            if (pid != 0)
                pids.push_back(pid);
        }

        return pids;
    }
};
