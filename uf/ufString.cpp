/*
 * See Readme, Notice and License files.
 */
#include "ufDef.h"
#include "ufString.h"
#include <iostream>
#include <string.h>

ufDEBUG_FILE
#define MAX_STRING_LENGTH 64000

const char QUOTE = '\"';
const char BACKSLASH = '\\';

static ufAllocator* STANDARD_ALLOCATOR =
    ufAllocator::CreateStandardAllocator();

const ufStealable MAKE_OWNER;

ufString::ufString() {
    const char* buf = 0;
    BOOL isOwner = FALSE;
    Init(buf, isOwner, ufSTRING_INIT_DEFAULT);
}

ufString::ufString(const ufStealable& makeOwner, ufString &string) {
    Init(string.fString, string.fOwnString, ufSTRING_INIT_STEAL);
}

ufString::ufString(const ufString &string) {
    ufString& s = (ufString&)string;
    Init(s.fString, s.fOwnString, ufSTRING_INIT_DEFAULT);
}

ufString::ufString(const char *lpszString) {
    BOOL isOwner = FALSE;
    Init(lpszString, isOwner, ufSTRING_INIT_FORCE_DUPLICATE);
}

ufString::~ufString() { MakeEmpty(); }

void ufString::Init(const char*& lpszString,
                    BOOL& ownString, int action) {
    if (action == ufSTRING_INIT_STEAL) {
        fString = lpszString;
        fOwnString = ownString;
        if (ownString) {
            lpszString = 0;
            ownString = 0;
        }
    } else if (lpszString && action == ufSTRING_INIT_FORCE_DUPLICATE) {
        fString = ufStrDup(lpszString);
        fOwnString = 1;
    } else if (lpszString && ownString) {
        // since we can't steal the string and 
        // the caller owns the string, we need our own copy
        fString = ufStrDup(lpszString);
        fOwnString = 1;
    } else if (lpszString) {
        fString = lpszString;
        fOwnString = 0;
    } else {
        fString = 0;
        fOwnString = 0;
    }
}

void ufString::TakeOwnership(char*& bufToOwn) {
    const char* b = bufToOwn;
    BOOL isOwner = TRUE;
    bufToOwn = 0;
    MakeEmpty();
    Init(b, isOwner, ufSTRING_INIT_STEAL);
}

void ufString::MakeOwner(char* bufToOwn) {
    TakeOwnership(bufToOwn);
}

void ufString::TakeOwnership(ufTmpBuf& bufToTakeCharArray) {
    char* bufToOwn;
    bufToTakeCharArray.ReleaseBuffer(bufToOwn);

    TakeOwnership(bufToOwn);
}

void ufString::MakeEmpty() {
    if (fOwnString) {
        delete[] fString;
    }
    fString = "";
    fOwnString = 0;
}

void ufString::MakeStringLiteral(const char* stringLiteral) {
    MakeEmpty();
    BOOL isOwner = FALSE;
    Init( stringLiteral, isOwner, ufSTRING_INIT_DEFAULT);
}

int ufString::GetLength() const {
    int len = fString != 0 ? strlen(fString) : 0;
    return len;
}

ufString &ufString::operator=(const ufString &string) {
    if (&string == this || string.fString == fString) {
        return *this;
    }

    MakeEmpty();
    ufString& s = (ufString&) string;
    Init( s.fString, s.fOwnString, ufSTRING_INIT_DEFAULT);

    return *this;
}

ufString &ufString::operator=(const char *lpszString) {
    if (lpszString == fString) {
        return *this;
    }

    MakeEmpty();
    BOOL isOwner = FALSE;
    Init( lpszString, isOwner, ufSTRING_INIT_FORCE_DUPLICATE);

    return *this;
}

// will convert fString to a writable string if needed.
char * ufString::CStrForMod() {
    if (fString != 0 && !fOwnString) {
        const char* newBuf = ufStrDup(fString);
        BOOL isOwner = TRUE;
        MakeEmpty();
        Init(newBuf, isOwner, ufSTRING_INIT_STEAL);
    }
    return (char*) CStr();
}

int ufString::Compare(const ufString &string) const {
    return Compare(string.fString);
}

int ufString::Compare(const char* lpszString) const {
    const char* a = fString != 0 ? fString : "";
    const char* b = lpszString != 0 ? lpszString : "";
    if (((void*)(fString)) == ((void*)(lpszString))
        || (*a == '\0' && *b == '\0')) {
        return 0;
    }

    int res = strcmp(a, b);
    return res;
}

