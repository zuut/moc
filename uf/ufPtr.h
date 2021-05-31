/*
 * See Readme, Notice and License files.
 */
#ifndef _ufPtr_h
#define _ufPtr_h

#include "ufDef.h"

/*
 * The smart pointer class provides memory management
 * of the instances. It is used as follows:
 *
 * ... file <foo.h> ....
 * #include <ufPtr.h>
 * class foo {
 *   ...
 * };
 *
 * declare(ufPtr,foo);
 * ......................
 *
 * .....foo.c............
 *
 * #include <foo.h>
 *
 * implement(ufPtr,foo);
 * ......................
 *
 * Finally, in some source file which uses this, ...
 *
 * #include <foo.h>
 * bar()
 * {
 *      ufPtr(foo)  myptr = foo::New(....);
 *
 *      myptr->FooMemberFunc(..);
 *
 *      delete myptr;
 */

#define ufPtr(TYPE) name2(TYPE, PTR)

class ufPtr(void);

class ufGenericPtr;

#ifdef _DEBUG
#define ufASSERT_VALID_PTR(TYPE)                                               \
    ufPtrAssertValidPtr_(This(), sizeof(TYPE), DeleteRealPtr);
#define ufASSERT_VALID_GENERIC_PTR()                                           \
    ufPtrAssertValidPtr_(this, sizeof(char), DeleteRealPtr);
#else
#define ufASSERT_VALID_PTR(TYPE)
#define ufASSERT_VALID_GENERIC_PTR()

#endif

const int kSharingPtr = 3;    // reference counted
const int kSelfPtrShared = 5; // don't call delete when destroyed

#define ufPtrdeclareclass(TYPE)                                               \
                                                                              \
    class ufPtr(TYPE) {                                                       \
        friend class TYPE;                                                    \
                                                                              \
    public:                                                                   \
        inline ufPtr(TYPE)();                                                 \
        inline ufPtr(TYPE)(const ufPtr(TYPE) & smartPtr);                     \
        virtual ~ufPtr(TYPE)();                                               \
                                                                              \
        inline void SwapWith(ufPtr(TYPE) & o);                                \
        inline void MakeEmpty();                                              \
        inline BOOL isNull() const;                                           \
        inline BOOL matchingRefCount(int refCnt) const;                       \
        inline BOOL isValid() const;                                          \
        TYPE *operator->() const;                                             \
        TYPE &operator*() const;                                              \
        ufPtr(TYPE) &operator=(const ufPtr(TYPE) & smartPtr);                 \
        ufPtr(TYPE) &operator=(int zero);                                     \
        operator TYPE &() const;                                              \
        inline PTR_INT GetPointerAsInt() const;                               \
        BOOL operator==(void *) const;                                        \
        inline BOOL operator!=(void *) const;                                 \
        int operator!() const;                                                \
        ufFRIEND_ARCHIVE_OPERATORS(ufPtr(TYPE)) protected                     \
            : ufPtr(TYPE)(TYPE * data);                                       \
        ufPtr(TYPE)(TYPE * data, int relationship);                           \
        ufPtr(TYPE)(int relationship);                                        \
        void SetPtr(TYPE *lpRealPtr);                                         \
        static void DeleteRealPtr(ufGenericPtr *lpRealPtr);                   \
        TYPE *fRealPtr;                                                       \
        int *fRefCnt;                                                         \
        int fRelationship;                                                    \
                                                                              \
    public: /*implemenation*/                                                 \
        ufPtr(void) * This() const { return (ufPtr(void) *)(void *)this; }    \
    };

