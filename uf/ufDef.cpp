/*
 * See Readme, Notice and License files.
 */
#include "ufDef.h"
#include "ufString.h"

#include <string.h>
#include <iostream>

ufDEBUG_FILE

#define BA_Quote(a) #a

static ufAllocator* STANDARD_ALLOCATOR;

const int nTmpBufSize = 250;
const int nNumOfTmpBufs = 10;

struct TmpBuf_ {
    BOOL fInUse;
    char fBuf[nTmpBufSize];
};

static char*   lpFirstBufEntry;
static char*   lpLastBufEntry;
static TmpBuf_ *lpBuffers;

inline long indexForBuf(const char* buf) {
    const char* lpBuffersAsChar = (char*)lpBuffers;
    long num = buf > lpBuffersAsChar ?
        (buf - lpBuffersAsChar) / sizeof(TmpBuf_) : -1 ;
    return num;
}

UF_API(void)
ufTmpBufInit() {
    STANDARD_ALLOCATOR = new ufAllocator();
    lpBuffers = NEW TmpBuf_[nNumOfTmpBufs];
    memset(lpBuffers, 0, sizeof(TmpBuf_) * nNumOfTmpBufs);
    lpFirstBufEntry = (char*)lpBuffers;
    lpLastBufEntry = ((char*)(&lpBuffers[nNumOfTmpBufs])) - 1;
}

UF_API(void) ufTmpBufDestroy() {
    delete[] lpBuffers;
    lpBuffers = 0;
    delete STANDARD_ALLOCATOR;
    STANDARD_ALLOCATOR = 0;
}

char* ufAllocator::Allocate(int& size, BOOL forceOnHeap) {
    ufCHECK_MEMORY

    ufASSERT(lpBuffers)
    ufASSERT(size > 0)

    if (lpBuffers == 0 || size <= 0) {
        std::cerr << "Corrupted state. Allocating size=" << size
                  << " lpBuffers=" << lpBuffers
                  << ". Did you call ufTmpBufInit()?"
                  << std::endl;
        exit(16);
    }

    if (!forceOnHeap && size < nTmpBufSize) {
        for (int i = 0; i < nNumOfTmpBufs; i++) {
            if (lpBuffers[i].fInUse == TRUE) {
                continue;
            }
            lpBuffers[i].fInUse = TRUE;
            char* buf = lpBuffers[i].fBuf;
            size = nTmpBufSize;
            buf[0] = buf[size - 1] = '\0';
            ufASSERT(buf)
            ufCHECK_MEMORY
            return buf;
        }
    }

    char* buf = NEW char[size];
    if (buf) {
        buf[0] = buf[size - 1] = '\0';
    } else {
        std::cerr << "Out of memory. Allocating size=" << size
                  << std::endl;
        exit(17);
    }

    ufASSERT(buf)
    ufCHECK_MEMORY
    return buf;
}

void ufAllocator::Reallocate(char*& buf, int oldSize, int& newSize) {
    ufCHECK_MEMORY
    ufASSERT(lpBuffers)
    ufASSERT(newSize > 0)
    char* oldBuf = buf;

    // if this was from our pre-allocated bufs, ensure the size is correct
    if (!IsOnHeap(oldBuf)) {
        ufASSERT(oldSize == nTmpBufSize)
        if (newSize <= nTmpBufSize) {
            newSize = nTmpBufSize;
            // nothing to be done. 
            return;
        }
    }
    if (newSize <= oldSize) {
        newSize = oldSize;
        // nothing to be done. 
        return;
    }

    buf = Allocate(newSize, /*forceOnHeap=*/TRUE);
    if (buf) {
        memcpy(buf, oldBuf, oldSize);
    }
    Free(oldBuf);

    ufASSERT(buf)
    ufCHECK_MEMORY
}

BOOL  ufAllocator::IsOnHeap(const char* buf) {
    BOOL result = buf < lpFirstBufEntry || buf > lpLastBufEntry;
    return result;
}