#ifdef USE_STL_STREAMS
std::ostream & ufString::WriteTo(std::ostream & out) const {
    ufStringReadonlyCursor c(fString);
    out << QUOTE;
    for (;!c.IsEmpty(); c++) {
        if (*c == QUOTE) {
            out << BACKSLASH;
        }
        out << *c;
    }
    out << QUOTE;
    return out;
}

std::istream& ufString::ReadFrom(std::istream& in) {
    char c1;
    in >> c1;
    if ( c1 == QUOTE ) {
        ufTmpBuf buf(100);
        ufTmpBufCursor b(buf);
        int c;
        while ((c=in.get()) != EOF) {
            if (c == QUOTE) {
                break;
            }
            if (c == BACKSLASH) {
                c = in.get();
                if (c == EOF) {
                    return in;
                }
            }
            *b++ = c;
        }
        *b = '\0';
        fString = buf.fBuf;
    }
    return in;
}
#endif // USE_STL_STREAMS

#ifdef USE_MFC
CArchive &operator<<(CArchive &out, const ufString &string) {
    CString s = string.fString;
    out << s;
    return out;
}

CArchive &operator>>(CArchive &in, ufString &string) {
    CString s;
    in >> s;
    string = (const char *)s;
    return in;
}
#endif

UF_API(char *)
ufStrDupLen(const char *lpszString, int *lpnLenReturn) {
    ufASSERT(lpnLenReturn != 0)
    if (lpszString == 0) {
        *lpnLenReturn = 0;
        ufCHECK_MEMORY
        return 0;
    }
    *lpnLenReturn = strlen(lpszString);
    char *lpszS = NEW char[*lpnLenReturn + 1];

    ufASSERT(lpszS != 0)

    if (!lpszS) {
        return 0;
    }

    strcpy(lpszS, lpszString);

    ufCHECK_MEMORY
    return lpszS;
}

UF_API(char *)
ufStrDup(const char *lpszString) { return ufStrCat(lpszString, 0); }

UF_API(char *)
ufStrCat(const char *lpszString, const char *lpszString2) {
    if (lpszString == 0) {
        ufCHECK_MEMORY
        return 0;
    }
    int nLenReturn = strlen(lpszString);
    if (lpszString2 != 0) {
        nLenReturn += strlen(lpszString2);
    }
    if (nLenReturn > MAX_STRING_LENGTH) {
        nLenReturn = MAX_STRING_LENGTH;
    }
    char *lpszS = NEW char[nLenReturn + 1];

    ufASSERT(lpszS != 0)

    if (!lpszS) {
        return 0;
    }

    strncpy(lpszS, lpszString, nLenReturn);
    if (lpszString2 != 0) {
        strncat(lpszS, lpszString2, nLenReturn);
    }

    lpszS[nLenReturn] = '\0';

    ufCHECK_MEMORY
    return lpszS;
}