#define ufPtrdeclareinlines(TYPE)                                             \
                                                                              \
    inline ufPtr(TYPE)::ufPtr(TYPE)()                                         \
        : fRealPtr(0), fRefCnt(0), fRelationship(kSharingPtr) {}              \
                                                                              \
    inline void ufPtr(TYPE)::SwapWith(ufPtr(TYPE) & o) {                      \
        ufPtrSwapWith_(This(), (ufPtr(void) *)&o);                            \
    }                                                                         \
                                                                              \
    inline ufPtr(TYPE)::ufPtr(TYPE)(int relationship)                         \
        : fRealPtr(0), fRefCnt(0), fRelationship(relationship) {}             \
                                                                              \
    inline ufPtr(TYPE)::ufPtr(TYPE)(const ufPtr(TYPE) & sptr) {               \
        ufPtrConstruct_(This(), sizeof(TYPE), DeleteRealPtr, sptr.This());    \
    }                                                                         \
                                                                              \
    inline void ufPtr(TYPE)::MakeEmpty() {                                    \
        ufPtrMakeEmpty_(This(), sizeof(TYPE), DeleteRealPtr);                 \
    }                                                                         \
                                                                              \
    inline BOOL ufPtr(TYPE)::isNull() const { return fRealPtr == 0; }         \
                                                                              \
    inline BOOL ufPtr(TYPE)::matchingRefCount(int refCnt) const {             \
        return (fRefCnt == 0 && refCnt == 0) ||                               \
               (fRefCnt != 0 && refCnt == *fRefCnt);                          \
    }                                                                         \
                                                                              \
    inline BOOL ufPtr(TYPE)::isValid() const { return fRealPtr != 0; }        \
                                                                              \
    inline TYPE *ufPtr(TYPE)::operator->() const {                            \
        ufASSERT_VALID_PTR(TYPE) ufASSERT(fRealPtr != 0)                      \
            ufASSERT(fRefCnt != 0) ufASSERT(*fRefCnt != 0) return fRealPtr;   \
    }                                                                         \
                                                                              \
    inline TYPE &ufPtr(TYPE)::operator*() const {                             \
        ufASSERT_VALID_PTR(TYPE) return *fRealPtr;                            \
    }                                                                         \
                                                                              \
    inline ufPtr(TYPE) & ufPtr(TYPE)::operator=(const ufPtr(TYPE) & sptr) {   \
        ufPtrAssignPtr_(This(), sizeof(TYPE), DeleteRealPtr, sptr.This());    \
        return *this;                                                         \
    }                                                                         \
                                                                              \
    inline ufPtr(TYPE) & ufPtr(TYPE)::operator=(int zero) {                   \
        ufPtrAssignInt_(This(), sizeof(TYPE), DeleteRealPtr, zero);           \
        return *this;                                                         \
    }                                                                         \
                                                                              \
    inline ufPtr(TYPE)::operator TYPE &() const {                             \
        ufASSERT_VALID_PTR(TYPE) return *fRealPtr;                            \
    }                                                                         \
                                                                              \
    inline BOOL ufPtr(TYPE)::operator==(void *x) const {                      \
        return ((void *)fRealPtr) == x;                                       \
    }                                                                         \
                                                                              \
    inline PTR_INT ufPtr(TYPE)::GetPointerAsInt() const {                     \
        return (PTR_INT)fRealPtr;                                             \
    }                                                                         \
                                                                              \
    inline BOOL ufPtr(TYPE)::operator!=(void *x) const {                      \
        return ((void *)fRealPtr) != x;                                       \
    }                                                                         \
                                                                              \
    inline int ufPtr(TYPE)::operator!() const { return !fRealPtr; }           \
                                                                              \
    inline void ufPtr(TYPE)::SetPtr(TYPE *lpNewPtr) {                         \
        ufPtrSetPtr_(This(), sizeof(TYPE), DeleteRealPtr,                     \
                     (ufGenericPtr *)lpNewPtr);                               \
    }

#define ufPtrdeclare(TYPE) ufPtrdeclareclass(TYPE) ufPtrdeclareinlines(TYPE)

#define ufPtrimplement(TYPE)                                                  \
                                                                              \
    ufPtr(TYPE)::ufPtr(TYPE)(TYPE * lpData, int relationship)                 \
        : fRealPtr(lpData), fRelationship(relationship) {                     \
        if (fRealPtr == 0 || (fRefCnt = ufNewInt_()) == 0) {                  \
            fRealPtr = 0;                                                     \
            fRefCnt = 0;                                                      \
            return;                                                           \
        }                                                                     \
        *fRefCnt = (fRelationship != kSelfPtrShared ? 1 : 0);                 \
    }                                                                         \
                                                                              \
    ufPtr(TYPE)::ufPtr(TYPE)(TYPE * lpData)                                   \
        : fRealPtr(lpData), fRelationship(kSharingPtr) {                      \
        if (fRealPtr == 0 || (fRefCnt = ufNewInt_()) == 0) {                  \
            fRealPtr = 0;                                                     \
            fRefCnt = 0;                                                      \
            return;                                                           \
        }                                                                     \
        *fRefCnt = 1;                                                         \
    }                                                                         \
                                                                              \
    ufPtr(TYPE)::~ufPtr(TYPE)() {                                             \
        MakeEmpty();                                                          \
        ufCHECK_MEMORY                                                        \
    }                                                                         \
                                                                              \
    void ufPtr(TYPE)::DeleteRealPtr(ufGenericPtr *d) {                        \
        delete ((TYPE *)d);                                                   \
    }

typedef void (*ufPtrDeleteFnc)(ufGenericPtr *);

UF_API(void)
ufPtrConstruct_(ufPtr(void) * lpSelf, unsigned uSize,
                ufPtrDeleteFnc lpfnDelFnc,
                const ufPtr(void) * lpOtherPtr);
UF_API(void)
ufPtrMakeEmpty_(ufPtr(void) * lpSelf, unsigned uSize,
                ufPtrDeleteFnc lpfnDelFnc);
UF_API(void)
ufPtrAssertValidPtr_(const ufPtr(void) * lpSelf, unsigned uSize,
                     ufPtrDeleteFnc lpfnDelFnc);
