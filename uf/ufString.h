/*
 * See Readme, Notice and License files.
 */
#ifndef _ufString_h
#define _ufString_h

#include "generic.h"
#include "ufDef.h"
#include <ctype.h>

/*
 * This file defines the following
 *      CONSTANTS
 *              const char dirSepChar = '\\';
 *              extern const char dirSepString[] //"\\"
 *      MACROS
 *      classes
 *              class ufString;
 *      global instances
 *              NONE.
 *      functions
 */

class ufStringLiteral;
class ufStealable {};
extern const ufStealable MAKE_OWNER;

class ufString {
    friend class ufStringTester;
    ufFRIEND_ARCHIVE_OPERATORS(ufString)
public :
    ufString();

    ufString(const ufString &string);

    ufString(const ufStealable& makeOwner, ufString &string);

    ufString(const char *lpszString);

    ~ufString();

    int GetLength() const;

    BOOL IsEmpty();
    void MakeEmpty();

    void LoseReference();

    void MakeStringLiteral(const char* stringLiteral);

    // this assumes ownership of the buffer. sets buf = 0 when done
    void TakeOwnership(char*& buf);
    void MakeOwner(char* buf); // buf isn't changed, but ufString is the owner

    void TakeOwnership(ufTmpBuf& bufToTakeCharArray);

    // if you need to modify a string, use a string cursor to do so.
    // string cursor ensures that the string is modifable.
    char operator [] (int index) const;

    ufString &operator=(const ufString &string);

    ufString &operator=(const char *lpszString);

    ufCOMPARISON_MEMBER_OPERATORS(const ufString&, Compare);
    ufCOMPARISON_MEMBER_OPERATORS(const char*, Compare);
#ifdef USE_STL_STREAMS
    ufSTREAM_OUT_MEMBERS(std::ostream &,const ufString&, WriteTo);
    ufSTREAM_IN_MEMBERS(std::istream &, ufString&, ReadFrom);
#endif
    operator const char *() const;
    const char *CStr() const { return fString; }
    char *CStrForMod(); // will convert fString to a writable string if needed.
    ufString Literal();
    //    operator PTR_INT() const { return (PTR_INT)fString; }
protected:
    const char *fString;
    BOOL fOwnString;
    #define ufSTRING_INIT_DEFAULT 0
    #define ufSTRING_INIT_FORCE_DUPLICATE 1
    #define ufSTRING_INIT_STEAL 2
    void Init(const char*& lpszString, BOOL& ownString,
              int action);
};
ufCOMPARISON_INLINE_GLOBAL_OPERATORS_TO_MEMBER_FUNCS(const char*, const ufString&, Compare);
#ifdef USE_STL_STREAMS
ufSTREAM_INLINE_GLOBAL_OUT(std::ostream&, const ufString&, WriteTo);
ufSTREAM_INLINE_GLOBAL_IN(std::istream&, ufString&, ReadFrom);
#endif

// ufStringReadonlyCursor has only 1 field const char* fBuf
// and no virtual methods so
// sizeof(ufStringReadonlyCursor) == sizeof(const char*)
// Please keep it this way.
class ufStringReadonlyCursor {
public:
    ufStringReadonlyCursor(const char* buf);
    ufStringReadonlyCursor(const ufStringReadonlyCursor& o) : fBuf(o.fBuf) {}
    ufStringReadonlyCursor(const ufString& str);

    int CalculateNewPos(int oldColumn);

    BOOL SkipTo(char c);
    char SkipSpaces();
    char SkipSpacesAnd(char c1, char c2=' ', char c3=' ', char c4=' ');

    BOOL Contains(char c);
    BOOL IsLower();
    BOOL IsUpper();
    BOOL IsEmpty();
    BOOL IsBlankLine();
    BOOL IsEqual(const char c);
    BOOL IsEqual(const char*c);
    BOOL IsEqual(const char*c, int n);
    BOOL StartsWith(const char*c);

    // copies the a token from the head of the cursor and advances the
    // cursor to the next token.
    BOOL ParseNextToken(ufTmpBuf &tokenBuf);

    ufStringReadonlyCursor& operator +=(int offset) {
        fBuf += offset;
        return *this;
    }
    ufStringReadonlyCursor& operator ++() {
        operator += (1);
        return *this;
    }
    ufStringReadonlyCursor operator ++(int) {
        ufStringReadonlyCursor beforeChange(*this);
        ++*this;
        return beforeChange;
    }

    const char& operator *() const { return *fBuf; };
    const char& operator [] (int index) const  { return fBuf[index]; };
    operator const char *() const { return fBuf; };

    ufStringReadonlyCursor &operator=(const char* lpszString) {
        fBuf = lpszString;
        return *this;
    }
    ufStringReadonlyCursor &operator=(const ufString &string) {
        fBuf = string.CStr();
        return *this;
    }

