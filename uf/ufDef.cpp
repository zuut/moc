/*
 * See Readme, Notice and License files.
 */
#include "ufDef.h"
#include <iostream>
#include <string.h>

ufDEBUG_FILE

#define BA_Quote(a) #a

    const int nTmpBufSize = 250;
const int nNumOfTmpBufs = 10;

struct TmpBuf_ {
    BOOL fInUse;
    char fBuf[nTmpBufSize];
};

static TmpBuf_ *lpBuffers;

UF_API(void)
ufTmpBufInit() {
    lpBuffers = NEW TmpBuf_[nNumOfTmpBufs];
    memset(lpBuffers, 0, sizeof(TmpBuf_) * nNumOfTmpBufs);
}

UF_API(void) ufTmpBufDestroy() {
    delete[] lpBuffers;
    lpBuffers = 0;
}

ufTmpBuf::ufTmpBuf() : fBuf(0), fNum(-1), fSize(0) {}

ufTmpBuf::ufTmpBuf(int nSize) : fBuf(0), fNum(-1), fSize(nSize) { Allocate(); }

ufTmpBuf::ufTmpBuf(const ufTmpBuf &o) : fBuf(0), fNum(-1), fSize(o.fSize) {
    Allocate();
}

ufTmpBuf::~ufTmpBuf() {
    ufCHECK_MEMORY

        if (fNum == -1) {
        delete[]((char *)fBuf);
        ufCHECK_MEMORY return;
    }
    lpBuffers[fNum].fInUse = FALSE;

    ufCHECK_MEMORY
}

void ufTmpBuf::Allocate() {
    ufCHECK_MEMORY

    ufASSERT(lpBuffers) ufASSERT(fNum == -1) ufASSERT(
        fSize > 0) if (lpBuffers == nullptr || fNum != -1 || fSize <= 0) {
        std::cerr << "Corrupted state. Allocating size=" << fSize
                  << " fNum=" << fNum << " lpBuffers=" << lpBuffers
                  << std::endl;
        exit(16);
    }

    if (fSize > nTmpBufSize) {
        ((char *&)fBuf) = NEW char[fSize];
        if (fBuf) {
            fBuf[0] = fBuf[fSize - 1] = '\0';
        } else {
            std::cerr << "Out of memory. Allocating size=" << fSize
                      << std::endl;
            exit(17);
        }
        ufCHECK_MEMORY return;
    }

    for (int i = 0; i < nNumOfTmpBufs; i++) {
        if (lpBuffers[i].fInUse == TRUE) {
            continue;
        }
        fNum = i;
        lpBuffers[i].fInUse = TRUE;
        ((char *&)fBuf) = lpBuffers[i].fBuf;
        fBuf[0] = fBuf[nTmpBufSize - 1] = '\0';
        ufCHECK_MEMORY return;
    }

    ((char *&)fBuf) = NEW char[fSize];
    ufASSERT(fBuf)

        ufCHECK_MEMORY
}

void ufTmpBuf::Reallocate(int nSize) {
    ufCHECK_MEMORY

    ufASSERT(lpBuffers)

        ufASSERT(nSize > 0)

            if (nSize > nTmpBufSize) {
        if (nSize > fSize) {
            ufTmpBuf old;
            ((char *&)old.fBuf) = fBuf;
            old.fNum = fNum;
            old.fSize = fSize;
            fSize = nSize;
            ((char *&)fBuf) = 0;
            fNum = -1;
            Allocate();
            ufASSERT(fBuf) if (fBuf) { memcpy(fBuf, old.fBuf, old.fSize); }
            // let old clean up itself.
        }

        ufCHECK_MEMORY return;
    }

    fSize = nSize;

    ufASSERT(fBuf)

        ufCHECK_MEMORY
}

const char *BASED_CODE DebugLog::fl = "\n";

/*DebugLog debugLog;*/
static const char FAR *lpszHeader = 0;

DebugLog::DebugLog()
    : std::ofstream(BA_Quote(PACKAGE_NAME) ".log"), fIsValid(TRUE) {}

DebugLog::~DebugLog() { fIsValid = FALSE; }

BOOL DebugLog::IsValid() { return fIsValid; }

void DebugLog::SetHeader(const char FAR *lpszHdr) { lpszHeader = lpszHdr; }

DebugLog &DebugLog::TimeStamp(const char FAR *lpszFilename,
                              unsigned long ulLineNo, const char *lpszClassName,
                              const char *lpszMethod) {
    if (lpszHeader) {
        *this << lpszHeader;
    }
    *this << "(" << lpszFilename << "," << ulLineNo << ")" << lpszClassName
          << "::" << lpszMethod << "(): ";
    return *this;
}
