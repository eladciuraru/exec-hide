#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")


void mainCRTStartup() {
    MessageBoxA(0, "test", "Test", 0);
    ExitProcess(0);
}
