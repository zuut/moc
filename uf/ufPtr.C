/*
 * See Readme, Notice and License files.
 */
#include "ufDef.h"
#include "ufPtr.h"

ufDEBUG_FILE

#define ufCHECK_REF_CNT(x)                                                     \
    ufASSERT(x == 0 || (x->fRefCnt == 0 && x->fRealPtr == 0) ||                \
             (*x->fRefCnt != 0 && x->fRealPtr != 0))

#ifdef _DEBUG
#define ufASSERT_VALID_VOID_PTR ufPtrAssertValidPtr_(lpSelf, uSz, lpfnDel);
#else
#define ufASSERT_VALID_VOID_PTR
#endif

    inline void
    ufAssignRelationship(ufPtr(void) * dest, const ufPtr(void) * src) {
    // when copying a self referential pointer, set the relationship to shared.
    dest->fRelationship =
        (src->fRelationship != kSelfPtrShared ? src->fRelationship
                                              : kSharingPtr);
}

UF_API(void)
ufPtrConstruct_(ufPtr(void) * lpSelf, unsigned uSz, ufPtrDeleteFnc lpfnDel,
                const ufPtr(void) * lpOther) {
    ufASSERT(lpSelf) ufCHECK_REF_CNT(lpOther)

        if (lpSelf == 0) {
        return;
    }

    if (lpOther != 0) {
        lpSelf->fRealPtr = lpOther->fRealPtr;
        lpSelf->fRefCnt = lpOther->fRefCnt;
        ufAssignRelationship(lpSelf, lpOther);
        if (lpSelf->fRefCnt) {
            (*lpSelf->fRefCnt)++;
        }
        ufASSERT_VALID_VOID_PTR
    } else {
        lpSelf->fRealPtr = 0;
        lpSelf->fRefCnt = 0;
        lpSelf->fRelationship = kSharingPtr;
    }
    ufCHECK_MEMORY ufCHECK_REF_CNT(lpSelf)
}

UF_API(void)
ufPtrMakeEmpty_(ufPtr(void) * lpSelf, unsigned uSz, ufPtrDeleteFnc lpfnDel) {
    ufASSERT(lpSelf != 0) ufCHECK_REF_CNT(lpSelf) if (!lpSelf) { return; }

    int tmpRelationship = lpSelf->fRelationship;
    int *lpTmpRefCnt = lpSelf->fRefCnt;
    ufGenericPtr *lpTmpRealPtr = lpSelf->fRealPtr;
    lpSelf->fRealPtr = 0;
    lpSelf->fRefCnt = 0;
    lpSelf->fRelationship = kSharingPtr;

    // self pointers don't participate in the ref counting.
    if (tmpRelationship != kSelfPtrShared) {
        if (lpTmpRefCnt && --(*lpTmpRefCnt) <= 0) {
            ufDeleteInt_(lpTmpRefCnt);
            (*lpfnDel)(lpTmpRealPtr);
        }
    }
    ufCHECK_MEMORY

        (void) uSz;
}

UF_API(void)
ufPtrAssertValidPtr_(const ufPtr(void) * lpSelf, unsigned uSz,
                     ufPtrDeleteFnc lpfnDel) {
    ufASSERT(lpSelf) ufCHECK_REF_CNT(lpSelf) if (!lpSelf) { return; }

    ufASSERT(lpSelf->fRealPtr != 0) ufASSERT(lpSelf->fRefCnt != 0)
        ufCHECK_MEMORY
#ifdef _DEBUG
            ufValidateAddress((const void FAR *)lpSelf->fRealPtr, uSz);
#endif
    (void)lpfnDel;
}

UF_API(void)
ufPtrSetPtr_(ufPtr(void) * lpSelf, unsigned uSz, ufPtrDeleteFnc lpfnDel,
             ufGenericPtr *lpNewPtr, BOOL bDeleteOldPtr) {
    ufASSERT(lpSelf) ufCHECK_REF_CNT(lpSelf) if (!lpSelf) { return; }

    if (bDeleteOldPtr) {
        ufPtrMakeEmpty_(lpSelf, uSz, lpfnDel);
    }

    if (lpNewPtr && (lpSelf->fRefCnt = ufNewInt_()) != 0) {
        *(lpSelf->fRefCnt) = 1;
        lpSelf->fRealPtr = lpNewPtr;
        lpSelf->fRelationship = kSharingPtr;
        ufASSERT_VALID_VOID_PTR
    } else {
        lpSelf->fRefCnt = 0;
        lpSelf->fRealPtr = 0;
        lpSelf->fRelationship = kSharingPtr;
    }
    ufCHECK_REF_CNT(lpSelf)
}