UF_API(void)
ufPtrSetPtr_(ufPtr(void) * lpSelf, unsigned uSize, ufPtrDeleteFnc lpfnDelFnc,
             ufGenericPtr *lpNewPtr, BOOL bDeleteOldRealPtr = TRUE);
UF_API(void)
ufPtrAssignInt_(ufPtr(void) * lpSelf, unsigned uSize,
                ufPtrDeleteFnc lpfnDelFnc,
                int nZero);
UF_API(void)
ufPtrAssignPtr_(ufPtr(void) * lpSelf, unsigned uSize,
                ufPtrDeleteFnc lpfnDelFnc,
                const ufPtr(void) * lpOtherPtr);
UF_API(void)
ufPtrAssignToEmptyPtr_(ufPtr(void) * lpSelf, const ufPtr(void) * lpOther);

UF_API(void)
ufPtrMoveToEmptyPtr_(ufPtr(void) * lpSelf, ufPtr(void) * lpOtherPtr);
UF_API(void) ufPtrClear_(ufPtr(void) * lpSelf);
UF_API(void) ufPtrSwapWith_(ufPtr(void) * lpSelf, ufPtr(void) * lpOther);

UF_API(int *) ufNewInt_();
UF_API(void) ufDeleteInt_(int *);

/* The ufPtr(void) class must be handled slightly differently
 * since void is not a class.
 *
 * declare(ufPtr,void);
 */

class ufPtr(void) {
public:
    ufPtr(void)();

    ufPtr(void)(const ufPtr(void) & smartPtr);

    virtual ~ufPtr(void)();

    void SwapWith(ufPtr(void) & o);

    void MakeEmpty();
    // THESE FUNCTIONS ARE NOT ALLOWED FOR void*
    //      void                    *operator -> () const;
    //      void                    *operator *() const;
    ufPtr(void) &operator=(const ufPtr(void) & smPtr);

    operator ufGenericPtr *() const;

    PTR_INT GetPointerAsInt() const;

    int operator!() const;

    // protected:
    void SetPtr(ufGenericPtr * lpNewPtr);

    static void DeleteRealPtr(ufGenericPtr * lpRealPtr);

    ufGenericPtr *fRealPtr;
    int *fRefCnt;
    int fRelationship;
};

inline ufPtr(void)::ufPtr(void)()
    : fRealPtr(0), fRefCnt(0), fRelationship(kSharingPtr) {}

inline ufPtr(void)::ufPtr(void)(const ufPtr(void) & sptr) {
    ufPtrConstruct_(this, sizeof(char), DeleteRealPtr, &sptr);
}

inline void ufPtr(void)::MakeEmpty() {
    ufPtrMakeEmpty_(this, sizeof(char), DeleteRealPtr);
}

inline void ufPtr(void)::SwapWith(ufPtr(void) & o) {
        ufPtrSwapWith_(this, &o);
}

inline ufPtr(void) & ufPtr(void)::operator=(const ufPtr(void) & sptr) {
    ufPtrAssignPtr_(this, sizeof(char), DeleteRealPtr, &sptr);
    return *this;
}

inline ufPtr(void)::operator ufGenericPtr *() const {
    ufASSERT_VALID_GENERIC_PTR() return fRealPtr;
}

inline PTR_INT ufPtr(void)::GetPointerAsInt() const {
    return (PTR_INT)fRealPtr;
}

inline int ufPtr(void)::operator!() const { return !fRealPtr; }

inline void ufPtr(void)::SetPtr(ufGenericPtr *lpNewPtr) {
    ufPtrSetPtr_(this, 1, DeleteRealPtr, lpNewPtr);
}

#ifdef USE_MFC
#define IMPLEMENT_SMART_SERIAL(class_name, base_class_name)                   \
                                                                              \
    IMPLEMENT_SERIAL(class_name, base_class_name, 1)                          \
                                                                              \
    CArchive &operator<<(CArchive &ar, ufPtr(class_name) & pOb) {             \
        ar << pOb.fRealPtr;                                                   \
        return ar;                                                            \
    }                                                                         \
                                                                              \
    CArchive &operator>>(CArchive &ar, ufPtr(class_name) & pOb) {             \
        class_name *p_ = 0;                                                   \
        pOb.MakeEmpty();                                                      \
        ar >> p_;                                                             \
        pOb.SetPtr(p_);                                                       \
        return ar;                                                            \
    }

#else

#define IMPLEMENT_SMART_SERIAL(class_name, base_class_name)

#endif

#ifdef ufPtrTEST
#define implementvoidptr()                                                    \
                                                                              \
    ufPtr(void)::~ufPtr(void)() {                                             \
        MakeEmpty();                                                          \
        ufCHECK_MEMORY                                                        \
    }                                                                         \
                                                                              \
    void ufPtr(void)::DeleteRealPtr(ufGenericPtr *d) {                        \
        delete ((void *)d);                                                   \
    }

#endif

#endif
