/*
 * WARNING:: This file contains some of microsoft's code to be used
 * only when the library is compiled to work with MS's MFC library.
 */
//#include <stdafx.h>
#include "ufDef.h"

#ifdef USE_MFC
#include <afx.h>
#include <afxwin.h>
#else

#include <stdlib.h>

#endif

ufDEBUG_FILE

#ifdef _DEBUG
    int NEAR ufIgnoreAssertCount = 0; // for testing diagnostics
int NEAR ufTraceEnabed = 0;
#endif

#ifdef USE_MFC
#pragma optimize("qgel", off) // assembler cannot be
// globally optimized
#endif

extern "C" {
extern void ufAbort();
extern void PASCAL ufAssertFailedLine(const char FAR *theAssertion,
                                      const char FAR *lpszFileName, int nLine);
extern void PASCAL ufAssertFailedLineSimple(const char FAR *lpszFilename,
                                            int nLine);

extern void ufCheckMemory(const char FAR *lpszFilename, int nLine);
extern void ufValidateAddress(const void FAR *ptr, unsigned int size);
}

void PASCAL ufAssertFailedLineSimple(const char FAR *lpszFilename, int nLine) {
    ufAssertFailedLine("Unknown", lpszFilename, nLine);
}

extern "C" void PASCAL ufAssertFailedLine(const char FAR *lpszTheAssertion,
                                          const char FAR *lpszFileName,
                                          int nLine) {
#ifdef _DEBUG
    if (ufIgnoreAssertCount > 0) {
        ufIgnoreAssertCount--;
        return;
    }

#ifdef USE_MFC

#ifdef _WINDOWS
    char sz[255];
    static char BASED_CODE szTitle[] = "Assertion Failed!";
    static char BASED_CODE szMessage[] = "%s:\n\t%s\n File %s, Line %d";
    static char BASED_CODE szUnknown[] = "<unknown application>";

    // get app name or NULL if unknown (don't call assert)
#ifndef _AFXDLL
    const char *pszAppName = afxCurrentAppName;
#else
    const char *pszAppName = _AfxGetAppData()->appCurrentAppName;
#endif
    wsprintf(sz, (LPCSTR)szMessage,
             (pszAppName == NULL) ? (LPCSTR)szUnknown : (LPCSTR)pszAppName,
             lpszTheAssertion, lpszFileName, nLine);

    if (ufTraceEnabed) {
        // assume the debugger or auxiliary port
        ::OutputDebugString(sz);
        ::OutputDebugString(", ");
        ::OutputDebugString(szTitle);
        ::OutputDebugString("\n\r");
    }

retry:
    int nCode = ::MessageBox(
        NULL, sz, szTitle, MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);
    if (nCode == IDIGNORE) {
        return; // ignore
    } else if (nCode == IDRETRY) {
        // break into the debugger (or Dr Watson log)
#ifndef _PORTABLE
        _asm { int 3}
        ;
#endif
        goto retry;
    }
    // else fall through and call ufAbort

#else
    static char szMessage[] = "Assertion Failed:\n\t%s\n file %Fs, line %d\r\n";
    fprintf(stderr, szMessage, lpszTheAssertion, lpszFileName, nLine);
#endif // _WINDOWS

#else
    // parameters not used if non-debug
    (void)lpszTheAssertion;
    (void)lpszFileName;
    (void)nLine;
#endif // MFC

#endif // _DEBUG

    ufAbort();
}

void ufAbort() {
#ifdef USE_MFC
    AfxAbort();
#else
    abort();
#endif
}

void ufCheckMemory(const char FAR *lpszFilename, int nLine) {
#ifdef USE_MFC
    if (AfxCheckMemory() == 0) {
        ufAssertFailedLine("AfxCheckMemory() != 0", lpszFilename, nLine);
    }
#else
#endif
    (void)lpszFilename;
    (void)nLine;
}

void ufValidateAddress(const void FAR *ptr, unsigned int size) {
#ifdef USE_MFC
    AfxIsValidAddress((const void FAR *)ptr, size);
#endif
    (void)ptr;
    (void)size;
}