UF_API(void)
ufPtrAssignInt_(ufPtr(void) * lpSelf, unsigned uSz, ufPtrDeleteFnc lpfnDel,
                int nZero) {
    ufASSERT(lpSelf) ufCHECK_REF_CNT(lpSelf) if (!lpSelf) { return; }

    ufASSERT(nZero == 0)(void) nZero;
    ufPtrMakeEmpty_(lpSelf, uSz, lpfnDel);
    ufCHECK_REF_CNT(lpSelf)
}

UF_API(void)
ufPtrSwapWith_(ufPtr(void) * lpSelf, ufPtr(void) * lpOther) {
    ufASSERT(lpSelf) ufASSERT(lpOther) ufCHECK_REF_CNT(lpSelf)
        ufCHECK_REF_CNT(lpOther)

            if (!lpSelf || !lpOther) {
        return;
    }

    if (lpOther == lpSelf) {
        return;
    }

    ufGenericPtr *tmpRealPtr = lpSelf->fRealPtr;
    int *tmpRefCnt = lpSelf->fRefCnt;
    int tmpRel = lpSelf->fRelationship;

    lpSelf->fRealPtr = lpOther->fRealPtr;
    lpSelf->fRefCnt = lpOther->fRefCnt;
    lpSelf->fRelationship = lpOther->fRelationship;

    lpOther->fRealPtr = tmpRealPtr;
    lpOther->fRefCnt = tmpRefCnt;
    lpOther->fRelationship = tmpRel;
}

UF_API(void)
ufPtrAssignPtr_(ufPtr(void) * lpSelf, unsigned uSz, ufPtrDeleteFnc lpfnDel,
                const ufPtr(void) * lpOther) {
    ufASSERT(lpSelf) ufASSERT(lpOther) ufCHECK_REF_CNT(lpSelf)
        ufCHECK_REF_CNT(lpOther) if (!lpSelf || !lpOther) {
        return;
    }

    if (lpOther == lpSelf || lpOther->fRealPtr == lpSelf->fRealPtr) {
        return;
    }

    ufPtrMakeEmpty_(lpSelf, uSz, lpfnDel);

    lpSelf->fRealPtr = lpOther->fRealPtr;
    lpSelf->fRefCnt = lpOther->fRefCnt;
    ufAssignRelationship(lpSelf, lpOther);

    if (lpSelf->fRefCnt) {
        (*(lpSelf->fRefCnt))++;
    }
    ufCHECK_REF_CNT(lpSelf)
}

UF_API(void)
ufPtrAssignToEmptyPtr_(ufPtr(void) * lpSelf, const ufPtr(void) * lpOther) {
    ufASSERT(lpSelf) ufASSERT(lpOther) ufCHECK_REF_CNT(lpSelf)
        ufCHECK_REF_CNT(lpOther) if (!lpSelf || !lpOther) {
        return;
    }

    if (lpOther == lpSelf || lpOther->fRealPtr == lpSelf->fRealPtr) {
        return;
    }

    lpSelf->fRealPtr = lpOther->fRealPtr;
    lpSelf->fRefCnt = lpOther->fRefCnt;
    ufAssignRelationship(lpSelf, lpOther);

    if (lpSelf->fRefCnt) {
        (*(lpSelf->fRefCnt))++;
    }
    ufCHECK_REF_CNT(lpSelf)
}

UF_API(void)
ufPtrMoveToEmptyPtr_(ufPtr(void) * lpSelf, ufPtr(void) * lpOther) {
    ufASSERT(lpSelf) ufASSERT(lpOther) ufCHECK_REF_CNT(lpSelf)
        ufCHECK_REF_CNT(lpOther) if (!lpSelf || !lpOther) {
        return;
    }

    if (lpOther == lpSelf || lpOther->fRealPtr == lpSelf->fRealPtr) {
        return;
    }

    lpSelf->fRealPtr = lpOther->fRealPtr;
    lpSelf->fRefCnt = lpOther->fRefCnt;
    ufAssignRelationship(lpSelf, lpOther);

    lpOther->fRealPtr = 0;
    lpOther->fRefCnt = 0;
    lpOther->fRelationship = kSharingPtr;
    ufCHECK_REF_CNT(lpSelf) ufCHECK_REF_CNT(lpOther)
}

