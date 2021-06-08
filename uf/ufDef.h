/*
 * See Readme, Notice and License files.
 */
#ifndef _ufDef_h
#define _ufDef_h

// using namespace std;
#ifdef USE_MFC
#include <afx.h>
#else
#ifndef BOOL_DEFINED
typedef int BOOL;
#endif
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
#ifdef USE_STL_STREAMS
#include <fstream>
#include <iostream>
#endif

/* The ufCOMPARISON_MEMBER_OPERTORS & GLOBAL OPERATOR Macros below
 * add inline operators that call a Comparison method on the class.
 * the comparison method should return < 0, 0 or > 0 depending
 * upon whether the instance of the class is less than
 * equal to or greater to the 'OtherClass'.
 * The GLOBAL to MEMBER_FUNC Macro delegates the global operators
 * to a member function of Class2.
 *
 * ufCOMPARISON_MEMBER_OPERATORS(OtherClass, CompareFunc)
 * ufCOMPARISON_INLINE_GLOBAL_OPERATORS(Class1, Class2, CompareFunc)
 * ufCOMPARISON_INLINE_GLOBAL_OPERATORS_TO_MEMBER_FUNCS(Class1, Class2, Class2MemberFunc)
 *
 * The STREAM macros add member and global inline operators that
 * delegate calls to the ArchiveFUnc
 *
 * ufSTREAM_OUT_MEMBERS(Archive, Class, ArchiveFunc)
 * ufSTREAM_IN_MEMBERS(Archive, Class, ArchiveFunc)
 *
 * ufSTREAM_INLINE_GLOBAL_OUT(Archive, Class, ArchiveFunc)
 * ufSTREAM_INLINE_GLOBAL_IN(Archive, Class, ArchiveFunc)
 *
 */

// declares member function int CompareFunc(Class d) const in
// containing class a
#define ufCOMPARISON_MEMBER_OPERATORS(OtherClass, CompareFunc)               \
    BOOL operator==(OtherClass data) const {                                 \
        return CompareFunc(data) == 0;                                       \
    }                                                                        \
    BOOL operator!=(OtherClass data) const {                                 \
        return CompareFunc(data) != 0;                                       \
    }                                                                        \
    BOOL operator<(OtherClass data) const {                                  \
        return CompareFunc(data) < 0;                                        \
    }                                                                        \
    BOOL operator<=(OtherClass data) const {                                 \
        return CompareFunc(data) <= 0;                                       \
    }                                                                        \
    BOOL operator>(OtherClass data) const {                                  \
        return CompareFunc(data) > 0;                                        \
    }                                                                        \
    BOOL operator>=(OtherClass data) const {                                 \
        return CompareFunc(data) >= 0;                                       \
    }                                                                        \
    int CompareFunc(OtherClass data) const

#define ufCOMPARISON_INLINE_GLOBAL_OPERATORS(Class1, Class2, CompareFunc)    \
inline BOOL operator==(Class1 data1, Class2 data2) {                         \
    return CompareFunc(data1, data2) == 0;                                   \
}                                                                            \
inline BOOL operator!=(Class1 data1, Class2 data2) {                         \
    return CompareFunc(data1, data2) != 0;                                   \
}                                                                            \
inline BOOL operator<(Class1 data1, Class2 data2) {                          \
    return CompareFunc(data1, data2) < 0;                                    \
}                                                                            \
inline BOOL operator<=(Class1 data1, Class2 data2) {                         \
    return CompareFunc(data1, data2) <= 0;                                   \
}                                                                            \
inline BOOL operator>(Class1 data1, Class2 data2) {                          \
    return CompareFunc(data1, data2) > 0;                                    \
}                                                                            \
inline BOOL operator>=(Class1 data1, Class2 data2) {                         \
    return CompareFunc(data1, data2) > 0;                                    \
}

#define ufCOMPARISON_INLINE_GLOBAL_OPERATORS_TO_MEMBER_FUNCS(Class1, Class2, Class2MemberFunc)    \
inline BOOL operator==(Class1 data1, Class2 data2) {                         \
    return data2.Class2MemberFunc(data1) == 0;                               \
}                                                                            \
inline BOOL operator!=(Class1 data1, Class2 data2) {                         \
    return data2.Class2MemberFunc(data1) != 0;                               \
}                                                                            \
inline BOOL operator<(Class1 data1, Class2 data2) {                          \
    return data2.Class2MemberFunc(data1) >= 0;                               \
}                                                                            \
inline BOOL operator<=(Class1 data1, Class2 data2) {                         \
    return data2.Class2MemberFunc(data1) > 0;                                \
}                                                                            \
inline BOOL operator>(Class1 data1, Class2 data2) {                          \
    return data2.Class2MemberFunc(data1) <= 0;                               \
}                                                                            \
inline BOOL operator>=(Class1 data1, Class2 data2) {                         \
    return data2.Class2MemberFunc(data1) < 0;                                \
}

