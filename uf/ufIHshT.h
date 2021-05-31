/*
 * See Readme, Notice and License files.
 */
#ifndef _ufIHshT_h_
#define _ufIHshT_h_

#include "ufPtr.h"

#define ufIntHash(TYPE) TYPE##IHT
#define ufIntHashIter(TYPE) TYPE##IHTI

typedef void (*ufIHMakeEmptyFnc)(void*);

#define ufIntHashdeclare(TYPE)                                                \
                                                                              \
    class ufIntHashIter(TYPE);                                                \
                                                                              \
    class ufIntHash(TYPE) {                                                   \
        friend class ufIntHashIter(TYPE);                                     \
                                                                              \
    public:                                                                   \
        ufIntHash(TYPE)(int nSize = 15); /* pick 2**N - 1 */                  \
        virtual ~ufIntHash(TYPE)();                                           \
                                                                              \
        int Insert(const int key, const ufPtr(TYPE) & data);                  \
        int Remove(const int key);                                            \
        int Find(const int key, ufPtr(TYPE) & data) const;                    \
                                                                              \
        int GetNumElements() const;                                           \
                                                                              \
        void MakeEmpty();                                                     \
        inline BOOL isEmpty() const;                                          \
        inline BOOL isNonEmpty() const;                                       \
                                                                              \
        operator PTR_INT() const;                                             \
        int operator!() const;                                                \
                                                                              \
    protected:                                                                \
        int fNumElements;                                                     \
        int fNumBuckets;                                                      \
        /*pointer array of hash entries*/                                     \
        ufIntHashIter(TYPE) * *fBuckets;                                      \
        ufIntHashIter(TYPE) * *fLastBucket;                                   \
    };                                                                        \
                                                                              \
    class ufIntHashIter(TYPE) {                                               \
        friend class ufIntHash(TYPE);                                         \
                                                                              \
    public:                                                                   \
        ufIntHashIter(TYPE)();                                                \
        ufIntHashIter(TYPE)(const ufIntHashIter(TYPE) & old);                 \
        ufIntHashIter(TYPE)(const ufIntHash(TYPE) & table);                   \
        virtual ~ufIntHashIter(TYPE)();                                       \
                                                                              \
        int fKey;                                                             \
        ufPtr(TYPE) fData;                                                    \
                                                                              \
        ufIntHashIter(TYPE) &operator=(const ufIntHashIter(TYPE) & old);      \
        ufIntHashIter(TYPE) &operator=(const ufIntHash(TYPE) & table);        \
        operator PTR_INT() const;                                             \
        ufIntHashIter(TYPE) &operator++();                                    \
        ufIntHashIter(TYPE) &operator++(int);                                 \
        ufIntHashIter(TYPE) &operator+=(int);                                 \
        ufIntHashIter(TYPE) operator+(int) const;                             \
                                                                              \
    protected:                                                                \
        ufIntHashIter(TYPE) * *fOwnerBucket;                                  \
        ufIntHashIter(TYPE) * *fLastBucket;                                   \
        ufIntHashIter(TYPE) * fNext;                                          \
        static void MakeEmpty(void *iter);                                    \
    };                                                                        \
                                                                              \
    inline ufIntHash(TYPE)::ufIntHash(TYPE)(int size) {                       \
        ufIHConstruct((ufGenericIntHash *)this, size);                        \
    }                                                                         \
                                                                              \
    inline int ufIntHash(TYPE)::Insert(const int key,                         \
                                       const ufPtr(TYPE) & data) {            \
        return ufIHInsert_((ufGenericIntHash *)this, key,                     \
                           (ufPtr(void) *)&data);                             \
    }                                                                         \
                                                                              \
    inline int ufIntHash(TYPE)::Remove(const int key) {                       \
        ufPtr(TYPE) tmp;                                                      \
        int ret =                                                             \
            ufIHRemove_((ufGenericIntHash *)this, key, (ufPtr(void) *)&tmp);  \
        tmp.MakeEmpty();                                                      \
        return ret;                                                           \
    }                                                                         \
                                                                              \
    inline int ufIntHash(TYPE)::Find(const int key, ufPtr(TYPE) & data)       \
        const {                                                               \
        if (data.isValid()) {                                                 \
            data.MakeEmpty();                                                 \
        }                                                                     \
        return ufIHFind_((ufGenericIntHash *)this, key, (ufPtr(void) *)&data);\
    }                                                                         \
                                                                              \
    inline int ufIntHash(TYPE)::GetNumElements() const {                      \
        return fNumElements;                                                  \
    }                                                                         \
                                                                              \
    inline ufIntHash(TYPE)::operator PTR_INT() const {                        \
        return fNumElements > 0 ? (PTR_INT)this : 0;                          \
    }                                                                         \
                                                                              \
    inline int ufIntHash(TYPE)::operator!() const { return !fNumElements; }   \
                                                                              \
    inline ufIntHashIter(TYPE)::operator PTR_INT() const {                    \
        return fData.GetPointerAsInt();                                       \
    }