UF_API(void)
ufPtrClear_(ufPtr(void) * lpSelf) {
    ufASSERT(lpSelf) ufCHECK_REF_CNT(lpSelf) if (!lpSelf) { return; }

    lpSelf->fRealPtr = 0;
    lpSelf->fRefCnt = 0;
    lpSelf->fRelationship = kSharingPtr;
    ufCHECK_REF_CNT(lpSelf)
}

ufPtr(void)::~ufPtr(void)() {
    MakeEmpty();
    ufCHECK_MEMORY
}

void ufPtr(void)::DeleteRealPtr(ufGenericPtr *) {
#ifdef _DEBUG
    int AttemptToDelete_ufPtr_void_ = 0;
    ufASSERT(AttemptToDelete_ufPtr_void_)
#endif
}

// TBD: add memory pool of ints
UF_API(int *)
ufNewInt_() { return NEW int; }

UF_API(void)
ufDeleteInt_(int *val) { delete val; }

#ifdef ufPtrTEST

#include <iostream>
#include <string>

#define ufVERIFY_REF_CNT(sp, rc, errCnt)                                       \
    {                                                                          \
        bool myassertres =                                                     \
            ((sp == 0 && rc == 0) || ((*sp).matchingRefCount(rc)));            \
        if (!myassertres) {                                                    \
            errCnt++;                                                          \
            std::cerr << "TEST FAILED: " << __FILE__ << ":" << __LINE__        \
                      << ": " << #sp << " != " << #rc << "\n"                  \
                      << std::flush;                                           \
        }                                                                      \
    }
#define ufVERIFY_RESULT(isVal, expected, numFailures)                          \
    if (isVal != expected) {                                                   \
        std::cout << " TEST FAILED: " << __FILE__ << ":" << __LINE__ << ": "   \
                  << #isVal << " != " << std::boolalpha << expected << "\n"    \
                  << std::flush;                                               \
        numFailures++;                                                         \
    }

/*
 * this is the test program for the smart pointer
 */

class Foo;
class ufPtr(Foo);

class Foo {
    friend class ufPtr(Foo);

public:
    void Hello();
    static ufPtr(Foo) New(int xval, bool &isValid);

    int fX;
    bool &fIsValid;
    // private: //normaly private but make it public for testing
    Foo(int xval, bool &isValid);
    ~Foo();
};

declare(ufPtr, Foo);
implement(ufPtr, Foo);

Foo::Foo(int xval, bool &isValid) : fX(xval), fIsValid(isValid) {
    fIsValid = true;
    std::cout << "Creating a Foo(" << ((void *)this) << ") with value " << fX
              << "\n"
              << std::flush;
}

Foo::~Foo() {
    std::cout << "Deleting a Foo(" << ((void *)this) << ") with value " << fX
              << "\n"
              << std::flush;
    fIsValid = false;
}

void Foo::Hello() {
    std::cout << "Foo(" << ((void *)this) << "," << fX << ") says hello\n"
              << std::flush;
}

ufPtr(Foo) Foo::New(int xval, bool &isValid) {
    ufPtr(Foo) ptr(NEW Foo(xval, isValid));
    return ptr;
}

class FooBar;
declareclass(ufPtr, FooBar);

class FooBar {
    friend class ufPtr(FooBar);

public:
    void Hello();
    static ufPtr(FooBar) New(int xval, bool &isValid);

    int fX;
    bool &fIsValid;
    ufPtr(FooBar) & GetSelfReferentialPointer() { return fSelf; }

    // private: //normaly private but make it public for testing
    FooBar(int xval, bool &isValid);
    ~FooBar();
    ufPtr(FooBar) fSelf;
};

declareinlines(ufPtr, FooBar);
implement(ufPtr, FooBar);

FooBar::FooBar(int xval, bool &isValid)
    : fX(xval), fIsValid(isValid),
      // WARNING: Self Referentials will keep an instance from being deleted
      // unless its allocated on the stack as there's no way to drop the
      // final reference!
      fSelf(this, kSelfPtrShared) {
    fIsValid = true;
    std::cout << "Creating a FooBar(" << ((void *)this) << ")  with value "
              << fX << "\n"
              << std::flush;
}

FooBar::~FooBar() {
    std::cout << "Deleting a FooBar(" << ((void *)this) << ")  with value "
              << fX << "\n"
              << std::flush;
    fIsValid = false;
}

