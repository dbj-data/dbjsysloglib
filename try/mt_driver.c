// refactored code from
// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-threads
#include <strsafe.h>

// using dbj syslog interface
#include "../dll/dbjsyslogclient.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAX_THREADS 0xF
#define BUF_SIZE BUFSIZ

#define WORKER_LOOP_SIZE 1

DWORD WINAPI thread_worker_function_(LPVOID lpParam);
void error_handler_(dbjsyslog_client* syslog_, LPCSTR lpszFunction);

// Sample custom data structure for threads to use.
// This is passed by void pointer so it can be any data type
// that can be passed using a single void pointer (LPVOID).
typedef struct Thread_user_data_type_ {
  int val1;
  int val2;
  // this is how we pass dbjsyslog interface pointer to threads
  dbjsyslog_client* syslog_;
} MYDATA, *PMYDATA;

int multi_threading_driver_(dbjsyslog_client* syslog_) 
{
  PMYDATA data_pointers_[MAX_THREADS] = {0};
  DWORD thread_identifiers_[MAX_THREADS] = {0};
  HANDLE thread_handles_[MAX_THREADS] = {0};

  // Create MAX_THREADS worker threads.

  for (int thread_index_ = 0; thread_index_ < MAX_THREADS; thread_index_++) {
    // Allocate memory for thread data.

    data_pointers_[thread_index_] =
        (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));

    if (data_pointers_[thread_index_] == NULL) {
      // If the array allocation fails, the system is out of memory
      // so there is no point in trying to print an error message.
      // Just terminate execution.
      // ExitProcess(2);
      // but not while on the cloud
      // users will never know what has happened
      // this we will syslog before leaving
      syslog_->alert("%s",
                     "Array allocation failed. Leaving the "
                     "multi_threading_driver_ service");
      return FALSE;
    }

    // Generate unique data for each thread to work with.

    data_pointers_[thread_index_]->val1 = thread_index_;
    data_pointers_[thread_index_]->val2 = thread_index_ + 100;
    data_pointers_[thread_index_]->syslog_ = syslog_;

    // Create the thread to begin execution on its own.

    thread_handles_[thread_index_] = CreateThread(
        NULL,                                  // default security attributes
        0,                                     // use default stack size
        thread_worker_function_,               // thread function name
        data_pointers_[thread_index_],         // argument to thread function
        0,                                     // use default creation flags
        &thread_identifiers_[thread_index_]);  // returns the thread identifier

    // Check the return value for success.
    // If CreateThread fails, terminate execution.
    // This will automatically clean up threads and memory.

    if (thread_handles_[thread_index_] == NULL) {
      // in this syslog enabled scenario
      // we know error_handler_ is doing sysloging
      error_handler_(syslog_, "CreateThread");
      // ExitProcess(3);
      // again we are on the server side and can not just exit
      // we need to syslog before we leave
      syslog_->alert("%s", "Leaving the multi_threading_driver_.");
      return FALSE;
    }
  }  // End of main thread creation loop.

  // Wait until all threads have terminated.

  WaitForMultipleObjects(MAX_THREADS, thread_handles_, TRUE, INFINITE);

  // Close all thread handles and free memory allocations.

  for (int thread_index_ = 0; thread_index_ < MAX_THREADS; thread_index_++) {
    CloseHandle(thread_handles_[thread_index_]);
    if (data_pointers_[thread_index_] != NULL) {
      HeapFree(GetProcessHeap(), 0, data_pointers_[thread_index_]);
      data_pointers_[thread_index_] = NULL;  // Ensure address is not reused.
    }
  }

  syslog_->debug("%s", "multi_threading_driver_, done");
  return TRUE;
}
/*-----------------------------------------------------------------*/
static DWORD WINAPI thread_worker_function_(LPVOID lpParam) {
  // HANDLE hStdout = 0;
  PMYDATA data_pointers_ = 0;

  CHAR msgBuf[BUF_SIZE] = {0};
  size_t cchStringSize = 0;

  // Cast the parameter to the correct data type.
  // The pointer is known to be valid because
  // it was checked for NULL before the thread was created.
  data_pointers_ = (PMYDATA)lpParam;

  dbjsyslog_client* syslog_ = data_pointers_->syslog_;

  StringCchPrintfA(
      msgBuf, BUF_SIZE, "Thread [%6d] received parameters = %3d, %3d",
      GetCurrentThreadId(), data_pointers_->val1, data_pointers_->val2);
  StringCchLengthA(msgBuf, BUF_SIZE, &cchStringSize);

  // note: dbj syslog functions do lock/unlock
  for (unsigned k = 0; k < WORKER_LOOP_SIZE; ++k) {
    syslog_->info("%s", msgBuf);
    syslog_->emergency("%s", msgBuf);
    syslog_->alert("%s", msgBuf);
    syslog_->critical("%s", msgBuf);
    syslog_->error("%s", msgBuf);
    syslog_->warning("%s", msgBuf);
    syslog_->debug("%s", msgBuf);
    Sleep(1000);
  }

  return 0;
}

/*-----------------------------------------------------------------*/
static void error_handler_(dbjsyslog_client* syslog_, LPCSTR lpszFunction) {
  // Retrieve the system error message for the last-error code.

  LPVOID lpMsgBuf = 0;
  LPVOID lpDisplayBuf = 0;
  DWORD dw = GetLastError();

  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPSTR)&lpMsgBuf, 0, NULL);

  // can not Display the error message.
  // will syslog() instead

  lpDisplayBuf = (LPVOID)LocalAlloc(
      LMEM_ZEROINIT,
      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) *
          sizeof(CHAR));

  StringCchPrintfA((LPSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(CHAR),
                   "%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf);

  // there is no message box on the server
  // MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

  syslog_->error("%s", (LPCTSTR)lpDisplayBuf);

  // Free error-handling buffer allocations.

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}