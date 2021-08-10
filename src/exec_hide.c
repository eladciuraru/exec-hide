#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "exec_hide.h"

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ntdll.lib")


void mainCRTStartup() {
    i32     ArgsCount = 0;
    wchar **ArgsList  = CommandLineToArgvW(GetCommandLineW(), &ArgsCount);

    if (ArgsCount < 2) {
        PrintUsage();
        goto EXIT;
    }

    if (lstrcmpiW(ArgsList[1], L"create") == 0
        && ArgsCount == 4) {
        HandleCreate(ArgsList[2], ArgsList[3]);

    } else if (lstrcmpiW(ArgsList[1], L"launch") == 0
               && ArgsCount == 3) {
        // TODO: Support passing parameters
        HandleLaunch(ArgsList[2]);

    } else if (lstrcmpiW(ArgsList[1], L"delete") == 0
               && ArgsCount == 3) {
        HandleDelete(ArgsList[2]);

    } else {
        PrintUsage();
    }

EXIT:
    LocalFree(ArgsList);
    ExitProcess(EXIT_SUCCESS);
}


void Print_(wchar *String, ...) {
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),
                  String, lstrlenW(String), 0, 0);

    va_list ArgsList;
    va_start(ArgsList, String);

    while (true) {
        wchar *Arg = va_arg(ArgsList, wchar *);
        if (Arg == 0) { break; }

        WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),
                      Arg, lstrlenW(Arg), 0, 0);
    }

    va_end(ArgsList);
}


void PrintU32(u32 Number) {
    wchar Buffer[128] = { 0 };

    u32 Index = 0;
    do {
        Buffer[Index++] = Number % 10 + L'0';
        Number          /= 10;
    } while (Number != 0);

    for (u32 Start = 0, End = Index - 1; Start < End; Start++, End--) {
        Buffer[Start] ^= Buffer[End];
        Buffer[End]   ^= Buffer[Start];
        Buffer[Start] ^= Buffer[End];
    }

    Print(Buffer);
}


void PrintUsage(void) {
    static wchar *UsageMessage =
        L"usage: \n"
        L"\thide.exe create <source path> <target path>\n"
        L"\thide.exe launch <target path>\n"
        L"\thide.exe delete <target path>\n"
        L"\n"
        L"Create command - takes the source file and output it next to the target path\n"
        L"Launch command - starts the process\n"
        L"Delete command - remove the file created on create command\n"
        L"\n";

    Print(UsageMessage);
}


void HandleCreate(wchar *OriginalFile, wchar *TargetFile) {
    wchar      *TargetPath = CreateTargetPath(TargetFile);
    loaded_file File       = FileRead(OriginalFile);
    if (!File.IsLoaded) {
        Print(L"[!] Failed to read source file - ", OriginalFile, L"\n");

    } else if (!FileWrite(TargetPath, File.Data, File.Size)) {
        Print(L"[!] Failed to write target file - ", TargetPath, L"\n");
    }

    HeapFree(GetProcessHeap(), 0, TargetPath);
    UnloadFile(File);
}


void HandleLaunch(wchar *TargetFile) {
    wchar *TargetPath = CreateTargetPath(TargetFile);

    STARTUPINFOW        StartInfo;
    PROCESS_INFORMATION ProcInfo;
    if (!CreateProcessW(TargetPath, 0, 0, 0, 0, 0, 0, 0,
                        &StartInfo, &ProcInfo)) {
        Print(L"[!] Failed to launch process - ", TargetPath, L"\n");

    } else {
        Print(L"[*] Create process id - ");
        PrintU32(ProcInfo.dwProcessId);
        Print(L"\n");
    }

    HeapFree(GetProcessHeap(), 0, TargetPath);
}


void HandleDelete(wchar *TargetFile) {
    wchar *TargetPath = CreateTargetPath(TargetFile);

    if (!DeleteFileW(TargetPath)) {
        Print(L"[!] Failed to delete - ", TargetPath, L"\n");
    }

    HeapFree(GetProcessHeap(), 0, TargetPath);
}


wchar *CreateTargetPath(wchar *Path) {
    UNICODE_STRING NtPath = { 0 };
    RtlDosPathNameToNtPathName_U(Path, &NtPath, 0, 0);

    usize  TargetSize = NtPath.Length + 2 * sizeof(NtPath.Buffer[0]);
    wchar *TargetPath = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, TargetSize);
    memcpy(TargetPath, NtPath.Buffer, NtPath.Length);

    usize Index = NtPath.Length / sizeof(NtPath.Buffer[0]);
    TargetPath[Index + 0] = L' ';
    TargetPath[Index + 1] = 0;

    // ReactOS showed that this function allocates
    // memory using the process heap
    HeapFree(GetProcessHeap(), 0, NtPath.Buffer);

    return TargetPath;
}


loaded_file FileRead(wchar *Path) {
    loaded_file File = { 0 };

    HANDLE FileHandle = CreateFileW(Path, GENERIC_READ, FILE_SHARE_READ,
                                    0, OPEN_EXISTING, 0, 0);
    if (FileHandle == INVALID_HANDLE_VALUE) {
        goto EXIT;
    }

    LARGE_INTEGER FileSize = { 0 };
    GetFileSizeEx(FileHandle, &FileSize);
    if (FileSize.HighPart != 0) {
        goto EXIT;
    }

    u8 *Buffer    = VirtualAlloc(0, FileSize.LowPart,
                                 MEM_COMMIT | MEM_RESERVE,
                                 PAGE_READWRITE);
    u32 BytesRead = 0;
    ReadFile(FileHandle, Buffer, FileSize.LowPart, (PDWORD) &BytesRead, 0);
    if (BytesRead != FileSize.LowPart) {
        VirtualFree(Buffer, 0, MEM_RELEASE);
        goto EXIT;
    }

    File.Size     = FileSize.LowPart;
    File.Data     = Buffer;
    File.IsLoaded = true;

EXIT:
    if (FileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(FileHandle);
    }

    return File;
}


bool FileWrite(wchar *Path, u8 *Data, u32 Size) {
    bool   Flag       = false;
    HANDLE FileHandle = CreateFileW(Path, GENERIC_WRITE, 0,
                                    0, CREATE_ALWAYS, 0, 0);
    if (FileHandle == INVALID_HANDLE_VALUE) {
        goto EXIT;
    }

    u32 BytesWritten = 0;
    WriteFile(FileHandle, Data, Size, (LPDWORD) &BytesWritten, 0);
    if (BytesWritten != Size) {
        goto EXIT;
    }

    Flag = true;

EXIT:
    if (FileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(FileHandle);
    }

    return Flag;
}


void UnloadFile(loaded_file File) {
    if (File.IsLoaded) {
        VirtualFree(File.Data, 0, MEM_RELEASE);
    }
}


// #################################
// CRT replacement shit
void *memset(void *Dest, int Value, size_t Size)
{
    u8 *DestU8 = (u8 *) Dest;

    for (size_t Index = 0; Index < Size; Index++) {
        DestU8[Index] = Value;
    }

    return Dest;
}


void *memcpy(void *Dest, void const *Source, size_t Size)
{
    u8 *SourceU8 = (u8 *) Source;
    u8 *DestU8   = (u8 *) Dest;

    for (size_t Index = 0; Index < Size; Index++) {
        DestU8[Index] = SourceU8[Index];
    }

    return Dest;
}