void  ufAllocator::Free(char*& buf) {
    ufCHECK_MEMORY

    if (IsOnHeap(buf)) {
        delete[] buf;
    } else {
        long num = indexForBuf(buf);
        if ( num < 0 || num >= nNumOfTmpBufs) {
            std::cerr << "Corrupted memory. Item not falling within lpBuffer"
                      << std::endl;
            exit(18);
        }
        lpBuffers[num].fInUse = FALSE;
    }

    buf = 0;
    ufCHECK_MEMORY
}


ufAllocator* ufAllocator::CreateStandardAllocator() {
    return STANDARD_ALLOCATOR;
}


ufTmpBuf::ufTmpBuf() :
  fBuf(0),
  fSize(0) {
}

ufTmpBuf::ufTmpBuf(int nSize) :
  fBuf(0),
  fSize(nSize) {
    Allocate();
}

ufTmpBuf::ufTmpBuf(const ufTmpBuf &o) :
  fBuf(0),
  fSize(o.fSize) {
    Allocate();
    if (fBuf != 0) {
        memcpy(fBuf, o.fBuf, o.fSize);
    }
}

ufTmpBuf::~ufTmpBuf() {
    MakeEmpty();
}

inline char*& ufTmpBuf::BufForMod() { return (char*&)fBuf; }

void ufTmpBuf::MakeEmpty() {
    ufCHECK_MEMORY

    STANDARD_ALLOCATOR->Free(BufForMod());
    fSize = 0;

    ufCHECK_MEMORY
}

BOOL ufTmpBuf::ReleaseBuffer(char*& buf) {
    ufCHECK_MEMORY
    if (fBuf == 0) {
        buf = 0;
        return FALSE;
    }

    if (STANDARD_ALLOCATOR->IsOnHeap(fBuf)) {
        buf = fBuf;
    } else {
        buf = ufStrDup(fBuf);
        STANDARD_ALLOCATOR->Free(BufForMod());
    }

    BufForMod() = 0;
    fSize = 0;

    ufCHECK_MEMORY
    return TRUE;
}

void ufTmpBuf::Allocate() {
    BufForMod() = STANDARD_ALLOCATOR->Allocate(fSize, /*forceHeap=*/FALSE);
}

void ufTmpBuf::Reallocate(int nSize) {
    STANDARD_ALLOCATOR->Reallocate(BufForMod(), fSize, nSize);
    fSize = nSize;
}

int ufTmpBuf::GetLength() const {
    return fBuf != 0 ? strnlen(fBuf, GetSize()) : 0;
}

BOOL ufTmpBuf::ConcatBuf(const ufTmpBuf &s) {
    return Concat(s.fBuf);
}

BOOL ufTmpBuf::Concat(const char* s) {
    int l2 = s != 0 ? strlen(s) : 0;
    return Concat(s, l2);
}

BOOL ufTmpBuf::Concat(const char* s, int l2/*=strlen(s)*/) {
    int l1 = fBuf ? strlen(fBuf) : 0;

    if (l1 + l2 + 20 > GetSize()) {
        Reallocate(l1 + l2 + 1024);
    }
    if (fBuf != 0 && s != 0) {
        strncat(fBuf, s, l1 + l2 + 10);
        fBuf[l1 + l2] = '\0';
        return TRUE;
    }
    return FALSE;
}

BOOL ufTmpBuf::Copy(const char* s) {
    fBuf[0]='\0';
    int l2 = s != 0 ? strlen(s) : 0;
    return Concat(s, l2);
}

BOOL ufTmpBuf::Copy(const char* s, int l2/*=strlen(s)*/) {
    fBuf[0]='\0';
    return Concat(s, l2);
}

int ufTmpBuf::Compare(const ufTmpBuf &tmp) const {
    return Compare(tmp.fBuf);
}

int ufTmpBuf::Compare(const char* lpszString) const {
    const char* a = fBuf != 0 ? fBuf : "";
    const char* b = lpszString != 0 ? lpszString : "";
    if (((void*)(fBuf)) == ((void*)(lpszString))
        || (*a == '\0' && *b == '\0')) {
        return 0;
    }

    int res = strncmp(a, b, fSize);
    return res;
}

const char *BASED_CODE DebugLog::fl = "\n";

/*DebugLog debugLog;*/
static const char FAR *lpszHeader = 0;

DebugLog::DebugLog()
    :
#ifdef USE_STL_STREAMS
    std::ofstream(BA_Quote(PACKAGE_NAME) ".log"),