#ifdef USE_STL_STREAMS
// declares member function Archive WriteTo(Archive out) const
// and operator << that calls this member func
#define ufSTREAM_OUT_MEMBERS(Archive, Class, WriteTo)                        \
    friend Archive operator<<(Archive out, Class data);                      \
    Archive WriteTo(Archive out) const
    
// declares member function Archive ReadFrom(Archive in)
// and operator >> that calls this member func
#define ufSTREAM_IN_MEMBERS(Archive, Class, ReadFrom)                       \
    friend Archive operator>>(Archive in, Class data);                      \
    Archive ReadFrom(Archive in)
    
#define ufSTREAM_INLINE_GLOBAL_OUT(Archive, Class, ArchiveFunc)             \
inline Archive operator<<( Archive out, Class data ) {                      \
    return data.ArchiveFunc( out );                                         \
}

#define ufSTREAM_INLINE_GLOBAL_IN(Archive, Class, ArchiveFunc)             \
inline Archive operator>>( Archive in, Class data ) {                      \
    return data.ArchiveFunc( in );                                         \
}
#else
#define ufSTREAM_OUT_MEMBERS(Archive, Class, WriteTo)
#define ufSTREAM_IN_MEMBERS(Archive, Class, ReadFrom)
#define ufSTREAM_INLINE_GLOBAL_OUT(Archive, Class, ArchiveFunc)
#define ufSTREAM_INLINE_GLOBAL_IN(Archive, Class, ArchiveFunc)
#endif // USE_STL_STREAMS

#ifdef USE_MFC
class CArchive;
#define ufFRIEND_ARCHIVE_OPERATORS(Class)                                     \
    friend CArchive &operator<<(CArchive &out, const Class &);                \
    friend CArchive &operator>>(CArchive &in, Class &);
#define ufEXTERN_ARCHIVE_OPERATORS(Class)                                     \
    extern CArchive &operator<<(CArchive &out, const Class &);                \
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
class ufAllocator;
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

class DebugLog
#ifdef USE_STL_STREAMS
    : public std::ofstream 
#endif
    {
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
#define ufASSERT(f)                                                           \
    {                                                                         \
        static char BASED_CODE a_[] = #f;                                     \
        ((f) ? (void)0 : ::ufAssertFailedLine(a_, THIS_FILE, __LINE__));      \
    }
#else
#define ufASSERT(f)                                                           \
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

#define ufVALIDATE_REFPTR(ptr, size)                                          \
    ufValidateAddress((const void FAR *)ptr, size);

#define ufRETURN(c)                                                           \
    { ufCHECK_MEMORY return (c); }

#define ufLOG(className, method, args)                                        \
    {                                                                         \
        static char BASED_CODE n_[] = #className;                             \
        static char BASED_CODE t_] = # method ;                               \
        if (debugLog.IsValid())                                               \
            debugLog.TimeStamp(THIS_FILE, __LINE__, n_, f)                    \
                << args << DebugLog::fl << flush;                             \
    }

#define ufVERIFY(c) ufASSERT(c)

extern DebugLog debugLog;
#else

#define ufASSERT(f)
#define ufDEBUG_FILE
#define ufCHECK_MEMORY
#define ufVALIDATE_REFPTR(ptr, size)
#define ufRETURN(c)                                                           \
    { return (c); }
#define ufLOG(className, method, args)
#define ufVERIFY(c) c

#endif

// Allocates a char[] . Array may or may not be from the
// heap. If you need to delete[] it, call IsOnHeap() to
// check first.
class ufAllocator {
public:
    // return is the buf or 0. size is set to the actual buf size
    virtual char* Allocate(int& size, BOOL forceOnHeap);
    // return buf or 0. newSize is set to the new size
    virtual void  Reallocate(char*& buf, int oldSize, int& newSize);
    virtual BOOL  IsOnHeap(const char* buf);
    // return buf = 0
    virtual void  Free(char*& buf);