#define ufIntHashimplement(TYPE)                                              \
                                                                              \
    ufIntHash(TYPE)::~ufIntHash(TYPE)() {                                     \
        MakeEmpty();                                                          \
        ufIHDeconstruct((ufGenericIntHash *)this);                            \
    }                                                                         \
                                                                              \
    void ufIntHash(TYPE)::MakeEmpty() {                                       \
        ufIHMakeEmpty_((ufGenericIntHash *)this,                              \
                       &ufIntHashIter(TYPE)::MakeEmpty);                      \
    }                                                                         \
                                                                              \
    BOOL ufIntHash(TYPE)::isNonEmpty() const { return fNumElements > 0; }     \
                                                                              \
    BOOL ufIntHash(TYPE)::isEmpty() const { return fNumElements == 0; }       \
                                                                              \
    ufIntHashIter(TYPE)::ufIntHashIter(TYPE)()                                \
        : fData(), fKey(0), fNext(0), fOwnerBucket(0), fLastBucket(0) {}      \
                                                                              \
    ufIntHashIter(TYPE)::ufIntHashIter(TYPE)(const ufIntHashIter(TYPE) & old) \
        : fData(old.fData), fKey(old.fKey),                                   \
          fNext((ufIntHashIter(TYPE) *)old.fNext),                            \
          fOwnerBucket((ufIntHashIter(TYPE) **)old.fOwnerBucket),             \
          fLastBucket((ufIntHashIter(TYPE) **)old.fLastBucket) {}             \
                                                                              \
    ufIntHashIter(TYPE)::ufIntHashIter(TYPE)(const ufIntHash(TYPE) & table)   \
        : fKey(0) {                                                           \
        *this = table;                                                        \
    }                                                                         \
                                                                              \
    ufIntHashIter(TYPE)::~ufIntHashIter(TYPE)() {}                            \
                                                                              \
    void ufIntHashIter(TYPE)::MakeEmpty(void* iter) {                         \
        ufIntHashIter(TYPE)* lpIter = (ufIntHashIter(TYPE)*)iter;             \
        lpIter->fData.MakeEmpty();                                            \
        lpIter->fKey = 0;                                                     \
        lpIter->fOwnerBucket = 0;                                             \
        lpIter->fLastBucket = 0;                                              \
        lpIter->fNext = 0;                                                    \
    }                                                                         \
                                                                              \
    ufIntHashIter(TYPE) &                                                     \
        ufIntHashIter(TYPE)::operator=(const ufIntHashIter(TYPE) & old) {     \
        if ( (&old) == this ) { return *this; }                               \
        fData.MakeEmpty();                                                    \
        fData = ((ufIntHashIter(TYPE) &)old).fData;                           \
        fKey = old.fKey;                                                      \
        fOwnerBucket = ((ufIntHashIter(TYPE) &)old).fOwnerBucket;             \
        fLastBucket = ((ufIntHashIter(TYPE) &)old).fLastBucket;               \
        fNext = ((ufIntHashIter(TYPE) &)old).fNext;                           \
        return *this;                                                         \
    }                                                                         \
                                                                              \
    ufIntHashIter(TYPE) &                                                     \
        ufIntHashIter(TYPE)::operator=(const ufIntHash(TYPE) & table) {       \
        if (table.fNumElements > 0 && table.fBuckets) {                       \
            ufIntHashIter(TYPE) **b = ((ufIntHash(TYPE) &)table).fBuckets;    \
            ufIntHashIter(TYPE) **last =                                      \
                ((ufIntHash(TYPE) &)table).fLastBucket;                       \
            while (b != last && *b == 0)                                      \
                b++;                                                          \
            fData = b[0]->fData;                                              \
            fKey = b[0]->fKey;                                                \
            fOwnerBucket = b;                                                 \
            fLastBucket = last;                                               \
            fNext = b[0]->fNext;                                              \
        } else {                                                              \
            fData.MakeEmpty();                                                \
            fKey = 0;                                                         \
            fLastBucket = fOwnerBucket = 0;                                   \
            fNext = 0;                                                        \
        }                                                                     \
        return *this;                                                         \
    }                                                                         \
                                                                              \
    ufIntHashIter(TYPE) & ufIntHashIter(TYPE)::operator++() {                 \
        if (fData.isValid()) {                                                \
            fData.MakeEmpty();                                                \
        }                                                                     \
        ufIHAdd_((ufGenericIntHashIter *)this, (ufGenericIntHashIter *)this,  \
                 1);                                                          \
        return *this;                                                         \
    }                                                                         \
                                                                              \
    ufIntHashIter(TYPE) & ufIntHashIter(TYPE)::operator++(int) {              \
        if (fData.isValid()) {                                                \
            fData.MakeEmpty();                                                \
        }                                                                     \
        ufIHAdd_((ufGenericIntHashIter *)this, (ufGenericIntHashIter *)this,  \
                 1);                                                          \
        return *this;                                                         \
    }                                                                         \
                                                                              \
    ufIntHashIter(TYPE) & ufIntHashIter(TYPE)::operator+=(int n) {            \
        if (fData.isValid()) {                                                \
            fData.MakeEmpty();                                                \
        }                                                                     \
        ufIHAdd_((ufGenericIntHashIter *)this, (ufGenericIntHashIter *)this,  \
                 n);                                                          \
        return *this;                                                         \
    }                                                                         \
                                                                              \
    ufIntHashIter(TYPE) ufIntHashIter(TYPE)::operator+(int n) const {         \
        ufIntHashIter(TYPE) tmp;                                              \
        ufIHAdd_((ufGenericIntHashIter *)&tmp, (ufGenericIntHashIter *)this,  \
                 n);                                                          \
        return tmp;                                                           \
    }

class ufGenericIntHash;

class ufGenericIntHashIter;

UF_API(void) ufIHConstruct(ufGenericIntHash *tbl, int size);
UF_API(void) ufIHDeconstruct(ufGenericIntHash *tbl);

UF_API(int)
ufIHInsert_(ufGenericIntHash *tbl, const int key, ufPtr(void) * data);
UF_API(int) ufIHRemove_(ufGenericIntHash *tbl, const int key, ufPtr(void) * d);

UF_API(int) ufIHFind_(ufGenericIntHash *tbl, const int key, ufPtr(void) * data);
UF_API(void)
ufIHAdd_(ufGenericIntHashIter *i1, ufGenericIntHashIter *i2, int incr);

UF_API(void) ufIHMakeEmpty_(ufGenericIntHash *tbl, ufIHMakeEmptyFnc f);

#endif
