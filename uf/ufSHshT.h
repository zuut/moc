/*
 * See Readme, Notice and License files.
 */
#ifndef _ufSHshT_h_
#define _ufSHshT_h_

#include "ufDef.h"
#include "ufPtr.h"
#include "ufString.h"

#define ufStrHash(TYPE) TYPE##SHT
#define ufStrHashIter(TYPE) TYPE##SHTI

#define ufStrHashdeclare(TYPE)                                                 \
                                                                               \
    class ufStrHashIter(TYPE);                                                 \
                                                                               \
    class ufStrHash(TYPE) {                                                    \
        friend class ufStrHashIter(TYPE);                                      \
                                                                               \
    public:                                                                    \
        ufStrHash(TYPE)(int nSize = 17); /* pick a prime */                    \
        virtual ~ufStrHash(TYPE)();                                            \
                                                                               \
        int Insert(const char *lpszKey, const ufPtr(TYPE) & data);             \
        int Remove(const char *lpszKey);                                       \
        int Find(const char *lpszKey, ufPtr(TYPE) & data) const;               \
                                                                               \
        int GetNumElements() const;                                            \
                                                                               \
        void MakeEmpty();                                                      \
        inline bool isEmpty() const;                                           \
        inline bool isNonEmpty() const;                                        \
                                                                               \
        operator PTR_INT() const;                                              \
        int operator!() const;                                                 \
                                                                               \
    protected:                                                                 \
        int fNumElements;                                                      \
        int fNumBuckets;                                                       \
        /*pointer array of hash entries*/                                      \
        ufStrHashIter(TYPE) * *fBuckets;                                       \
        ufStrHashIter(TYPE) * *fLastBucket;                                    \
    };                                                                         \
                                                                               \
    class ufStrHashIter(TYPE) {                                                \
        friend class ufStrHash(TYPE);                                          \
                                                                               \
    public:                                                                    \
        ufStrHashIter(TYPE)();                                                 \
        ufStrHashIter(TYPE)(const ufStrHashIter(TYPE) & old);                  \
        ufStrHashIter(TYPE)(const ufStrHash(TYPE) & table);                    \
        virtual ~ufStrHashIter(TYPE)();                                        \
                                                                               \
        ufString fKey;                                                         \
        ufPtr(TYPE) fData;                                                     \
                                                                               \
        ufStrHashIter(TYPE) &operator=(const ufStrHashIter(TYPE) & old);       \
        ufStrHashIter(TYPE) &operator=(const ufStrHash(TYPE) & table);         \
        operator PTR_INT() const;                                              \
        ufStrHashIter(TYPE) &operator++();                                     \
        ufStrHashIter(TYPE) &operator++(int);                                  \
        ufStrHashIter(TYPE) &operator+=(int);                                  \
        ufStrHashIter(TYPE) operator+(int) const;                              \
                                                                               \
    protected:                                                                 \
        ufStrHashIter(TYPE) * *fOwnerBucket;                                   \
        ufStrHashIter(TYPE) * *fLastBucket;                                    \
        ufStrHashIter(TYPE) * fNext;                                           \
    };                                                                         \
                                                                               \
    inline ufStrHash(TYPE)::ufStrHash(TYPE)(int nSize) {                       \
        ufSHConstruct((ufGenericStrHash *)this, nSize);                        \
    }                                                                          \
                                                                               \
    inline int ufStrHash(TYPE)::Insert(const char *lpszKey,                    \
                                       const ufPtr(TYPE) & data) {             \
        return ufSHInsert_((ufGenericStrHash *)this, lpszKey,                  \
                           (ufPtr(void) *)&data);                              \
    }                                                                          \
                                                                               \
    inline int ufStrHash(TYPE)::Remove(const char *lpszKey) {                  \
        ufPtr(TYPE) tmp;                                                       \
        int ret = ufSHRemove_((ufGenericStrHash *)this, lpszKey,               \
                              (ufPtr(void) *)&tmp);                            \
        tmp.MakeEmpty();                                                       \
        return ret;                                                            \
    }                                                                          \
                                                                               \
    inline int ufStrHash(TYPE)::Find(const char *lpszKey, ufPtr(TYPE) & data)  \
        const {                                                                \
        if (data != 0) {                                                       \
            data.MakeEmpty();                                                  \
        }                                                                      \
        return ufSHFind_((ufGenericStrHash *)this, lpszKey,                    \
                         (ufPtr(void) *)&data);                                \
    }                                                                          \
                                                                               \
    inline int ufStrHash(TYPE)::GetNumElements() const {                       \
        return fNumElements;                                                   \
    }                                                                          \
                                                                               \
    inline ufStrHash(TYPE)::operator PTR_INT() const {                         \
        return fNumElements > 0 ? (PTR_INT)this : 0;                           \
    }                                                                          \
                                                                               \
    inline int ufStrHash(TYPE)::operator!() const { return !fNumElements; }    \
                                                                               \
    inline ufStrHashIter(TYPE)::operator PTR_INT() const {                     \
        return fData.GetPointerAsInt();                                        \
    }