BOOL ufStringReadonlyCursor::SkipTo(char c) {
    if (fBuf) {
        while (*fBuf != '\0') {
            if (*fBuf++ == c) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL ufStringReadonlyCursor::Contains(char c) {
    const char* curBuf = fBuf;
    BOOL result = SkipTo(c);
    fBuf = curBuf;
    return result;
}
BOOL ufStringReadonlyCursor::IsEqual(const char*c) {
    return !strcmp(fBuf, c);
}

BOOL ufStringReadonlyCursor::IsEqual(const char*c, int n) {
    return !strncmp(fBuf, c, n);
}

char ufStringReadonlyCursor::SkipSpacesAnd(
                               char c1, char c2, char c3, char c4) {
    if (fBuf == 0) {
        return '\0';
    }
    while (isspace(*fBuf) || *fBuf==c1 || *fBuf==c2
           || *fBuf==c3 || *fBuf==c4) {
        fBuf++;
    }
    return *fBuf;
}

// copies the a token from the head of the cursor and advances the
// cursor to the next token.
BOOL ufStringReadonlyCursor::ParseNextToken(ufTmpBuf &tokenBuf) {
    if (fBuf == 0 || *fBuf == '\0') {
        if (tokenBuf.GetSize() > 0) {
            tokenBuf[0] = '\0';
        }
        return FALSE;
    }

    SkipSpaces();
    int len = tokenBuf.GetSize();
    int   j = 0;
    while (*fBuf != '\0' && !isspace(*fBuf)) {
        tokenBuf[j++] = *fBuf++;
    };
    tokenBuf[j] = '\0';
    SkipSpaces();
    return TRUE;
}

void ufStringCursor::TrimSpaces() {
    TrimSpacesFromStart();
    TrimSpacesFromEnd();
}

void ufStringCursor::TrimSpacesFromStart() {
    char* buf = BufToMod();
    if (buf != 0) {
        int len = strlen(buf);
        char* s = buf;
        char* e = buf + len;
        while (isspace(*s) && s !=e)
            s++;
        while (s != e) {
            *buf++ = *s++;
        }
        *buf = '\0';
    }
}

void ufStringCursor::TrimSpacesFromEnd() {
    char* buf = BufToMod();
    if (buf != 0) {
        int len = strlen(buf);
        char* e = buf + len - 1;
        while (isspace(*e) && e != buf)
            e--;
        e[1] = '\0';
    }
}

ufTmpBufCursor& ufTmpBufCursor::MoveToEndOfString() {
    int len = strlen(fTmpBuf->fBuf);
    Init(fTmpBuf->fBuf + len);
    return *this;
}

#ifdef ufStringTEST
#include <sstream>

struct ufStringTester {
  static const char* getString(ufString& o) { return o.fString; }
  static BOOL& getOwnString(ufString& o) { return o.fOwnString; }
};


int main(int argc, char *argv[]) {
    ufTmpBufInit();
    std:: cout << "TESTING ufString " << std::endl;
    int numFailures = 0;

    const char* orig = "beta";
    const char* less = "alpha";
    const char* empty = "";
    ufString s_less(less);
    ufString s_empty(empty);
    {
        ufTEST("ufString(orig)");
        ufString s_orig(orig);
        const char* b = s_orig;

        ufVERIFY_RESULT(ufStringTester::getOwnString(s_orig), TRUE, numFailures);

        ufVERIFY_RESULT(b == orig, FALSE, numFailures);
        ufVERIFY_RESULT(strcmp(less, s_orig) < 0, TRUE, numFailures);
        ufVERIFY_RESULT(less  < s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(less <= s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(less == s_orig, FALSE, numFailures);
        ufVERIFY_RESULT(less != s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(less  > s_orig, FALSE, numFailures);
        ufVERIFY_RESULT(less >= s_orig, FALSE, numFailures);

        ufVERIFY_RESULT(s_orig  > less, TRUE, numFailures);
        ufVERIFY_RESULT(s_orig >= less, TRUE, numFailures);
        ufVERIFY_RESULT(s_orig == less, FALSE, numFailures);
        ufVERIFY_RESULT(s_orig != less, TRUE, numFailures);
        ufVERIFY_RESULT(s_orig  < less, FALSE, numFailures);
        ufVERIFY_RESULT(s_orig <= less, FALSE, numFailures);

        ufVERIFY_RESULT(s_less  < s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(s_less <= s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(s_less == s_orig, FALSE, numFailures);
        ufVERIFY_RESULT(s_less != s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(s_less  > s_orig, FALSE, numFailures);
        ufVERIFY_RESULT(s_less >= s_orig, FALSE, numFailures);

        ufVERIFY_RESULT(orig  > s_less, TRUE, numFailures);
        ufVERIFY_RESULT(orig >= s_less, TRUE, numFailures);
        ufVERIFY_RESULT(orig == s_less, FALSE, numFailures);
        ufVERIFY_RESULT(orig != s_less, TRUE, numFailures);
        ufVERIFY_RESULT(orig  < s_less, FALSE, numFailures);
        ufVERIFY_RESULT(orig <= s_less, FALSE, numFailures);

        ufVERIFY_RESULT(orig    != empty, TRUE, numFailures);
        ufVERIFY_RESULT(empty   != orig, TRUE, numFailures);
        ufVERIFY_RESULT(orig    != s_empty, TRUE, numFailures);
        ufVERIFY_RESULT(s_empty != orig, TRUE, numFailures);

        ufVERIFY_RESULT(orig    > empty, TRUE, numFailures);
        ufVERIFY_RESULT(empty   < orig, TRUE, numFailures);
        ufVERIFY_RESULT(orig    > s_empty, TRUE, numFailures);
        ufVERIFY_RESULT(s_empty < orig, TRUE, numFailures);

        ufVERIFY_RESULT(0      != s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(0       < s_orig, TRUE, numFailures);
        ufVERIFY_RESULT(s_orig != 0, TRUE, numFailures);
        ufVERIFY_RESULT(s_orig  > 0 , TRUE, numFailures);
    }
    {
        ufTEST("TrimSpaces");
        #define PHRASE "a string to display"
        const char* noLeadingOrTrailingSpaces = PHRASE ;
        const char* leadingSpaces = "  " PHRASE ;
        const char* trailingSpaces = PHRASE "  " ;
        const char* leadingAndTrailingSpaces = "  "  PHRASE "  ";
        ufString s;
        s = leadingAndTrailingSpaces;
        const char* sPreTrim = s;
        ufVERIFY_RESULT(s[0] == ' ', TRUE, numFailures);
        ufVERIFY_RESULT(s.GetLength() == strlen(s), TRUE, numFailures);
        ufVERIFY_RESULT(s[s.GetLength() - 1] == ' ', TRUE, numFailures);
        ufVERIFY_RESULT(s == leadingAndTrailingSpaces, TRUE, numFailures);

        ufStringCursor c(s);
        c.TrimSpacesFromStart();
        std::cout << "string.trim start '" << s << "'" << std::endl;
        std::cout << "trailingSpaces '" << trailingSpaces << "'" << std::endl;
        ufVERIFY_RESULT(s[0] != ' ', TRUE, numFailures);
        ufVERIFY_RESULT(s[s.GetLength() - 1] == ' ', TRUE, numFailures);
        ufVERIFY_RESULT(s == trailingSpaces, TRUE, numFailures);

        c.TrimSpacesFromEnd();
        std::cout << "string.trim end '" << s << "'" << std::endl;
        const char* sPostTrim = s;
        ufVERIFY_RESULT(s[0] != ' ', TRUE, numFailures);
        ufVERIFY_RESULT(s[s.GetLength() - 1] != ' ', TRUE, numFailures);
        ufVERIFY_RESULT(sPreTrim == sPostTrim, TRUE, numFailures);
        ufVERIFY_RESULT(s == noLeadingOrTrailingSpaces, TRUE, numFailures);
    }
    {
        ufTEST("ufStringLiteral(orig)");
        ufStringLiteral a(orig);
        const char* b = a;

        ufVERIFY_RESULT(ufStringTester::getOwnString(a), FALSE, numFailures);
        ufVERIFY_RESULT(b == orig, TRUE, numFailures);

        ufString c;
        ufTEST_STMT(c.MakeStringLiteral(a));
        ufVERIFY_RESULT(ufStringTester::getOwnString(c), FALSE, numFailures);
        ufVERIFY_RESULT(c == a, TRUE, numFailures);
        ufVERIFY_RESULT(c.CStr() == a.CStr(), TRUE, numFailures);
    }
    {
        ufTEST("ufStringLiteral(orig)");
        ufStringLiteral a(orig);
        const char* b = a;
        ufVERIFY_RESULT(ufStringTester::getOwnString(a), FALSE, numFailures);
        ufVERIFY_RESULT(b == orig, TRUE, numFailures);
    }
    {
        ufTEST("TakeOwnership()");
        ufTmpBuf buf(1000);
        const char* bufPtr = buf.fBuf;
        ufString a;
        ufTEST_STMT(a.TakeOwnership(buf));
        ufVERIFY_RESULT(ufStringTester::getOwnString(a), TRUE, numFailures);
        ufVERIFY_RESULT(ufStringTester::getString(a) == bufPtr, TRUE, numFailures);
        ufVERIFY_RESULT(buf.fBuf == 0, TRUE, numFailures);
    }
    {
        ufTEST("TakeOwnership()");
        char* buf = ufStrDup("A happy string");
        char* bufPtr = buf;
        ufString a;
        ufTEST_STMT(a.TakeOwnership(buf));
        ufVERIFY_RESULT(buf == 0, TRUE, numFailures);
        ufVERIFY_RESULT(ufStringTester::getOwnString(a), TRUE, numFailures);
        ufVERIFY_RESULT(ufStringTester::getString(a) == bufPtr, TRUE, numFailures);
    }

    {
        ufTEST("ufTmpBufCursor::Reallocate");
        ufTEST_STMT( ufTmpBuf buf(4) );
        ufTmpBufCursor b(buf);

        std::cout << "ufTmpBuf 0x" << ((void*)buf.fBuf)
                  << " size is " << buf.GetSize() << std::endl;

        ufVERIFY_RESULT( buf.GetSize() >= 4, TRUE, numFailures);

        b.MoveToIndex(2);
        ufVERIFY_RESULT( b.GetIndex() == 2, TRUE, numFailures);
        
        b.Reallocate(buf.GetSize() + 100);
        ufVERIFY_RESULT( b.GetIndex() == 2, TRUE, numFailures);
    }
    {
        ufTEST("ufTmpBufCursor");
        ufTEST_STMT( ufTmpBuf buf(4) );
        ufTmpBufCursor b(buf);

        std::cout << "ufTmpBuf 0x" << ((void*)buf.fBuf)
                  << " size is " << buf.GetSize() << std::endl;

        ufVERIFY_RESULT( buf.GetSize() >= 4, TRUE, numFailures);

        b.MoveToStart();
        ufVERIFY_RESULT(b.GetIndex() == 0, TRUE, numFailures);
        *b = 'A';
        b[1] = 'A';
        ufVERIFY_RESULT(buf[0] == 'A', TRUE, numFailures);
        ufVERIFY_RESULT(buf[1] == 'A', TRUE, numFailures);
        *b++ = 'B';
        ufVERIFY_RESULT(b.GetIndex() == 1, TRUE, numFailures);

        ufVERIFY_RESULT(buf[0] == 'B', TRUE, numFailures);
        ufVERIFY_RESULT(buf[1] == 'A', TRUE, numFailures);
        ufVERIFY_RESULT(*b == 'A', TRUE, numFailures);
        ufVERIFY_RESULT(b == buf.fBuf + 1, TRUE, numFailures);

        b.MoveToStart();
        ufVERIFY_RESULT(b.GetIndex() == 0, TRUE, numFailures);

        *b = 'A';
        b[1] = 'A';
        ufVERIFY_RESULT(buf[0] == 'A', TRUE, numFailures);
        ufVERIFY_RESULT(buf[1] == 'A', TRUE, numFailures);
        *++b = 'B';
        ufVERIFY_RESULT(b.GetIndex() == 1, TRUE, numFailures);

        ufVERIFY_RESULT(buf[0] == 'A', TRUE, numFailures);
        ufVERIFY_RESULT(buf[1] == 'B', TRUE, numFailures);
        ufVERIFY_RESULT(*b == 'B', TRUE, numFailures);
        ufVERIFY_RESULT(b == buf.fBuf + 1, TRUE, numFailures);

        b.MoveToStart();
        ufVERIFY_RESULT(b.GetIndex() == 0, TRUE, numFailures);
        memset(buf.fBuf, ' ', buf.GetSize() - 1);
        buf[buf.GetSize() - 1]='\0';


        b.MoveToIndex(buf.GetSize() - 1);
        ufVERIFY_RESULT(b.GetIndex() == buf.GetSize() - 1, TRUE, numFailures);
        ufVERIFY_RESULT(*b == '\0', TRUE, numFailures);

        int origSize = buf.GetSize();
        int startingIndex = b.GetIndex();
        {
            char* start = b;
            ufVERIFY_RESULT( start == buf.fBuf + buf.GetSize() - 1, TRUE, numFailures);
            ufVERIFY_RESULT( start == b.Buf(), TRUE, numFailures);
        }
        std::cout << "b is now " << ((void*)b.Buf()) <<  std::endl;
        
        const char* longStr = "a very long string indeed";
        const char* s = longStr;
        std::cout << "b.GetIndex() = " << b.GetIndex() << std::endl;
        while (*s != '\0') {
            *b++ = *s++;
        }
        *b = '\0';
        std::cout << "copied '" << (buf.fBuf + startingIndex) << "'" << std::endl;
        std::cout << "b is now " << ((void*)b.Buf()) <<  std::endl;
        b.MoveToIndex(startingIndex);
        std::cout << "cursor pointing to '" << b << "'" << std::endl;

        ufVERIFY_RESULT(strcmp(b, longStr) == 0, TRUE, numFailures);
        std::cout << "ufTmpBuf 0x" << ((void*)buf.fBuf)
                  << " size is " << buf.GetSize() << std::endl;
        ufVERIFY_RESULT(buf.GetSize() > origSize, TRUE, numFailures);

        b.MoveToStart();
        ufVERIFY_RESULT(b == buf.fBuf, TRUE, numFailures);

        b.MoveToEnd();
        ufVERIFY_RESULT(b == buf.fBuf + buf.GetSize(), TRUE, numFailures);

        b.MoveToIndex(3);
        ufVERIFY_RESULT(b == buf.fBuf + 3, TRUE, numFailures);
    }

#ifdef USE_STL_STREAMS
    {
        std::stringstream ss;
        const char* LITERAL ="my own string";
        const int MYCONST = 1;
        ufString a(LITERAL);
        ufString b;
        
        std::cout << "a=" << a << std::endl;
        ss << a << MYCONST;

        int c;
        ss >> b >> c;
        std::cout << "Read b=" << b << std::endl;
        
        ufVERIFY_RESULT(a == LITERAL, TRUE, numFailures);
        ufVERIFY_RESULT(b == LITERAL, TRUE, numFailures);
        ufVERIFY_RESULT(a == b, TRUE, numFailures);
        ufVERIFY_RESULT(c == MYCONST, TRUE, numFailures);
    }
#endif
    
    std::cout << "TEST DONE. Total Failures: " << numFailures << "\n";

    ufTmpBufDestroy();
    return (numFailures);
}

#endif
