/*
 * See Readme, Notice and License files.
 */
#include <iostream>

#ifdef ufListTEST
#define protected public
#endif

#include "ufDef.h"
#include "ufList.h"
#include "ufPtr.h"

ufDEBUG_FILE

class ufGenericList;

class ufGenericListIter;

class ufGenericList {
public:
    ufGenericList(){};

    virtual ~ufGenericList();

    ufGenericListIter *fFirst;
    ufGenericListIter *fLast;
};

ufGenericList::~ufGenericList() {
}

class ufGenericListIter {
public:
    ufGenericListIter(){};

    virtual ~ufGenericListIter();

    ufPtr(void) fData;
    ufGenericListIter *fPrev;
    ufGenericListIter *fNext;
};

ufGenericListIter::~ufGenericListIter() {
    fPrev = fNext = 0;
}

UF_API(void)
ufListInsertFirst_(ufGenericList *lpSelf, ufPtr(void) * lpData) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpData != 0 && lpData->fRealPtr != 0 && lpData->fRefCnt != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0 || lpData == 0) {
        return;
    }

    ufGenericListIter *lpIter = NEW ufGenericListIter;

    ufASSERT(lpIter != 0)

    if (lpData->fRealPtr) {
        ufPtrAssignToEmptyPtr_(&(lpIter->fData), lpData);
    }

    if (lpSelf->fFirst == 0) {
        lpIter->fNext = 0;
        lpSelf->fFirst = lpSelf->fLast = lpIter;
    } else {
        lpSelf->fFirst->fPrev = lpIter;
        lpIter->fNext = lpSelf->fFirst;
        lpSelf->fFirst = lpIter;
    }
    lpIter->fPrev = 0;

    ufCHECK_MEMORY
}

UF_API(void)
ufListInsertLast_(ufGenericList *lpSelf, ufPtr(void) * lpData) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpData != 0 && lpData->fRealPtr != 0 && lpData->fRefCnt != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0 || lpData == 0) {
        return;
    }

    ufGenericListIter *lpIter = NEW ufGenericListIter;
    if (lpData->fRealPtr) {
        ufPtrAssignToEmptyPtr_(&(lpIter->fData), lpData);
    }

    if (lpSelf->fFirst == 0) {
        lpIter->fPrev = 0;
        lpSelf->fFirst = lpSelf->fLast = lpIter;
    } else {
        lpSelf->fLast->fNext = lpIter;
        lpIter->fPrev = lpSelf->fLast;
        lpSelf->fLast = lpIter;
    }
    lpIter->fNext = 0;

    ufCHECK_MEMORY
}

UF_API(void)
ufListInsertNth_(ufGenericList *lpSelf, ufPtr(void) * lpData, int nth) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpData != 0 && lpData->fRealPtr != 0 && lpData->fRefCnt != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0 || lpData == 0) {
        return;
    }

    if (lpSelf->fFirst == 0) {
        if (nth == 0) {
            ufListInsertFirst_(lpSelf, lpData);
            ufCHECK_MEMORY
            return;
        }
        // cerr << "0 NumElements list can not accept
        // InsertNth(" << nth << ")\n" << flush;
        ufCHECK_MEMORY
        return;
    }
    int n = nth;
    ufGenericListIter *lpIter = lpSelf->fFirst;

    while (n-- > 0 && lpIter) {
        lpIter = lpIter->fNext;
    }

    ufASSERT(n == -1 && lpIter != 0)
    if (n != -1 || lpIter == 0) {
        // cerr << "list too short for InsertNth("
        // << nth << ")\n" << flush;
        ufCHECK_MEMORY
        return;
    }

    ufGenericListIter *lpIter2 = NEW ufGenericListIter;
    if (lpData->fRealPtr) {
        ufPtrAssignToEmptyPtr_(&(lpIter2->fData), lpData);
    }

    // OLD ORDER FOR LIST: lpIter.fPrev, lpIter, lpIter.fNext
    // NEW ORDER FOR LIST: lpIter.fPrev, lpIter2, lpIter,
    // lpIter.fNext
    if (lpIter->fPrev) {
        lpIter->fPrev->fNext = lpIter2;
    }
    lpIter2->fPrev = lpIter->fPrev;
    lpIter2->fNext = lpIter;
    lpIter->fPrev = lpIter2;

    if (lpIter2->fPrev == 0) {
        lpSelf->fFirst = lpIter2;
    }
    if (lpIter2->fNext == 0) {
        lpSelf->fLast = lpIter2;
    }

    ufCHECK_MEMORY
}

