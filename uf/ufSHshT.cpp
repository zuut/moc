/*
 * See Readme, Notice and License files.
 */
#include <string.h>

#ifdef ufSHshTTEST
#define protected public
#endif

#include "ufDef.h"
#include "ufPtr.h"
#include "ufSHshT.h"

ufDEBUG_FILE

    class ufGenericStrHashIter;

class ufGenericStrHash {
public:
    ufGenericStrHash(){};

    virtual ~ufGenericStrHash();

    int fNumElements;
    int fNumBuckets;
    ufGenericStrHashIter **fBuckets;
    ufGenericStrHashIter **fLastBucket;
};

ufGenericStrHash::~ufGenericStrHash() {}

class ufGenericStrHashIter {
public:
    ufGenericStrHashIter(){};

    virtual ~ufGenericStrHashIter();

    ufString fKey;
    ufPtr(void) fData;

    ufGenericStrHashIter **fOwnerBucket;
    ufGenericStrHashIter **fLastBucket;
    ufGenericStrHashIter *fNext;
};

ufGenericStrHashIter::~ufGenericStrHashIter() {}

#define HASH(tbl, key, h, len)                                                \
    {                                                                         \
        const char *c;                                                        \
        h = 0;                                                                \
        for (c = key; *c != '\0'; c++) {                                      \
            h = (h << 1) ^ ((int)*c);                                         \
        }                                                                     \
        len = c - (const char *)key;                                          \
        h &= tbl->fNumBuckets;                                                \
    }

UF_API(void)
ufSHConstruct(ufGenericStrHash *lpSelf, int nSize) {
    ufASSERT(lpSelf != 0)
    if (lpSelf == 0) { return; }

    ufCHECK_MEMORY

    lpSelf->fNumBuckets = 16;
    while (lpSelf->fNumBuckets < nSize) {
        lpSelf->fNumBuckets <<= 1;
    }
    lpSelf->fBuckets = NEW ufGenericStrHashIter * [lpSelf->fNumBuckets];
    memset(lpSelf->fBuckets, 0,
           lpSelf->fNumBuckets * sizeof(ufGenericStrHash *));
    lpSelf->fLastBucket = lpSelf->fBuckets + (--(lpSelf->fNumBuckets));
    lpSelf->fNumElements = 0;

    ufCHECK_MEMORY
}

UF_API(void)
ufSHDeconstruct(ufGenericStrHash *lpSelf) {
    ufCHECK_MEMORY

    delete[] lpSelf->fBuckets;
    lpSelf->fBuckets = 0;
    lpSelf->fNumElements = 0;

    ufCHECK_MEMORY
}

UF_API(void)
ufSHMakeEmpty_(ufGenericStrHash *lpSelf, ufSHMakeEmptyFnc makeEmpty) {
    ufGenericStrHashIter * *b;
    ufGenericStrHashIter * i;
    ufGenericStrHashIter * next;

    for (b = lpSelf->fBuckets; b != lpSelf->fLastBucket; b++) {
        for (i = *b; i; i = next) {
            next = i->fNext;
            makeEmpty(i);
            delete i;
        }
        *b = 0;
    }
}

UF_API(int)
ufSHInsert_(ufGenericStrHash *lpSelf, const char *lpszKey,
            ufPtr(void) * lpData) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpszKey != 0)
    ufASSERT(lpData != 0)
    if (lpSelf == 0 || lpszKey == 0 || lpData == 0) {
        return 0;
    }

    ufCHECK_MEMORY

    unsigned int h;
    int nLen;
    ufGenericStrHashIter *i;
    HASH(lpSelf, lpszKey, h, nLen);

    for (i = lpSelf->fBuckets[h]; i; i = i->fNext) {
        if (nLen == i->fKey.GetLength() && lpszKey == i->fKey) //== lpszKey)
        {
            ufCHECK_MEMORY
            return 0;
        }
    }

    if ((i = NEW ufGenericStrHashIter) == 0 || (i->fKey = lpszKey) == 0) {
        ufCHECK_MEMORY
        return 0;
    }

    ufPtrAssignToEmptyPtr_(&(i->fData), lpData);
    i->fOwnerBucket = lpSelf->fBuckets + h;
    i->fNext = lpSelf->fBuckets[h];
    lpSelf->fBuckets[h] = i;
    i->fLastBucket = lpSelf->fLastBucket;
    lpSelf->fNumElements++;

    ufCHECK_MEMORY
    return 1;
}

