#include <Windows.h>
#include <stdio.h>

class PrivilegeManager {
public:
    static void EnableDebugPrivilege() {
        HANDLE tokenHandle;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tokenHandle)) {
            LUID luid;
            if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
                wprintf(L"[-] Error retrieving SE_DEBUG_NAME LUID. Error: %d\n", GetLastError());
                CloseHandle(tokenHandle);
                return;
            }

            TOKEN_PRIVILEGES tp;
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            if (!AdjustTokenPrivileges(tokenHandle, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
                wprintf(L"[-] Error setting debug privilege. Error: %d\n", GetLastError());
            }
            else {
                wprintf(L"[+] Debug privilege successfully enabled.\n");
            }
            CloseHandle(tokenHandle);
        }
        else {
            wprintf(L"[-] Error opening process token. Error: %d\n", GetLastError());
        }
    }
};

class DebuggerProcess {
public:
    static void StartDebugging(DWORD targetPID) {
        wprintf(L"[+] Attempting to execute DebugActiveProcess on target PID: %d...\n", targetPID);
        Sleep(2000);

        if (DebugActiveProcess(targetPID)) {
            wprintf(L"[+] No debugger detected, self-debugging successful.\n");

            // Wait for debugging events (for demonstration purposes)
            DEBUG_EVENT debugEvent;
            if (WaitForDebugEvent(&debugEvent, 5000)) {  // 5-second timeout
                wprintf(L"[+] Debug event detected. Debugging completed.\n");
                ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
            }
            else {
                wprintf(L"[+] No debug events detected, ending debugging.\n");
            }
            DebugActiveProcessStop(targetPID); // Stops debugging without terminating the main process
        }
        else {
            wprintf(L"[-] Debugger detected.\n");  // Fails if another debugger is already attached
        }

        Sleep(5000); // Keep the window open to display the output
    }
};

int wmain(int argc, wchar_t* argv[]) {
    if (argc > 2 && wcscmp(argv[1], L"--debug") == 0) {
        DWORD pid = _wtoi(argv[2]);
        if (pid == 0) {
            wprintf(L"[-] Error: Invalid process ID provided.\n");
            return 1;
        }
        DebuggerProcess::StartDebugging(pid);
        return 0;
    }

    // Main process mode
    DWORD pid = GetCurrentProcessId();
    wprintf(L"[+] Your process ID is %d\n", pid);

    // Prompt to enable debug privilege
    wprintf(L"Do you want to enable debug privilege? (Yes/No): ");
    wchar_t input[4];
    wscanf_s(L"%3s", input, (unsigned)_countof(input));
    if (wcscmp(input, L"Yes") == 0 || wcscmp(input, L"yes") == 0) {
        PrivilegeManager::EnableDebugPrivilege();
    }
    else {
        wprintf(L"[+] Debug privilege will not be enabled.\n");
    }

    // Restart the program in debugger mode and pass the main process's PID
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    wchar_t commandLine[256];
    swprintf(commandLine, sizeof(commandLine) / sizeof(wchar_t), L"\"%s\" --debug %d", argv[0], pid);

    if (CreateProcessW(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        wprintf(L"[+] Monitoring process started (PID: %d).\n", pi.dwProcessId);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        wprintf(L"[-] Error starting monitoring process. Error: %d\n", GetLastError());
    }

    Sleep(5000); // Wait to see the output in the main process
    return 0;
}