UF_API(ufPtr(void) *)
ufListRemoveFirst_(ufGenericList *lpSelf, ufPtr(void) * lpDataReturn) {
    ufASSERT(lpDataReturn != 0)
    if (lpDataReturn == 0) { return 0; }

    ufASSERT(lpSelf != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0) {
        lpDataReturn->fRefCnt = 0;
        lpDataReturn->fRealPtr = 0;
        return lpDataReturn;
    }

    ufGenericListIter *lpIter = lpSelf->fFirst;

    if (lpIter) {
        if (lpIter->fNext) {
            lpIter->fNext->fPrev = lpIter->fPrev;
        }
        if (lpIter->fPrev) {
            lpIter->fPrev->fNext = lpIter->fNext;
        }
        ufPtrMoveToEmptyPtr_(lpDataReturn, &(lpIter->fData));
        ufASSERT(lpDataReturn != 0 && lpDataReturn->fRealPtr != 0 &&
                 lpDataReturn->fRefCnt != 0)
    }

    if (lpSelf->fFirst == lpIter) {
        lpSelf->fFirst = lpIter->fNext;
    }
    if (lpSelf->fLast == lpIter) {
        lpSelf->fLast = lpIter->fPrev;
    }
    if (lpSelf->fFirst == 0) {
        lpSelf->fLast = 0;
    }

    delete lpIter;

    ufCHECK_MEMORY
    return lpDataReturn;
}

UF_API(ufPtr(void) *)
ufListRemoveLast_(ufGenericList *lpSelf, ufPtr(void) * lpDataReturn) {
    ufASSERT(lpDataReturn != 0)
    if (lpDataReturn == 0) { return 0; }

    ufASSERT(lpSelf != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0) {
        ufPtrClear_(lpDataReturn);
        return lpDataReturn;
    }

    ufGenericListIter *lpIter = lpSelf->fLast;

    if (lpIter) {
        if (lpIter->fNext) {
            lpIter->fNext->fPrev = lpIter->fPrev;
        }
        if (lpIter->fPrev) {
            lpIter->fPrev->fNext = lpIter->fNext;
        }
        ufPtrMoveToEmptyPtr_(lpDataReturn, &(lpIter->fData));
        ufASSERT(lpDataReturn != 0 && lpDataReturn->fRealPtr != 0 &&
                 lpDataReturn->fRefCnt != 0)
    }

    if (lpSelf->fFirst == lpIter) {
        lpSelf->fFirst = lpIter->fNext;
    }
    if (lpSelf->fLast == lpIter) {
        lpSelf->fLast = lpIter->fPrev;
    }
    if (lpSelf->fFirst == 0) {
        lpSelf->fLast = 0;
    }

    delete lpIter;

    ufCHECK_MEMORY
    return lpDataReturn;
}

UF_API(ufPtr(void) *)
ufListRemoveNth_(ufGenericList *lpSelf, ufPtr(void) * lpDataReturn, int nth) {
    ufASSERT(lpDataReturn != 0)
    if (lpDataReturn == 0) { return 0; }

    ufASSERT(lpSelf != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0) {
        ufPtrClear_(lpDataReturn);
        return lpDataReturn;
    }

    ufGenericListIter *lpIter = lpSelf->fFirst;
    int n = nth;

    while (n-- > 0 && lpIter) {
        lpIter = lpIter->fNext;
    }

    ufASSERT(n == -1 && lpIter != 0)
    if (n != -1 || lpIter == 0) {
        // cerr << "List is too short for
        //  RemoveNth(" << nth << ")\n" << flush;
        ufCHECK_MEMORY
        return 0;
    }
    if (lpIter) {
        if (lpIter->fNext) {
            lpIter->fNext->fPrev = lpIter->fPrev;
        }
        if (lpIter->fPrev) {
            lpIter->fPrev->fNext = lpIter->fNext;
        }
        ufPtrMoveToEmptyPtr_(lpDataReturn, &(lpIter->fData));
        ufASSERT(lpDataReturn != 0 && lpDataReturn->fRealPtr != 0 &&
                 lpDataReturn->fRefCnt != 0)
    }

    if (lpSelf->fFirst == lpIter) {
        lpSelf->fFirst = lpIter->fNext;
    }
    if (lpSelf->fLast == lpIter) {
        lpSelf->fLast = lpIter->fPrev;
    }
    if (lpSelf->fFirst == 0) {
        lpSelf->fLast = 0;
    }

    delete lpIter;

    ufCHECK_MEMORY
    return lpDataReturn;
}