void FooBar::Hello() {
    std::cout << "FooBar(" << ((void *)this) << ", " << fX << ") says hello\n"
              << std::flush;
}

ufPtr(FooBar) FooBar::New(int xval, bool &isValid) {
    return (NEW FooBar(xval, isValid))->fSelf;
}

int main(int argc, char *argv[]) {
    int numFailures = 0;

    bool stackPtrIsValid = false;
    {
        std::cout << "Testing Foo on the stack \n" << std::flush;
        Foo foo(1, stackPtrIsValid);
        ufVERIFY_RESULT(stackPtrIsValid, true, numFailures);
    }
    ufVERIFY_RESULT(stackPtrIsValid, false, numFailures);

    std::cout << "Testing Single Pointer \n" << std::flush;
    bool aptrIsValid = false;
    ufVERIFY_RESULT(aptrIsValid, false, numFailures);
    ufPtr(Foo) aptr = Foo::New(1, aptrIsValid);
    ufVERIFY_RESULT(aptrIsValid, true, numFailures);
    ufVERIFY_REF_CNT(&aptr, 1, numFailures);
    aptr.MakeEmpty();
    ufVERIFY_REF_CNT(&aptr, 0, numFailures);
    ufVERIFY_RESULT(aptrIsValid, false, numFailures);

    std::cout << "Num failures so far: " << numFailures << " \n" << std::flush;
    std::cout << "\n\n\nTesting Two Pointers to Simple Structs \n"
              << std::flush;
    bool myptr1IsValid = false;
    bool myptr2IsValid = false;
    ufPtr(Foo) myptr1 = Foo::New(11, myptr1IsValid);
    ufPtr(Foo) myptr2;

    ufVERIFY_RESULT(myptr1IsValid, true, numFailures);
    ufVERIFY_RESULT(myptr2IsValid, false, numFailures);
    std::cout << "myptr1IsValid inited\n" << std::flush;

    ufVERIFY_REF_CNT(&myptr1, 1, numFailures);
    ufVERIFY_REF_CNT(&myptr2, 0, numFailures);

    myptr2 = Foo::New(12, myptr2IsValid);
    std::cout << "myptr2IsValid inited\n" << std::flush;
    ufVERIFY_RESULT(myptr1IsValid, true, numFailures);
    ufVERIFY_RESULT(myptr2IsValid, true, numFailures);
    ufVERIFY_REF_CNT(&myptr2, 1, numFailures);

    myptr1->Hello();
    ufVERIFY_REF_CNT(&myptr1, 1, numFailures);

    myptr1 = myptr2;
    std::cout << "myptr1 = myptr2\n" << std::flush;
    ufVERIFY_REF_CNT(&myptr1, 2, numFailures);
    ufVERIFY_REF_CNT(&myptr2, 2, numFailures);
    ufVERIFY_RESULT(myptr1IsValid, false, numFailures);
    ufVERIFY_RESULT(myptr2IsValid, true, numFailures);

    myptr1->Hello();
    ufVERIFY_REF_CNT(&myptr1, 2, numFailures);
    ufVERIFY_REF_CNT(&myptr2, 2, numFailures);
    ufVERIFY_RESULT(myptr1IsValid, false, numFailures);
    ufVERIFY_RESULT(myptr2IsValid, true, numFailures);

    myptr2.MakeEmpty();
    std::cout << "myptr2.MakeEmpty()\n" << std::flush;
    ufVERIFY_RESULT(myptr1IsValid, false, numFailures);
    ufVERIFY_RESULT(myptr2IsValid, true, numFailures);

    ufVERIFY_REF_CNT(&myptr1, 1, numFailures);
    ufVERIFY_REF_CNT(&myptr2, 0, numFailures);

    myptr1->Hello();
    ufVERIFY_REF_CNT(&myptr1, 1, numFailures);
    ufVERIFY_REF_CNT(&myptr2, 0, numFailures);

    ufVERIFY_RESULT(myptr1.isValid(), true, numFailures);
    ufVERIFY_RESULT(myptr2.isValid(), false, numFailures);

    bool stackSelfPtrIsValid = false;
    {
        std::cout << "Num failures so far: " << numFailures << " \n"
                  << std::flush;
        std::cout << "Testing FooBar on the stack \n" << std::flush;
        FooBar fooBar(1, stackSelfPtrIsValid);
        ufVERIFY_RESULT(stackSelfPtrIsValid, true, numFailures);
    }
    ufVERIFY_RESULT(stackSelfPtrIsValid, false, numFailures);

    std::cout << "Num failures so far: " << numFailures << " \n" << std::flush;
    std::cout << "\n\n\n\nFooBar w/ self referential pointer test\n"
              << std::flush;
    bool selfPtr1IsValid = false;
    bool selfPtr2IsValid = false;
    // FooBar has an fSelf referencing ptr
    ufPtr(FooBar) selfPtr1 = FooBar::New(101, selfPtr1IsValid);
    ufPtr(FooBar) &selfPtr1Ref = selfPtr1->GetSelfReferentialPointer();
    ufPtr(FooBar) selfPtr2 = FooBar::New(102, selfPtr2IsValid);
    ufPtr(FooBar) &selfPtr2Ref = selfPtr2->GetSelfReferentialPointer();

    ufVERIFY_RESULT(selfPtr1IsValid, true, numFailures);
    ufVERIFY_RESULT(selfPtr2IsValid, true, numFailures);
    ufVERIFY_REF_CNT(&selfPtr1, 1, numFailures);
    ufVERIFY_REF_CNT(&selfPtr1Ref, 1, numFailures);
    ufVERIFY_REF_CNT(&selfPtr2, 1, numFailures);
    ufVERIFY_REF_CNT(&selfPtr2Ref, 1, numFailures);

    selfPtr1 = selfPtr2;
    ufVERIFY_REF_CNT(&selfPtr1, 2, numFailures);
    // WARNING: selfPtr1Ref is a BAD REF
    ufVERIFY_RESULT(selfPtr1IsValid, false, numFailures);
    ufVERIFY_REF_CNT(&selfPtr2, 2, numFailures);
    ufVERIFY_REF_CNT(&selfPtr2Ref, 2, numFailures);
    ufVERIFY_RESULT(selfPtr2IsValid, true, numFailures);

    std::cout << "Num failures so far: " << numFailures << " \n" << std::flush;
    std::cout << "\n\n\n\nFooBar w/ self referential pointer test\n"
              << std::flush;
    bool dblPtr1IsValid = false;
    bool dblPtr2IsValid = false;
    // FooBar has an fSelf referential pointer
    ufPtr(FooBar) dblPtr1 = FooBar::New(1001, dblPtr1IsValid);
    ufPtr(FooBar) dblPtr2;
    ufVERIFY_RESULT(dblPtr1IsValid, true, numFailures);
    ufVERIFY_RESULT(dblPtr2IsValid, false, numFailures);
    ufVERIFY_REF_CNT(&dblPtr1, 1, numFailures);
    ufVERIFY_REF_CNT(&dblPtr2, 0, numFailures);

    dblPtr2 = FooBar::New(1002, dblPtr2IsValid);
    ufVERIFY_RESULT(dblPtr1IsValid, true, numFailures);
    ufVERIFY_RESULT(dblPtr2IsValid, true, numFailures);
    ufVERIFY_REF_CNT(&dblPtr1, 1, numFailures);
    ufVERIFY_REF_CNT(&dblPtr2, 1, numFailures);

    dblPtr1->Hello();
    ufVERIFY_REF_CNT(&dblPtr1, 1, numFailures);
    ufVERIFY_REF_CNT(&dblPtr2, 1, numFailures);

    dblPtr1 = dblPtr2;
    std::cout << "dblPtr1 = dblPtr2\n" << std::flush;
    ufVERIFY_RESULT(dblPtr1IsValid, false, numFailures);
    ufVERIFY_RESULT(dblPtr2IsValid, true, numFailures);
    ufVERIFY_REF_CNT(&dblPtr1, 2, numFailures);
    ufVERIFY_REF_CNT(&dblPtr2, 2, numFailures);

    dblPtr1->Hello();
    dblPtr2.MakeEmpty();
    ufVERIFY_RESULT(dblPtr2IsValid, true, numFailures);
    ufVERIFY_REF_CNT(&dblPtr1, 1, numFailures);
    ufVERIFY_REF_CNT(&dblPtr2, 0, numFailures);

    dblPtr1->Hello();

    ufVERIFY_RESULT(dblPtr2.isValid(), false, numFailures);

    std::cout << "TEST DONE. Total Failures: " << numFailures << "\n";
    return (numFailures);
}

#endif