#define ufStrHashimplement(TYPE)                                               \
                                                                               \
    ufStrHash(TYPE)::~ufStrHash(TYPE)() {                                      \
        MakeEmpty();                                                           \
        delete[] fBuckets;                                                     \
        fBuckets = 0;                                                          \
    }                                                                          \
                                                                               \
    void ufStrHash(TYPE)::MakeEmpty() {                                        \
        ufStrHashIter(TYPE) * *b;                                              \
        ufStrHashIter(TYPE) * i;                                               \
        ufStrHashIter(TYPE) * next;                                            \
                                                                               \
        for (b = fBuckets; b != fLastBucket; b++) {                            \
            for (i = *b; i; i = next) {                                        \
                next = i->fNext;                                               \
                delete i;                                                      \
            }                                                                  \
            *b = 0;                                                            \
        }                                                                      \
    }                                                                          \
                                                                               \
    bool ufStrHash(TYPE)::isNonEmpty() const { return fNumElements > 0; }      \
                                                                               \
    bool ufStrHash(TYPE)::isEmpty() const { return fNumElements == 0; }        \
                                                                               \
    ufStrHashIter(TYPE)::ufStrHashIter(TYPE)()                                 \
        : fKey(), fData(), fOwnerBucket(0), fLastBucket(0), fNext(0) {}        \
                                                                               \
    ufStrHashIter(TYPE)::ufStrHashIter(TYPE)(const ufStrHashIter(TYPE) & old)  \
        : fKey(old.fKey), fData(old.fData),                                    \
          fOwnerBucket((ufStrHashIter(TYPE) **)old.fOwnerBucket),              \
          fLastBucket((ufStrHashIter(TYPE) **)old.fLastBucket),                \
          fNext((ufStrHashIter(TYPE) *)old.fNext) {}                           \
                                                                               \
    ufStrHashIter(TYPE)::ufStrHashIter(TYPE)(const ufStrHash(TYPE) & table)    \
        : fKey(), fData(), fOwnerBucket(0), fLastBucket(0), fNext(0) {         \
        *this = table;                                                         \
    }                                                                          \
                                                                               \
    ufStrHashIter(TYPE)::~ufStrHashIter(TYPE)() {}                             \
                                                                               \
    ufStrHashIter(TYPE) &                                                      \
        ufStrHashIter(TYPE)::operator=(const ufStrHashIter(TYPE) & old) {      \
        fData = ((ufStrHashIter(TYPE) &)old).fData;                            \
        fKey = ((ufStrHashIter(TYPE) &)old).fKey;                              \
        fOwnerBucket = ((ufStrHashIter(TYPE) &)old).fOwnerBucket;              \
        fLastBucket = ((ufStrHashIter(TYPE) &)old).fLastBucket;                \
        fNext = ((ufStrHashIter(TYPE) &)old).fNext;                            \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    ufStrHashIter(TYPE) &                                                      \
        ufStrHashIter(TYPE)::operator=(const ufStrHash(TYPE) & table) {        \
        if (table.fNumElements > 0 && table.fBuckets) {                        \
            ufStrHashIter(TYPE) **b = ((ufStrHash(TYPE) &)table).fBuckets;     \
            ufStrHashIter(TYPE) **last =                                       \
                ((ufStrHash(TYPE) &)table).fLastBucket;                        \
            while (b != last && *b == 0)                                       \
                b++;                                                           \
            fData = b[0]->fData;                                               \
            fKey = b[0]->fKey;                                                 \
            fOwnerBucket = b;                                                  \
            fLastBucket = last;                                                \
            fNext = b[0]->fNext;                                               \
        } else {                                                               \
            fData.MakeEmpty();                                                 \
            fKey = 0;                                                          \
            fLastBucket = fOwnerBucket = 0;                                    \
            fNext = 0;                                                         \
        }                                                                      \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    ufStrHashIter(TYPE) & ufStrHashIter(TYPE)::operator++() {                  \
        if (fData != 0) {                                                      \
            fData.MakeEmpty();                                                 \
        }                                                                      \
        ufSHAdd_((ufGenericStrHashIter *)this, (ufGenericStrHashIter *)this,   \
                 1);                                                           \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    ufStrHashIter(TYPE) & ufStrHashIter(TYPE)::operator++(int) {               \
        if (fData != 0) {                                                      \
            fData.MakeEmpty();                                                 \
        }                                                                      \
        ufSHAdd_((ufGenericStrHashIter *)this, (ufGenericStrHashIter *)this,   \
                 1);                                                           \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    ufStrHashIter(TYPE) & ufStrHashIter(TYPE)::operator+=(int n) {             \
        if (fData != 0) {                                                      \
            fData.MakeEmpty();                                                 \
        }                                                                      \
        ufSHAdd_((ufGenericStrHashIter *)this, (ufGenericStrHashIter *)this,   \
                 n);                                                           \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    ufStrHashIter(TYPE) ufStrHashIter(TYPE)::operator+(int n) const {          \
        ufStrHashIter(TYPE) tmp;                                               \
        ufSHAdd_((ufGenericStrHashIter *)&tmp, (ufGenericStrHashIter *)this,   \
                 n);                                                           \
        return tmp;                                                            \
    }

class ufGenericStrHash;

class ufGenericStrHashIter;

UF_API(void) ufSHConstruct(ufGenericStrHash *lpTable, int nSize);
UF_API(int)
ufSHInsert_(ufGenericStrHash *lpTable, const char *lpszKey,
            ufPtr(void) * lpData);
UF_API(int)
ufSHRemove_(ufGenericStrHash *lpTable, const char *lpszKey,
            ufPtr(void) * lpData);
UF_API(int)
ufSHFind_(ufGenericStrHash *lpTable, const char *lpszKey, ufPtr(void) * lpData);
UF_API(void)
ufSHAdd_(ufGenericStrHashIter *lpIter1, ufGenericStrHashIter *lpIter2,
         int nIncr);

#endif