    static ufAllocator* CreateStandardAllocator(); // not thread safe
};

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
    int GetLength() const; // strnlen()

    void Reallocate(int newSize);

    BOOL IsEmpty();
    void MakeEmpty();

    BOOL ConcatBuf(const ufTmpBuf& other);
    BOOL Concat(const char* other);
    BOOL Concat(const char* other, int otherLen);
    BOOL Copy(const char* other); //strcopy
    BOOL Copy(const char* other, int otherLen);

    char& Buf(int index, int incrementalIncrease);

    char& operator [] (int index);

    ufTmpBuf& operator +=(ufTmpBuf& other) {
        ConcatBuf(other);
        return *this;
    }
    ufTmpBuf& operator +=(const char* other) {
        Concat(other);
        return *this;
    }

    // sets fBuf = 0 & fSize = 0. Returns
    // either old fBuf if fBuf was allocated on the heap
    // or ufStrDup(fBuf) if fBuf wasn't
    BOOL ReleaseBuffer(char*& buf);

    char *const fBuf;

    ufCOMPARISON_MEMBER_OPERATORS(const ufTmpBuf&, Compare);
    ufCOMPARISON_MEMBER_OPERATORS(const char*, Compare);
private:
    int fSize;

    ufTmpBuf();

    char*& BufForMod();
    void Allocate();
};

ufCOMPARISON_INLINE_GLOBAL_OPERATORS_TO_MEMBER_FUNCS(const char*, const ufTmpBuf&, Compare);

inline BOOL ufTmpBuf::IsEmpty() {
    return fBuf == 0 || *fBuf == '\0';
}

inline char& ufTmpBuf::Buf(int index, int incrementalIncrease)  {
    if (index + 20 >= fSize) {
        Reallocate(fSize + incrementalIncrease );
    }
    return fBuf[index];
}

inline char& ufTmpBuf::operator [] (int index)  {
    return Buf(index, 256);
}


#define ufARCHIVE(x)                                                          \
    {                                                                         \
        if (archive.IsStoring())                                              \
            archive << x;                                                     \
        else                                                                  \
            archive >> x;                                                     \
    }

#define ufARCHIVE_INT(x)                                                      \
    {                                                                         \
        WORD w_;                                                              \
        if (archive.IsStoring()) {                                            \
            w_ = WORD(x);                                                     \
            archive << w_;                                                    \
        } else {                                                              \
            archive >> w_;                                                    \
            x = int(w_);                                                      \
        }                                                                     \
    }

#define ufARCHIVE_ENUM(type, x)                                               \
    {                                                                         \
        long l_;                                                              \
        if (archive.IsStoring()) {                                            \
            l_ = x;                                                           \
            archive << l_;                                                    \
        } else {                                                              \
            archive >> l_;                                                    \
            x = type(l_);                                                     \
        }                                                                     \
    }

#define ufARCHIVE_LIST(type, x)                                               \
    {                                                                         \
        if (archive.IsStoring()) {                                            \
            long n_ = x.GetNumElements();                                     \
            archive << n_;                                                    \
            ufPtr(type) p_;                                                   \
                                                                              \
            for (ufListIter(type) i = x; i; i++) {                            \
                archive << (i.fData);                                         \
            }                                                                 \
        } else {                                                              \
            long n_;                                                          \
            archive >> n_;                                                    \
            ufPtr(type) p_;                                                   \
            for (int i = 0; i < n_; i++) {                                    \
                archive >> p_;                                                \
                if (p_) {                                                     \
                    x.InsertLast(p_);                                         \
                }                                                             \
            }                                                                 \
        }                                                                     \
    }

UF_API(void) ufTmpBufInit();
UF_API(void) ufTmpBufDestroy();

extern void CleanupInput();
extern void CleanupOutput();

#ifndef NEW
#define NEW new
#endif
#ifndef BASED_CODE
#define BASED_CODE
#endif

#ifdef TEST

#define ufTEST(x) std::cout << x << std::endl << std::flush

#define ufTEST_STMT(x) std::cout << # x << std::endl << std::flush; x

#define ufVERIFY_RESULT(isVal, expected, numFailures)                         \
    if (isVal != expected) {                                                  \
        std::cerr << __FILE__ << ":" << __LINE__ << ":  TEST FAILED: "        \
                  << #isVal << " != " << std::boolalpha << expected << "\n"   \
                  << std::flush;                                              \
        numFailures++;                                                        \
    } else {                                                                  \
      std::cout << " Verified: " << #isVal << " == " << std::boolalpha        \
                << expected << std::endl << std::flush;                       \
    }

#endif

#endif