UF_API(int)
ufSHRemove_(ufGenericStrHash *lpSelf, const char *lpszKey,
            ufPtr(void) * lpData) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpszKey != 0)
    ufASSERT(lpData != 0)

    if (lpSelf == 0 || lpszKey == 0 || lpData == 0) {
        return 0;
    }

    ufCHECK_MEMORY

    unsigned int h;
    int nLen;
    ufGenericStrHashIter *i;
    ufGenericStrHashIter *p = 0;
    HASH(lpSelf, lpszKey, h, nLen);

    for (i = lpSelf->fBuckets[h];
         i != 0 && (i->fKey.GetLength() != nLen ||
                    lpszKey != i->fKey); // i->fKey != lpszKey);
         p = i, i = i->fNext)
        ;

    if (i) {
        if (p) {
            p->fNext = i->fNext;
        } else {
            lpSelf->fBuckets[h] = i->fNext;
        }
        ufPtrMoveToEmptyPtr_(lpData, &(i->fData));
        delete i;
        lpSelf->fNumElements--;
        ufCHECK_MEMORY
        return 1;
    }

    ufCHECK_MEMORY
    return 0;
}

UF_API(int)
ufSHFind_(ufGenericStrHash *lpSelf, const char *lpszKey, ufPtr(void) * lpData) {
    ufASSERT(lpSelf != 0)
    ufASSERT(lpszKey != 0)
    ufASSERT(lpData != 0)

    if (lpSelf == 0 || lpszKey == 0 || lpData == 0) {
        return 0;
    }

    ufCHECK_MEMORY

    unsigned int h;
    int nLen;
    ufGenericStrHashIter *i;
    ufGenericStrHashIter *p = 0;
    HASH(lpSelf, lpszKey, h, nLen);

    for (i = lpSelf->fBuckets[h];
         i != 0 && (i->fKey.GetLength() != nLen || i->fKey != lpszKey);
         p = i, i = i->fNext)
        ;

    if (i) {
        if (p) {
            // Move to head of bucket list.
            p->fNext = i->fNext;
            ufGenericStrHashIter *first = lpSelf->fBuckets[h];
            lpSelf->fBuckets[h] = i;
            i->fNext = first;
        }
        ufPtrAssignToEmptyPtr_(lpData, &(i->fData));
        ufCHECK_MEMORY
        return 1;
    }

    ufCHECK_MEMORY
    return 0;
}

UF_API(void)
ufSHAdd_(ufGenericStrHashIter *i1, ufGenericStrHashIter *i2, int incr) {
    ufASSERT(i1 != 0)
    ufASSERT(i2 != 0)
    if (i1 == 0 || i2 == 0) { return; }

    ufCHECK_MEMORY

    ufGenericStrHashIter **last = i2->fLastBucket;

    while (incr-- > 0) {
        if (i2->fNext) {
            i2 = i2->fNext;
        } else {
            ufGenericStrHashIter **b = i2->fOwnerBucket;
            if (b == last) {
                ufCHECK_MEMORY
                return;
            }
            while (++b != last && *b == 0)
                ;
            if (*b) {
                i2 = *b;
            } else {
                ufCHECK_MEMORY
                return;
            }
        }
    }
    ufPtrAssignToEmptyPtr_(&(i1->fData), &(i2->fData));

    i1->fKey = i2->fKey;
    i1->fOwnerBucket = i2->fOwnerBucket;
    i1->fLastBucket = i2->fLastBucket;
    i1->fNext = i2->fNext;

    ufCHECK_MEMORY
}

#ifdef ufSHshTTEST

#include <iostream>
#include <stdio.h>

using namespace std;
/*
 * this is the test program for the hash tables.
 */

class Foo;
class ufPtr(Foo);
class ufStrHash(Foo);

class Foo {
    friend class ufPtr(Foo);

public:
    void Hello();
    void HelloAgain();
    static ufPtr(Foo) New(int xval);

    int fX;
    char fName[60];

private:
    Foo(int xval);
    ~Foo();
};

declare(ufPtr, Foo);
declare(ufStrHash, Foo);

implement(ufPtr, Foo);
implement(ufStrHash, Foo);

Foo::Foo(int xval) : fX(xval) {
    sprintf(fName, "Foo(%d)", fX);
    cout << "Creating a foo with value " << fX << "\n" << flush;
}

Foo::~Foo() { cout << "Deleting a foo with value " << fX << "\n" << flush; }

void Foo::Hello() { cout << "Foo(" << fX << ") says hello\n" << flush; }

void Foo::HelloAgain() {
    cout << "Foo(" << fX << ") says hello again\n" << flush;
}

ufPtr(Foo) Foo::New(int xval) {
    ufPtr(Foo) ptr(NEW Foo(xval));
    return ptr;
}

#define KEY( num ) "Foo(" #num ")"
#define KEY_FIELD fName
typedef ufStrHash(Foo) TestHashTable;
typedef ufStrHashIter(Foo) TestHashTableIterator;

int main(int argc, char *argv[]) {
    TestHashTable myhashTable;
    for (int i = 0; i < 10; i++) {
        ufPtr(Foo) myptr = Foo::New(i);
        myhashTable.Insert(myptr->KEY_FIELD, myptr);
        myhashTable.Remove(myptr->KEY_FIELD);
    }
}