#endif
    fIsValid(TRUE) {}

DebugLog::~DebugLog() { fIsValid = FALSE; }

BOOL DebugLog::IsValid() { return fIsValid; }

void DebugLog::SetHeader(const char FAR *lpszHdr) { lpszHeader = lpszHdr; }

DebugLog &DebugLog::TimeStamp(const char FAR *lpszFilename,
                              unsigned long ulLineNo,
                              const char *lpszClassName,
                              const char *lpszMethod) {
#ifdef USE_STL_STREAMS
    std::ostream& out = *this;
#else
    std::ostream& out = std::cout;
#endif
    
    if (lpszHeader) {
        out << lpszHeader;
    }
    out << "(" << lpszFilename << "," << ulLineNo << ")" << lpszClassName
          << "::" << lpszMethod << "(): ";

    return *this;
}

#ifdef ufDefTEST

int main(int argc, char *argv[]) {
    ufTmpBufInit();
    std:: cout << "TESTING ufTmpBuf and ufAllocator " << std::endl;
    int numFailures = 0;
    ufAllocator* alloc = STANDARD_ALLOCATOR;
    {
        ufTEST("indexForBuf");
        for (long i =0; i < nNumOfTmpBufs ; i++) {
            char* buf = lpBuffers[i].fBuf;
            long num = indexForBuf(buf);
            ufVERIFY_RESULT(i == num, TRUE, numFailures);
        }
    }
    {
        ufTEST("indexForBuf");
        int size = nTmpBufSize - 1;
        char*b = alloc->Allocate(size, /*forceOnHeap=*/TRUE);
        long num = indexForBuf(b);
        ufVERIFY_RESULT( num < 0 || num >= nNumOfTmpBufs, TRUE, numFailures);
        alloc->Free(b);
    }

    {
        ufTEST("ufAllocator(tmpbuf)");
        int size = nTmpBufSize - 1;
        char* b = alloc->Allocate(size, /*forceOnHeap=*/FALSE);
        ufVERIFY_RESULT(b != 0, TRUE, numFailures);
        ufVERIFY_RESULT(alloc->IsOnHeap(b), FALSE, numFailures);
        ufTEST_STMT(alloc->Free(b));
        ufVERIFY_RESULT(b == 0, TRUE, numFailures);
    }

    {
        static char* b[ nNumOfTmpBufs + 2 ];
        ufTEST("ufAllocator(tmpbuf)");
        for (int i = 0 ; i < sizeof(b) / sizeof(*b); i++) {
            std::cout << "checking b[i=" << i << "]" << std::endl;
            int size = nTmpBufSize - 1;
            b[i] = alloc->Allocate(size, /*forceOnHeap=*/FALSE);
            std::cout << "calculated index = b[i=" << indexForBuf(b[i]) << "]" << std::endl;
            ufVERIFY_RESULT(b[i] != 0, TRUE, numFailures);
            // b shouldn't be on the heap except at the very last
            ufVERIFY_RESULT(alloc->IsOnHeap(b[i]), (i >= nNumOfTmpBufs), numFailures);
        }
        for (int i = 0 ; i < sizeof(b) / sizeof(*b); i++) {
            std::cout << "checking b[i=" << i << "]" << std::endl;
            ufVERIFY_RESULT(alloc->IsOnHeap(b[i]), (i >= nNumOfTmpBufs), numFailures);
            alloc->Free(b[i]);
            ufVERIFY_RESULT(b[i] == 0, TRUE, numFailures);
        }
    }

    {
        ufTEST("ufAllocator(tmpbuf)");
        int size = nTmpBufSize - 1;
        char* b = alloc->Allocate(size, /*forceOnHeap=*/TRUE);
        ufVERIFY_RESULT(b != 0, TRUE, numFailures);
        ufVERIFY_RESULT(alloc->IsOnHeap(b), TRUE, numFailures);
        ufTEST_STMT(alloc->Free(b));
        ufVERIFY_RESULT(b == 0, TRUE, numFailures);
    }

    std::cout << "TEST DONE. Total Failures: " << numFailures << "\n";

    ufTmpBufDestroy();
    return (numFailures);
}

#endif
