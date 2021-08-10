#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef uintptr_t usize;
typedef intptr_t  isize;

typedef wchar_t wchar;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


#define Print(String, ...) Print_(String, __VA_ARGS__, 0);

void Print_(wchar *String, ...);
void PrintU32(u32 Number);
void PrintUsage(void);

void HandleCreate(wchar *OriginalFile, wchar *TargetFile);
void HandleLaunch(wchar *TargetFile);
void HandleDelete(wchar *TargetFile);


typedef struct {
    u32 Size;
    u8 *Data;

    bool IsLoaded;
} loaded_file;

bool        FileWrite(wchar *Path, u8 *Data, u32 Size);
loaded_file FileRead(wchar *Path);
void        UnloadFile(loaded_file File);


typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef void *PRTL_RELATIVE_NAME_U;

NTSYSAPI BOOLEAN NTAPI
RtlDosPathNameToNtPathName_U(PCWSTR DosFileName,
                             PUNICODE_STRING NtFileName,
                             PWSTR *FilePart,
                             PRTL_RELATIVE_NAME_U RelativeName);

wchar *CreateTargetPath(wchar *Path);