int main1(int argc, char *argv[]) {

    std:: cout << "TESTING ufStrHash " << std::endl;
    int numFailures = 0;

    ufTmpBufInit();

    TestHashTable myhashTable;
    ufPtr(Foo) myptr;

    myptr = Foo::New(1);
    ufTEST_STMT( myhashTable.Insert(myptr->KEY_FIELD, myptr) );
    ufVERIFY_RESULT(myhashTable.GetNumElements() == 1, TRUE, numFailures);
    myptr.MakeEmpty();

    myptr = Foo::New(2);
    if (myptr.isValid()) {
        myhashTable.Insert(myptr->KEY_FIELD, myptr);
    }

    myptr = Foo::New(3);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);
    myptr = Foo::New(4);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);
    myptr = Foo::New(5);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);

    myptr = Foo::New(10);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);
    myptr = Foo::New(9);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);
    myptr = Foo::New(8);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);
    myptr = Foo::New(7);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);
    myptr = Foo::New(6);
    myhashTable.Insert(myptr->KEY_FIELD, myptr);

    ufVERIFY_RESULT(myhashTable.GetNumElements() == 10, TRUE, numFailures);

    TestHashTableIterator i;
    {
        int j = 0;
        for (i = myhashTable; i; i++, j++) {
            i.fData->Hello();
            cout << "\trealptr=" << i.fData.fRealPtr
                 << ", refcnt = " << (i.fData.fRefCnt ? *(i.fData.fRefCnt) : -1)
                 << "\n"
                 << flush;
        }
        ufVERIFY_RESULT(j == 10, TRUE, numFailures);
    }
    if (myhashTable) {
        cout << "table has " << myhashTable.GetNumElements() << " elements\n"
             << flush;
    } else {
        cout << "table is empty\n" << flush;
    }

    ufTEST_STMT(BOOL wasFound = myhashTable.Find(KEY(3), myptr));
    ufVERIFY_RESULT(wasFound, TRUE, numFailures);

    if (wasFound) {
        cout << "OK: myhashTable.Find(" << KEY(3) <<") found "
             << myptr->fName << "\n"
             << flush;
    } else {
        cout << "ERROR: myhashTable did not find "
             << KEY(3) << "\n" << flush;
    }

    ufTEST_STMT(myhashTable.Remove(KEY(3)));
    ufTEST_STMT((wasFound = myhashTable.Find(KEY(3), myptr)));
    ufVERIFY_RESULT(wasFound, FALSE, numFailures);
    if (wasFound) {
        cout << "ERROR: myhashTable.Find("
             << KEY(3) << ") found " << myptr->fName
             << " after it was removed\n"
             << flush;
        myptr.MakeEmpty();
    } else {
        cout << "OK: myhashTable did not find 3 after "
                "it was removed\n"
             << flush;
    }
    myptr = Foo::New(-4);
    // using KEY(4) as it exists already in hashtable
    ufTEST_STMT( BOOL wasInserted = myhashTable.Insert(KEY(4), myptr) );
    ufVERIFY_RESULT( wasInserted, FALSE, numFailures);

    if (wasInserted) {
        cout << "ERROR: myhashTable excepted a "
                "duplicate entry for the "
                "same key=4\n"
             << flush;
    } else {
        cout << "OK: myhashTable did not allow a "
                "duplicate entry for the "
                "same key=4\n"
             << flush;
    }

    {
        int j = 0;
        for (i = myhashTable; i; i++, j++) {
            i.fData->HelloAgain();
            cout << "\tbucket[" << i.fOwnerBucket - myhashTable.fBuckets
                 << "]=" << i.fOwnerBucket
                 << ", refcnt = " << (i.fData.fRefCnt ? *(i.fData.fRefCnt) : -1)
                 << "\n"
                 << flush;
        }
        ufVERIFY_RESULT(j == 9, TRUE, numFailures);
    }

    myptr = Foo::New(-4);
    ufTEST_STMT( wasInserted = myhashTable.Insert(myptr->KEY_FIELD, myptr) );
    ufVERIFY_RESULT( wasInserted, TRUE, numFailures);
    {
        int j = 0;
        for (i = myhashTable; i; i++, j++) {
            i.fData->HelloAgain();
            cout << "\tbucket[" << i.fOwnerBucket - myhashTable.fBuckets
                 << "]=" << i.fOwnerBucket
                 << ", refcnt = " << (i.fData.fRefCnt ? *(i.fData.fRefCnt) : -1)
                 << "\n"
                 << flush;
        }
        ufVERIFY_RESULT(j == 10, TRUE, numFailures);
    }

    cout << "TEST DONE\n";

    ufTmpBufDestroy();

    std::cout << "TEST DONE. Total Failures: " << numFailures << "\n";

    return (numFailures);
}

#endif