UF_API(ufPtr(void) *)
ufListPeekFirst_(ufGenericList *lpSelf, ufPtr(void) * lpDataReturn) {
    ufASSERT(lpDataReturn != 0)
    if (lpDataReturn == 0) { return 0; }

    ufASSERT(lpSelf != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0) {
        ufPtrClear_(lpDataReturn);
        return lpDataReturn;
    }

    ufGenericListIter *lpIter = lpSelf->fFirst;

    if (lpIter) {
        ufPtrAssignToEmptyPtr_(lpDataReturn, &(lpIter->fData));
        ufASSERT(lpDataReturn != 0 && lpDataReturn->fRealPtr != 0 &&
                 lpDataReturn->fRefCnt != 0)
    }

    ufCHECK_MEMORY
    return lpDataReturn;
}

UF_API(ufPtr(void) *)
ufListPeekLast_(ufGenericList *lpSelf, ufPtr(void) * lpDataReturn) {
    ufASSERT(lpDataReturn != 0)
    if (lpDataReturn == 0) { return 0; }

    ufASSERT(lpSelf != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0) {
        ufPtrClear_(lpDataReturn);
        return lpDataReturn;
    }

    ufGenericListIter *lpIter = lpSelf->fLast;

    if (lpIter) {
        ufPtrAssignToEmptyPtr_(lpDataReturn, &(lpIter->fData));
        ufASSERT(lpDataReturn != 0 && lpDataReturn->fRealPtr != 0 &&
                 lpDataReturn->fRefCnt != 0)
    }

    ufCHECK_MEMORY
    return lpDataReturn;
}

UF_API(ufPtr(void) *)
ufListPeekNth_(ufGenericList *lpSelf, ufPtr(void) * lpDataReturn, int nth) {
    ufASSERT(lpDataReturn != 0)
    if (lpDataReturn == 0) { return 0; }

    ufASSERT(lpSelf != 0)
    ufCHECK_MEMORY

    if (lpSelf == 0) {
        ufPtrClear_(lpDataReturn);
        return lpDataReturn;
    }

    ufGenericListIter *lpIter = lpSelf->fFirst;
    int n = nth;

    while (n-- > 0 && lpIter) {
        lpIter = lpIter->fNext;
    }
    if (n != -1 || lpIter == 0) {
        // cerr << "List is too short for RemoveNth("
        // << nth << ")\n" << flush;
        ufCHECK_MEMORY
        return 0;
    }

    if (lpIter) {
        ufPtrAssignToEmptyPtr_(lpDataReturn, &(lpIter->fData));
        ufASSERT(lpDataReturn != 0 && lpDataReturn->fRealPtr != 0 &&
                 lpDataReturn->fRefCnt != 0)
    }

    ufCHECK_MEMORY
    return lpDataReturn;
}

UF_API(int)
ufListNumElements_(ufGenericList *lpSelf) {
    ufASSERT(lpSelf != 0)
    if (lpSelf == 0) { return 0; }

    ufCHECK_MEMORY

    ufGenericListIter *lpIter = lpSelf->fFirst;
    int i = 0;

    while (lpIter) {
        lpIter = lpIter->fNext;
        i++;
        ufASSERT(lpIter == 0 ||
                 (lpIter->fData.fRealPtr != 0 && lpIter->fData.fRefCnt != 0))
    }

    ufCHECK_MEMORY
    return (i);
}

UF_API(void)
ufListMakeEmpty_(ufGenericList *lpSelf, ufListMakeEmptyFnc lpfnMakeEmpty) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpfnMakeEmpty)
    if (lpSelf == 0) { return; }

    ufCHECK_MEMORY

    ufGenericListIter *lpIter = lpSelf->fFirst;
    while (lpIter) {
        ufGenericListIter *lpNext = lpIter->fNext;
        (*lpfnMakeEmpty)(lpIter);
        delete lpIter;
        lpIter = lpNext;
    }

    lpSelf->fFirst = lpSelf->fLast = 0;

    ufCHECK_MEMORY
}

