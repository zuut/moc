/*
 * See Readme, Notice and License files.
 */
#ifndef _ufList_h_
#define _ufList_h_

#include "ufDef.h"
#include "ufPtr.h"

#define ufListSortFnc(TYPE) TYPE##LSTSortFnc
#define ufList(TYPE) TYPE##LST
#define ufListIter(TYPE) TYPE##LSTI

enum ufCompareResult { GreaterThan, EqualTo, LessThan };

#define ufListdeclare(TYPE)                                                    \
    class ufListIter(TYPE);                                                    \
                                                                               \
    typedef ufCompareResult (*ufListSortFnc(TYPE))(ufPtr(TYPE) * element1,     \
                                                   ufPtr(TYPE) * element2);    \
                                                                               \
    class ufList(TYPE) {                                                       \
        friend class ufListIter(TYPE);                                         \
                                                                               \
    public:                                                                    \
        ufList(TYPE)();                                                        \
        virtual ~ufList(TYPE)();                                               \
                                                                               \
        void InsertFirst(ufPtr(TYPE) & data);                                  \
        void InsertLast(ufPtr(TYPE) & data);                                   \
        void InsertNth(ufPtr(TYPE) & data, int nth);                           \
                                                                               \
        ufPtr(TYPE) RemoveFirst();                                             \
        ufPtr(TYPE) RemoveLast();                                              \
        ufPtr(TYPE) RemoveNth(int nth);                                        \
                                                                               \
        ufPtr(TYPE) PeekFirst() const;                                         \
        ufPtr(TYPE) PeekLast() const;                                          \
        ufPtr(TYPE) PeekNth(int nth) const;                                    \
                                                                               \
        void SortInIncreasingOrder(ufListSortFnc(TYPE) fnc);                   \
                                                                               \
        void MakeEmpty();                                                      \
        int GetNumElements() const;                                            \
        operator PTR_INT() const;                                              \
        int operator!() const;                                                 \
                                                                               \
    protected:                                                                 \
        ufList(TYPE)(const ufList(TYPE) &);                                    \
        ufListIter(TYPE) * fFirst;                                             \
        ufListIter(TYPE) * fLast;                                              \
                                                                               \
    public: /*implemenation*/                                                  \
        ufGenericList *This() const { return (ufGenericList *)(void *)this; }  \
    };                                                                         \
                                                                               \
    class ufListIter(TYPE) {                                                   \
        friend class ufList(TYPE);                                             \
                                                                               \
    public:                                                                    \
        ufListIter(TYPE)();                                                    \
        ufListIter(TYPE)(const ufListIter(TYPE) & iter);                       \
        ufListIter(TYPE)(ufList(TYPE) & list);                                 \
        ufListIter(TYPE)(ufList(TYPE) & list, int nth);                        \
        virtual ~ufListIter(TYPE)();                                           \
                                                                               \
        ufPtr(TYPE) fData;                                                     \
                                                                               \
        ufListIter(TYPE) &operator=(const ufListIter(TYPE) & old);             \
        ufListIter(TYPE) &operator=(const ufList(TYPE) & list);                \
        operator PTR_INT() const;                                              \
        ufListIter(TYPE) &operator++();                                        \
        ufListIter(TYPE) &operator++(int);                                     \
        ufListIter(TYPE) &operator+=(int);                                     \
        ufListIter(TYPE) operator+(int) const;                                 \
                                                                               \
    protected:                                                                 \
        ufListIter(TYPE) * fPrev;                                              \
        ufListIter(TYPE) * fNext;                                              \
        static void SetPtr(ufPtr(void) *, ufPtr(void) *);                      \
        static void MakeEmpty(void *iter);                                     \
                                                                               \
    public: /*implemenation*/                                                  \
        ufGenericListIter *This() const {                                      \
            return (ufGenericListIter *)(void *)this;                          \
        }                                                                      \
    };                                                                         \
                                                                               \
    inline void ufList(TYPE)::MakeEmpty() {                                    \
        ufListMakeEmpty_(This(), ufListIter(TYPE)::MakeEmpty);                 \
    }                                                                          \
                                                                               \
    inline int ufList(TYPE)::GetNumElements() const {                          \
        return ufListNumElements_(This());                                     \
    }                                                                          \
                                                                               \
    inline ufList(TYPE)::operator PTR_INT() const { return (PTR_INT)fFirst; }  \
                                                                               \
    inline int ufList(TYPE)::operator!() const { return !fFirst; }             \
                                                                               \
    inline void ufList(TYPE)::InsertFirst(ufPtr(TYPE) & data) {                \
        ufListInsertFirst_(This(), (ufPtr(void) *)&data);                      \
    }                                                                          \
                                                                               \
    inline void ufList(TYPE)::InsertLast(ufPtr(TYPE) & data) {                 \
        ufListInsertLast_(This(), (ufPtr(void) *)&data);                       \
    }                                                                          \
                                                                               \
    inline void ufList(TYPE)::InsertNth(ufPtr(TYPE) & data, int nth) {         \
        ufListInsertNth_(This(), (ufPtr(void) *)&data, nth);                   \
    }                                                                          \
                                                                               \
    inline ufPtr(TYPE) ufList(TYPE)::RemoveFirst() {                           \
        ufPtr(TYPE) dataReturn;                                                \
        ufListRemoveFirst_(This(), (ufPtr(void) *)&dataReturn);                \
        return dataReturn;                                                     \
    }                                                                          \
                                                                               \
    inline ufPtr(TYPE) ufList(TYPE)::RemoveLast() {                            \
        ufPtr(TYPE) dataReturn;                                                \
        ufListRemoveLast_(This(), (ufPtr(void) *)&dataReturn);                 \
        return dataReturn;                                                     \
    }                                                                          \
                                                                               \
    inline ufPtr(TYPE) ufList(TYPE)::RemoveNth(int nth) {                      \
        ufPtr(TYPE) dataReturn;                                                \
        ufListRemoveNth_(This(), (ufPtr(void) *)&dataReturn, nth);             \
        return dataReturn;                                                     \
    }                                                                          \
                                                                               \
    inline void ufList(TYPE)::SortInIncreasingOrder(ufListSortFnc(TYPE) fnc) { \
        ufListSortIncr_(This(), (ufListSortFnc(void))fnc);                     \
    }                                                                          \
                                                                               \
    inline ufPtr(TYPE) ufList(TYPE)::PeekFirst() const {                       \
        ufPtr(TYPE) dataReturn;                                                \
        ufListPeekFirst_(This(), (ufPtr(void) *)&dataReturn);                  \
        return dataReturn;                                                     \
    }                                                                          \
                                                                               \
    inline ufPtr(TYPE) ufList(TYPE)::PeekLast() const {                        \
        ufPtr(TYPE) dataReturn;                                                \
        ufListPeekLast_(This(), (ufPtr(void) *)&dataReturn);                   \
        return dataReturn;                                                     \
    }                                                                          \
                                                                               \
    inline ufPtr(TYPE) ufList(TYPE)::PeekNth(int nth) const {                  \
        ufPtr(TYPE) dataReturn;                                                \
        ufListPeekNth_(This(), (ufPtr(void) *)&dataReturn, nth);               \
        return dataReturn;                                                     \
    }                                                                          \
                                                                               \
    inline ufListIter(TYPE) &                                                  \
        ufListIter(TYPE)::operator=(const ufList(TYPE) & list) {               \
        ufListIterAssignList_(This(), list.This(), &ufListIter(TYPE)::SetPtr); \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    inline ufListIter(TYPE)::operator PTR_INT() const {                        \
        return fData.GetPointerAsInt();                                        \
    }                                                                          \
                                                                               \
    inline ufListIter(TYPE) & ufListIter(TYPE)::operator++() {                 \
        ufListIterAdd_(This(), This(), 1, &ufListIter(TYPE)::SetPtr);          \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    inline ufListIter(TYPE) & ufListIter(TYPE)::operator++(int) {              \
        ufListIterAdd_(This(), This(), 1, &ufListIter(TYPE)::SetPtr);          \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    inline ufListIter(TYPE) & ufListIter(TYPE)::operator+=(int incr) {         \
        ufListIterAdd_(This(), This(), incr, &ufListIter(TYPE)::SetPtr);       \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    inline ufListIter(TYPE) ufListIter(TYPE)::operator+(int incr) const {      \
        ufListIter(TYPE) i;                                                    \
        ufListIterAdd_(i.This(), This(), incr, &ufListIter(TYPE)::SetPtr);     \
        return i;                                                              \
    }

#define ufListimplement(TYPE)                                                  \
                                                                               \
    ufList(TYPE)::ufList(TYPE)() : fFirst(0), fLast(0) {}                      \
                                                                               \
    ufList(TYPE)::ufList(TYPE)(const ufList(TYPE) &)                           \
        : fFirst(0), fLast(0){ufASSERT(0)}                                     \
                                                                               \
          ufList(TYPE)::~ufList(TYPE)() {                                      \
        MakeEmpty();                                                           \
    }                                                                          \
                                                                               \
    ufListIter(TYPE)::ufListIter(TYPE)() : fData(), fPrev(0), fNext(0) {}      \
                                                                               \
    ufListIter(TYPE)::ufListIter(TYPE)(const ufListIter(TYPE) & iter)          \
        : fData(iter.fData), fPrev(iter.fPrev), fNext(iter.fPrev) {}           \
                                                                               \
    ufListIter(TYPE)::ufListIter(TYPE)(ufList(TYPE) & list, int nth) {         \
        ufListIterAdd_(This(), list.fFirst->This(), nth,                       \
                       &ufListIter(TYPE)::SetPtr);                             \
    }                                                                          \
                                                                               \
    ufListIter(TYPE)::ufListIter(TYPE)(ufList(TYPE) & list) : fData() {        \
        ufListIterAssignList_(This(), list.This(), &ufListIter(TYPE)::SetPtr); \
    }                                                                          \
                                                                               \
    ufListIter(TYPE)::~ufListIter(TYPE)() { MakeEmpty(this); }                 \
                                                                               \
    ufListIter(TYPE) &                                                         \
        ufListIter(TYPE)::operator=(const ufListIter(TYPE) & old) {            \
        fData = old.fData;                                                     \
        fNext = old.fNext;                                                     \
        fPrev = old.fPrev;                                                     \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    void ufListIter(TYPE)::MakeEmpty(void *iter) {                             \
        ufListIter(TYPE) *i = (ufListIter(TYPE) *)iter;                        \
        i->fData.MakeEmpty();                                                  \
        i->fPrev = i->fNext = 0;                                               \
    }                                                                          \
                                                                               \
    void ufListIter(TYPE)::SetPtr(ufPtr(void) * data1p,                        \
                                  ufPtr(void) * data2p) {                      \
        if (data2p) {                                                          \
            *((ufPtr(TYPE) *)data1p) = *((ufPtr(TYPE) *)data2p);               \
        } else {                                                               \
            ((ufPtr(TYPE) *)data1p)->MakeEmpty();                              \
        }                                                                      \
    }

class ufGenericList;

class ufGenericListIter;

typedef ufCompareResult (*ufListSortFnc(void))(ufPtr(void) *, ufPtr(void) *);

typedef void (*ufListMakeEmptyFnc)(void *);

typedef void (*ufListSetPtrFnc)(ufPtr(void) *, ufPtr(void) *);

UF_API(int) ufListNumElements_(ufGenericList *lpList);
UF_API(void) ufListInsertFirst_(ufGenericList *lpList, ufPtr(void) * lpData);
UF_API(void) ufListInsertLast_(ufGenericList *lpList, ufPtr(void) * lpData);
UF_API(void)
ufListInsertNth_(ufGenericList *lpList, ufPtr(void) * lpData, int nth);
UF_API(ufPtr(void) *)
ufListRemoveFirst_(ufGenericList *lpList, ufPtr(void) * lpReturn);
UF_API(ufPtr(void) *)
ufListRemoveLast_(ufGenericList *lpList, ufPtr(void) * lpReturn);
UF_API(ufPtr(void) *)
ufListRemoveNth_(ufGenericList *lpList, ufPtr(void) * lpReturn, int nth);

UF_API(ufPtr(void) *)
ufListPeekFirst_(ufGenericList *lpList, ufPtr(void) * lpReturn);
UF_API(ufPtr(void) *)
ufListPeekLast_(ufGenericList *lpList, ufPtr(void) * lpReturn);
UF_API(ufPtr(void) *)
ufListPeekNth_(ufGenericList *lpList, ufPtr(void) * lpReturn, int nth);
UF_API(void) ufListMakeEmpty_(ufGenericList *lpList, ufListMakeEmptyFnc);
UF_API(void)
ufListIterAdd_(ufGenericListIter *i, ufGenericListIter *start, int count,
               ufListSetPtrFnc unref);
UF_API(void)
ufListIterAssignList_(ufGenericListIter *i, const ufGenericList *list,
                      ufListSetPtrFnc f);

UF_API(void) ufListSortIncr_(ufGenericList *i, ufListSortFnc(void) f);

#endif