    const char* Buf() const { return fBuf; }
protected:
    char* BufToMod() { return (char*)fBuf; }
    void Init(const char* buf);
    ufStringReadonlyCursor() : fBuf(0) {}
private:
    const char* fBuf;
private:
    /*disallowed*/
    ufStringReadonlyCursor(const ufTmpBuf& buf);
};

// ufStringReadonlyCursor & ufStringCursor have only 1 field
// const char* fBuf and no virtual methods so
// sizeof(ufStringReadonlyCursor) == sizeof(const char*)
// Please keep it this way.
class ufStringCursor : public ufStringReadonlyCursor {
public:
    ufStringCursor(char* buf);
    ufStringCursor(const ufStringCursor& o) :
        ufStringReadonlyCursor(o) {}
    ufStringCursor(ufString& str);

    void TrimSpaces();
    void TrimSpacesFromEnd();
    void TrimSpacesFromStart();

    ufStringCursor& operator +=(int offset) {
        ufStringReadonlyCursor::operator += (offset);
        return *this;
    }
    ufStringCursor& operator ++() {
        ufStringReadonlyCursor::operator += (1);
        return *this;
    }
    ufStringCursor operator ++(int) {
        ufStringCursor beforeChange(*this);
        ++*this;
        return beforeChange;
    }
    char& operator *()  { return *BufToMod(); };
    const char operator *() const { return *Buf(); };
    char& operator [](int index)  { return BufToMod()[index]; };
    const char operator [] (int index) const  { return Buf()[index]; };
    operator char *() { return BufToMod(); };
    operator const char *() const { return Buf(); };

    ufStringCursor &operator=(char* lpszString) {
        ufStringReadonlyCursor::operator=(lpszString);
        return *this;
    }
    ufStringCursor &operator=(ufString &string) {
        ufStringReadonlyCursor::operator=(string.CStrForMod());
        return *this;
    }
protected:
    ufStringCursor() {}
    
private:
    // ufStringCursor may modify a ufString so that ufString has
    // a modifiable buffer. DON'T convert ufString to a const char*!!!
    // instead make it modifiable.
    ufStringCursor(const ufString& str) {/*illegal*/;}
    ufStringCursor &operator=(const ufString &string) {
        /*illegal*/return *this;
    };
    ufStringCursor(const ufTmpBuf& buf) { /*illegal*/ }
};

// use only with string literals. i.e.
// ufStringLiteral a("My literal string");
class ufStringLiteral : public ufString {
public:
    ufStringLiteral(const char *lpszString) {
      int len = -1;
      BOOL isOwner = FALSE;
      Init(lpszString, isOwner, ufSTRING_INIT_DEFAULT);
    }
};

class ufTmpBufCursor : public ufStringCursor {
public:
    ufTmpBufCursor(ufTmpBuf& buf);
    ufTmpBufCursor(ufTmpBuf& buf, int index); // index == -1 :=> end of buf
    ufTmpBufCursor(const ufTmpBufCursor& o);

    void Reallocate(int newSize) {
        int delta = BufToMod() - fTmpBuf->fBuf;
        fTmpBuf->Reallocate(newSize);
        Init(fTmpBuf->fBuf + delta);
    }
    int GetIndex() {
        return BufToMod() - fTmpBuf->fBuf;
    }
    // moves this cursor to point to ufTmpBuf[i] location
    ufTmpBufCursor& MoveToIndex(int i) {
        Init(fTmpBuf->fBuf + i);
        return *this;
    }
    ufTmpBufCursor& MoveToStart() {
        Init(fTmpBuf->fBuf);
        return *this;
    }
    ufTmpBufCursor& MoveToEndOfString();
    ufTmpBufCursor& MoveToEnd() {
        Init(fTmpBuf->fBuf + fTmpBuf->GetSize());
        return *this;
    }
    ufTmpBufCursor& operator +=(int offset) {
        ufStringCursor::operator += (offset);
        return *this;
    }
    ufTmpBufCursor& operator ++() {
        ufStringCursor::operator += (1);
        return *this;
    }
    ufTmpBufCursor operator ++(int) {
        SafeDereferenceChar(1);
        ufTmpBufCursor beforeChange(*this);
        ++*this;
        return beforeChange;
    }
    char& operator *()  {
        return SafeDereferenceCharRef(0);
    };
    char& operator [](int index)  { return SafeDereferenceCharRef(index); };
    operator char *() { return BufToMod(); };
    operator const char *() const { return Buf(); };

    ufTmpBufCursor &operator=(ufTmpBuf &tmpBuf) {
        ufStringCursor::operator=(tmpBuf.fBuf);
        fTmpBuf = &tmpBuf;
        return *this;
    }
private:
    ufTmpBuf * fTmpBuf;

    ufTmpBufCursor(const ufString& str) {/*illegal*/;}