UF_API(void)
ufListIterAdd_(ufGenericListIter *lpSelf, ufGenericListIter *lpStart,
               int nCount, ufListSetPtrFnc lpfnSetPtr) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpfnSetPtr != 0)

    if (lpSelf == 0) {
        return;
    }

    ufCHECK_MEMORY

    if (!lpStart) {
        lpfnSetPtr(&(lpSelf->fData), 0);
        lpSelf->fNext = 0;
        ufCHECK_MEMORY
        return;
    }

    ufGenericListIter *lpIter2 = lpStart;
    while (nCount-- > 0 && lpIter2 != 0) {
        lpIter2 = (lpIter2 != 0 ? lpIter2->fNext : 0);
    }

    if (lpIter2) {
        (*lpfnSetPtr)(&(lpSelf->fData), &(lpIter2->fData));
        lpSelf->fNext = lpIter2->fNext;
    } else {
        lpSelf->fNext = 0;
        (*lpfnSetPtr)(&(lpSelf->fData), 0);
    }

    ufCHECK_MEMORY
}

UF_API(void)
ufListIterAssignList_(ufGenericListIter *lpSelf, const ufGenericList *lpList,
                      ufListSetPtrFnc lpfnSetPtr) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpList != 0)
    ufASSERT(lpfnSetPtr != 0)

    if (lpList == 0 || lpSelf == 0) {
        return;
    }

    if (lpList->fFirst) {
        (*lpfnSetPtr)(&(lpSelf->fData), &(lpList->fFirst->fData));
        lpSelf->fNext = lpList->fFirst->fNext;
        lpSelf->fPrev = lpList->fFirst->fPrev;
    } else {
        (*lpfnSetPtr)(&(lpSelf->fData), 0);
        lpSelf->fNext = 0;
        lpSelf->fPrev = 0;
    }
}

UF_API(void)
ufListSortIncr_(ufGenericList *lst, ufListSortFnc(void) f) {
    ufGenericListIter *i;
    ufGenericListIter *j;

    if (lst->fLast == lst->fFirst) {
        return;
    }

    for (i = lst->fLast; i != lst->fFirst; i = i->fPrev) {
        for (j = lst->fFirst->fNext; j != i->fNext; j = j->fNext) {
            if ((*f)(&(j->fPrev->fData), &(j->fData)) == GreaterThan) {
                j->fPrev->fData.SwapWith(j->fData);
            }
        }
    }
}

#ifdef ufListTEST

#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;
/*
 * this is the test program for the double linked lpSelf.
 */

class Foo;
class ufPtr(Foo);
class ufList(Foo);

class Foo {
    friend class ufPtr(Foo);

public:
    void Hello();
    void HelloAgain();
    static ufPtr(Foo) New(int xval, BOOL &isValid);

    int fX;
    char fName[60];
    BOOL &fIsValid;

private:
    Foo(int xval, BOOL &isValid);
    ~Foo();
};

declare(ufPtr, Foo);
declare(ufList, Foo);

implement(ufPtr, Foo);
implement(ufList, Foo);

Foo::Foo(int xval, BOOL &isValid) : fX(xval), fIsValid(isValid) {
    fIsValid = TRUE;
    sprintf(fName, "Foo(%d)", fX);
    cout << "Creating a foo with value " << fX << "\n" << flush;
}

Foo::~Foo() {
    cout << "Deleting a foo with value " << fX << "\n" << flush;
    fIsValid = FALSE;
}

void Foo::Hello() { cout << "Foo(" << fX << ") says hello\n" << flush; }

void Foo::HelloAgain() {
    cout << "Foo(" << fX << ") says hello again\n" << flush;
}

ufPtr(Foo) Foo::New(int xval, BOOL &isValid) {
    ufPtr(Foo) ptr(NEW Foo(xval, isValid));
    return ptr;
}

#define VERIFY_EXPR(expr1, expr2, numFailures)                                 \
    {                                                                          \
        verifyExpression((expr1) == (expr2), #expr1, #expr2, __FILE__,         \
                         __LINE__, numFailures);                               \
    }
inline void verifyExpression(BOOL expressionsAreEqual, const char *expr1,
                             const char *expr2, const char *file, int lineno,
                             int &numFailures) {
    if (!expressionsAreEqual) {
        std::cout << "TEST FAILED: " << file << ":" << lineno << ": " << expr1
                  << " != " << expr2 << "\n"
                  << std::flush;
    }
}

