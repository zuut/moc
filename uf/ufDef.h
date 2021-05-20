/*
 * See Readme, Notice and License files.
 */
#ifndef _ufDef_h
#define _ufDef_h

// using namespace std;

#ifdef USE_MFC
#include <afx.h>
#else
typedef int BOOL;
#endif

#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#include "generic.h"
#include <fstream>
#include <iostream>

#ifdef USE_MFC
class CArchive;
#define ufFRIEND_ARCHIVE_OPERATORS(Class)                                      \
    friend CArchive &operator<<(CArchive &out, const Class &);                 \
    friend CArchive &operator>>(CArchive &in, Class &);
#define ufEXTERN_ARCHIVE_OPERATORS(Class)                                      \
    extern CArchive &operator<<(CArchive &out, const Class &);                 \
    extern CArchive &operator>>(CArchive &in, Class &);
#else
#define ufFRIEND_ARCHIVE_OPERATORS(Class)
#define ufEXTERN_ARCHIVE_OPERATORS(Class)
#endif

/*
 * This file defines the following
 *      CONSTANTS
 *              const char dirSepChar = '\\';
 *              extern const char dirSepString[] //"\\"
 *      MACROS
 *              PTR_INT
 *              UF_API
 *              ASSERT( <boolean condition> )
 *              DEBUG_FILE
 *              CHECK_MEMORY
 *              VALIDATE_REFPTR(ptr,size)
 *              RETURN(c);
 *              LOG(className,method,args)
 *              ARCHIVE(x);
 *              ARCHIVE_STR(x);
 *              ARCHIVE_STRPTR(x);
 *              ARCHIVE_INT(x);
 *              ARCHIVE_ENUM(type,x);
 *              ARCHIVE_LIST(type,x);
 *      classes
 */
class DebugLog;

class ufTmpBuf;
/*     global instances
 *              DebugLog debugLog;
 *      functions
 *              int     isDirSep(char c)
 *              char    *ufStrDupLen(const char *lpszString,
 *                                   int *len);
 *              char    *ufStrDup(const char *lpszString);
 */
#ifndef PTR_INT
#define PTR_INT long
#endif

#ifndef UF_API
//#define UF_API(type) extern "C" type __far __export __pascal
//#define UF_API(type) extern "C" type __export
#define UF_API(type) extern "C" type
#endif

class DebugLog : public std::ofstream {
public:
    DebugLog();

    ~DebugLog();

    BOOL IsValid();

    void SetHeader(const char FAR *lpszHdr);

    DebugLog &TimeStamp(const char FAR *lpszFilename, unsigned long ulLineNo,
                        const char *lpszClassName, const char *lpszMethod);

    static const char *fl;

protected:
    BOOL fIsValid;
};

#ifdef _DEBUG
extern "C" {
extern void PASCAL ufAssertFailedLine(const char FAR *lpszAssertion,
                                      const char FAR *lpszFilename, int nLine);
extern void PASCAL ufAssertFailedLineSimple(const char FAR *lpszFilename,
                                            int nLine);
extern void ufCheckMemory(const char FAR *lpszFilename, int nLine);
extern void ufValidateAddress(const void FAR *ptr, unsigned int size);
}

#define _ASSERT_VERBOSE

#ifdef _ASSERT_VERBOSE
#define ufASSERT(f)                                                            \
    {                                                                          \
        static char BASED_CODE a_[] = #f;                                      \
        ((f) ? (void)0 : ::ufAssertFailedLine(a_, THIS_FILE, __LINE__));       \
    }
#else
#define ufASSERT(f)                                                            \
    ((f) ? (void)0 : ::ufAssertFailedLineSimple(THIS_FILE, __LINE__));
#endif

#ifdef USE_MFC
#undef THIS_FILE
#define NEW DEBUG_NEW
#define ufDEBUG_FILE static char BASED_CODE THIS_FILE[] = __FILE__;
#else
#ifndef THIS_FILE
#define THIS_FILE __FILE__
#endif
#define ufDEBUG_FILE
#define NEW new
#endif

#define ufCHECK_MEMORY ufCheckMemory(THIS_FILE, __LINE__);

#define ufVALIDATE_REFPTR(ptr, size)                                           \
    ufValidateAddress((const void FAR *)ptr, size);

#define ufRETURN(c)                                                            \
    { ufCHECK_MEMORY return (c); }

#define ufLOG(className, method, args)                                         \
    {                                                                          \
        static char BASED_CODE n_[] = #className;                              \
        static char BASED_CODE t_] = # method ;                                \
        if (debugLog.IsValid())                                                \
            debugLog.TimeStamp(THIS_FILE, __LINE__, n_, f)                     \
                << args << DebugLog::fl << flush;                              \
    }

#define ufVERIFY(c) ufASSERT(c)

extern DebugLog debugLog;
#else

#define ufASSERT(f)
#define ufDEBUG_FILE
#define ufCHECK_MEMORY
#define ufVALIDATE_REFPTR(ptr, size)
#define ufRETURN(c)                                                            \
    { return (c); }
#define ufLOG(className, method, args)
#define ufVERIFY(c) c

#endif

// This class exists because Windows would crash would
// creating large char arrays on the stack. This allows
// us to have bufs that are deallocated when the
// code blocks exits their scope.
class ufTmpBuf {
public:
    ufTmpBuf(int size);

    ufTmpBuf(const ufTmpBuf &);

    ~ufTmpBuf();

    int GetSize() const { return fSize; }

    void Reallocate(int newSize);

    char *const fBuf;

private:
    ufTmpBuf();
    void Allocate();

    int fNum; // if == -1, this buf was allocated on the heap. if >-1, its from
              // an array of bufs
    int fSize;
};

#define ufARCHIVE(x)                                                           \
    {                                                                          \
        if (archive.IsStoring())                                               \
            archive << x;                                                      \
        else                                                                   \
            archive >> x;                                                      \
    }

#define ufARCHIVE_INT(x)                                                       \
    {                                                                          \
        WORD w_;                                                               \
        if (archive.IsStoring()) {                                             \
            w_ = WORD(x);                                                      \
            archive << w_;                                                     \
        } else {                                                               \
            archive >> w_;                                                     \
            x = int(w_);                                                       \
        }                                                                      \
    }

#define ufARCHIVE_ENUM(type, x)                                                \
    {                                                                          \
        long l_;                                                               \
        if (archive.IsStoring()) {                                             \
            l_ = x;                                                            \
            archive << l_;                                                     \
        } else {                                                               \
            archive >> l_;                                                     \
            x = type(l_);                                                      \
        }                                                                      \
    }

#define ufARCHIVE_LIST(type, x)                                                \
    {                                                                          \
        if (archive.IsStoring()) {                                             \
            long n_ = x.GetNumElements();                                      \
            archive << n_;                                                     \
            ufPtr(type) p_;                                                    \
                                                                               \
            for (ufListIter(type) i = x; i; i++) {                             \
                archive << (i.fData);                                          \
            }                                                                  \
        } else {                                                               \
            long n_;                                                           \
            archive >> n_;                                                     \
            ufPtr(type) p_;                                                    \
            for (int i = 0; i < n_; i++) {                                     \
                archive >> p_;                                                 \
                if (p_) {                                                      \
                    x.InsertLast(p_);                                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }

UF_API(void) ufTmpBufInit();
UF_API(void) ufTmpBufDestroy();

#ifndef NEW
#define NEW new
#endif
#ifndef BASED_CODE
#define BASED_CODE
#endif

#endif