    ufTmpBufCursor &operator=(const ufString &string) {
        /*illegal*/return *this;
    };
    ufTmpBufCursor &operator=(ufString &string) {
        /*illegal*/
        ufStringCursor::operator=(string.CStrForMod());
        return *this;
    }
    ufTmpBufCursor &operator=(char* lpszString) {
        /* illegal. fBuf must point to some legitimate area in
         * ufTmpBuf's buffer. It can't be some arbitrary const char* pointer
         */
        ufStringCursor::operator=(lpszString);
        return *this;
    }
    char& SafeDereferenceCharRef(int offset) {
        int oldCursorIndex = GetIndex();
        int absoluteIndex = oldCursorIndex + offset;
        if (absoluteIndex >= fTmpBuf->GetSize()) {
            Reallocate(absoluteIndex + 256);
            Init(fTmpBuf->fBuf + oldCursorIndex);
        }
        return absoluteIndex >= 0 ? BufToMod()[offset] :
            BufToMod()[0];
    }
    char SafeDereferenceChar(int offset) {
        char c = SafeDereferenceCharRef(offset);
        return c;
    }
};

//
// INLINE FUNCTIONS
//

inline BOOL ufString::IsEmpty() { return fString == 0 || *fString == '\0'; }

inline void ufString::LoseReference() {
    fString = 0;
    fOwnString = 0;
}

inline ufString ufString::Literal() { return ufStringLiteral(fString); }

inline char ufString::operator[](int n) const { return fString[n]; }

inline ufString::operator const char *() const { return fString; }

ufEXTERN_ARCHIVE_OPERATORS(ufString)

inline ufStringReadonlyCursor::ufStringReadonlyCursor(const char* buf) {
    Init(buf);
}

inline ufStringReadonlyCursor::ufStringReadonlyCursor(const ufString& str) {
    Init(str.CStr());
}

inline ufStringReadonlyCursor::ufStringReadonlyCursor(const ufTmpBuf& tmp) {
    Init(tmp.fBuf);
}

inline int ufStringReadonlyCursor::CalculateNewPos(int oldColumn) {
    const char c = *fBuf;
    if (c != '\t' && c != '\n') {
        oldColumn++;
    } else if (c == '\t') {
        oldColumn += 8;
    } else {
        oldColumn = 0;
    }
    return oldColumn;
}

inline void ufStringReadonlyCursor::Init(const char* buf) {
    fBuf = buf;
}

inline char ufStringReadonlyCursor::SkipSpaces() {
    if (fBuf != 0) {
        while (isspace(*fBuf))
            fBuf++;
        return *fBuf;
    }      
    return '\0';
}

inline BOOL ufStringReadonlyCursor::IsEqual(const char c) {
    return fBuf != 0 && *fBuf == c;
}

inline BOOL ufStringReadonlyCursor::IsLower() {
    return fBuf != 0 && islower(*fBuf);
}

inline BOOL ufStringReadonlyCursor::IsUpper() {
    return fBuf != 0 && isupper(*fBuf);
}

inline BOOL ufStringReadonlyCursor::IsEmpty() {
    return fBuf == 0 || *fBuf == '\0';
}

inline BOOL ufStringReadonlyCursor::IsBlankLine() {
    return SkipSpaces() == '\0';
}

inline BOOL ufStringReadonlyCursor::StartsWith(const char*c2) {
    const char*c1 = fBuf;
    while (*c1 == *c2 && *c1 != '\0' && *c2 != '\0') {
        c1++;
        c2++;
    }
    BOOL c1IsDone = *c1 == '\0' || isspace(*c1) || *c1 == '=';
    BOOL c2IsDone = *c2 == '\0' || isspace(*c2) || *c2 == '=';
    return c1IsDone && c2IsDone;
}

inline ufStringCursor::ufStringCursor(char* buf) {
    Init(buf);
}

inline ufStringCursor::ufStringCursor(ufString& str) {
    Init(str.CStrForMod());
}

inline ufTmpBufCursor::ufTmpBufCursor(ufTmpBuf& buf) {
    Init(buf.fBuf);
    fTmpBuf = &buf;
}

// index == -1 :=> end of buf
inline ufTmpBufCursor::ufTmpBufCursor(ufTmpBuf& buf, int index) {
    Init(buf.fBuf + (index >= 0 ? index : buf.GetSize()) );
    fTmpBuf = &buf;
}

inline ufTmpBufCursor::ufTmpBufCursor(const ufTmpBufCursor& o) {
    Init(o.Buf());
    fTmpBuf = o.fTmpBuf;
}


UF_API(char *) ufStrDupLen(const char *lpszString, int *lpnLen);
UF_API(char *) ufStrDup(const char *lpszString);
UF_API(char *) ufStrCat(const char *lpszString, const char *lpszString2);

#endif
