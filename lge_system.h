#pragma once
#ifndef __LGE_SYSTEM_H__
#define __LGE_SYSTEM_H__

#ifdef _WIN32

#include <windows.h>
typedef DWORD THREAD_RET;
#define THRAPI __stdcall
#include <stdint.h>

#else  //_WIN32

#include <stdint.h>
#include <pthread.h>

typedef void * THREAD_RET;
typedef THREAD_RET (*PTHREAD_START_ROUTINE)(void *lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef pthread_mutex_t CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

#define THRAPI

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef void * HANDLE;
#define MAXIMUM_WAIT_OBJECTS 64
#define INFINITE       (-1)
#define WAIT_FAILED    (-1)
#define WAIT_TIMEOUT   0x102
#define WAIT_OBJECT    0
#define WAIT_OBJECT_0  0
#define WAIT_ABANDONED   128
#define WAIT_ABANDONED_0 128

HANDLE CreateEvent(void *pSecAttr = 0, bool manualReset = false, bool initialState = false, void *pName = 0);
bool SetEvent(HANDLE event);
bool ResetEvent(HANDLE event);
int WaitForSingleObject(HANDLE event, uint32_t milliseconds = INFINITE);
int WaitForSingleObjectEx(HANDLE event, uint32_t milliseconds = INFINITE, bool bAlertable = false);
int WaitForMultipleObjects(uint32_t count, const HANDLE *events, bool waitAll = false, uint32_t milliseconds = INFINITE);
int WaitForMultipleObjectsEx(uint32_t count, const HANDLE *events, bool waitAll = false, uint32_t milliseconds = INFINITE, bool bAlertable = false);

bool InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
bool DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
bool EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
bool LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

HANDLE CreateThread(void *lpThreadAttributes, uint32_t dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress, void *lpParameter,
    uint32_t dwCreationFlags, uint32_t *lpThreadId);

#endif //_WIN32

bool DestroyEvent(HANDLE event);

bool CloseThread(HANDLE thread);
bool WaitThread(HANDLE thread);
bool SetThreadName(const char *name);

uint64_t GetTime();

#endif //__LGE_SYSTEM_H__