int main(int argc, char *argv[]) {
    int numFailures = 0;
    const static int expected[] = {1, 5, 4, 7, 3, 2, 6, 8, 9, 10};
    const int numExpected = sizeof(expected) / sizeof(expected[0]);

    ufList(Foo) mylist;
    ufPtr(Foo) myptr;

    cout << "Building a list of \n\t(";
    int e;
    for (e = 0; e < numExpected; e++) {
        cout << " " << expected[e] << ", ";
    }
    cout << ")\n";

    cout << "Num failures so far: " << numFailures << " \n" << flush;
    cout << "\n\n\n\nValidate our test class cleans up itself for the tests to "
            "come\n"
         << flush;
    BOOL item2IsValid = FALSE;
    {
        ufPtr(Foo) lptr = Foo::New(2, item2IsValid);
        VERIFY_EXPR(item2IsValid, TRUE, numFailures);
    }
    VERIFY_EXPR(item2IsValid, FALSE, numFailures);

    cout << "Num failures so far: " << numFailures << " \n" << flush;
    cout << "\n\n\n\nValidate a 1 element ufList(Foo) cleans up itself\n"
         << flush;
    {
        ufList(Foo) mylist;
        {
            ufPtr(Foo) lptr = Foo::New(2, item2IsValid);
            mylist.InsertFirst(lptr);
            VERIFY_EXPR(item2IsValid, TRUE, numFailures);
        }
        VERIFY_EXPR(item2IsValid, TRUE, numFailures);
    }
    VERIFY_EXPR(item2IsValid, FALSE, numFailures);

    cout << "Num failures so far: " << numFailures << " \n" << flush;
    cout << "\n\n\n\nUse InsertFirst to add items\n" << flush;
    BOOL itemIsValid[numExpected];
    for (e = 0; e < numExpected; e++) {
        myptr = Foo::New(expected[e], itemIsValid[e]);
        VERIFY_EXPR(itemIsValid[e], TRUE, numFailures);
        mylist.InsertFirst(myptr);
    }
    myptr.MakeEmpty();

    for (e = 0; e < numExpected; e++) {
        VERIFY_EXPR(itemIsValid[e], TRUE, numFailures);
    }

    cout << "Num failures so far: " << numFailures << " \n" << flush;
    cout << "\n\n\n\nInsertFirst a number of items\n" << flush;

    e = numExpected - 1;
    ufListIter(Foo) i;
    int numEntriesValidated = 0;
    for (i = mylist; i; i++, e--) {
        numEntriesValidated++;
        VERIFY_EXPR(expected[e], i.fData->fX, numFailures);
        ufASSERT(expected[e] = i.fData->fX)
        i.fData->Hello();
        cout << "\tdata=" << i.fData.fRealPtr
             << ", fRefCnt = " << (i.fData.fRefCnt ? *(i.fData.fRefCnt) : -1)
             << "\n"
             << flush;
    }
    VERIFY_EXPR(mylist.GetNumElements(), numEntriesValidated, numFailures);
    VERIFY_EXPR(mylist.GetNumElements(), 10, numFailures);

    if (mylist) {
        cout << "list has " << mylist.GetNumElements() << " elements\n"
             << flush;
    } else {
        cout << "list is empty\n" << flush;
    }

    myptr = mylist.PeekFirst();
    cout << "First item on the list is a " << myptr->fName << "\n" << flush;
    myptr = mylist.PeekLast();
    cout << "Last item on the list is a " << myptr->fName << "\n" << flush;
    myptr = mylist.PeekNth(5);
    cout << "The 6th item on the list is a " << myptr->fName << "\n" << flush;

    myptr = mylist.RemoveFirst();
    cout << "Removed the first item " << myptr->fName << "\n" << flush;
    myptr = mylist.RemoveLast();
    cout << "Removed the last item " << myptr->fName << "\n" << flush;
    myptr = mylist.RemoveNth(4);
    cout << "Removed the 5th item " << myptr->fName << "\n" << flush;

    cout << "Explicitedly unreferenceing the 5th item \n" << flush;
    myptr.MakeEmpty();

    for (i = mylist; i; ++i) {
        i.fData->HelloAgain();
    }

    if (mylist) {
        cout << "list has " << mylist.GetNumElements() << " elements\n"
             << flush;
    } else {
        cout << "list is empty\n" << flush;
    }

    cout << "Num failures so far: " << numFailures << " \n" << flush;
    cout << "\n\n\n\nRemove all remaining elements in the list\n" << flush;
    mylist.MakeEmpty();
    VERIFY_EXPR(mylist.GetNumElements(), 0, numFailures);
    for (e = 0; e < numExpected; e++) {
        VERIFY_EXPR(itemIsValid[e], FALSE, numFailures);
    }

    cout << "TEST DONE. Total Failures: " << numFailures << "\n";

    return (numFailures);
}

#endif
