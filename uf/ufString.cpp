/*
 * See Readme, Notice and License files.
 */
#include "ufDef.h"
#include "ufString.h"
#include <iostream>
#include <string.h>

ufDEBUG_FILE
#define MAX_STRING_LENGTH 64000

ufString::ufString()
    : fLen(0), fString(0) {
}

ufString::ufString(const ufString &string) {
    if (string.fString) {
        fString = ufStrDupLen(string.fString, &fLen);
    } else {
        fString = 0;
        fLen = 0;
    }
}

ufString::ufString(const char *lpszString) {
    if (lpszString) {
        fString = ufStrDupLen(lpszString, &fLen);
    } else {
        fString = 0;
        fLen = 0;
    }
}

ufString::~ufString() { MakeEmpty(); }

void ufString::MakeEmpty() {
    delete[] fString;
    fString = 0;
    fLen = 0;
}

ufString &ufString::operator=(const ufString &string) {
    if (&string == this || string.fString == fString) {
        return *this;
    }

    delete[] fString;
    if (string.fString) {
        fString = ufStrDupLen(string.fString, &fLen);
    } else {
        fString = 0;
        fLen = 0;
    }

    return *this;
}

ufString &ufString::operator=(const char *lpszString) {
    if (lpszString == fString) {
        return *this;
    }

    delete[] fString;
    if (lpszString) {
        fString = ufStrDupLen(lpszString, &fLen);
    } else {
        fString = 0;
        fLen = 0;
    }

    return *this;
}

int ufString::operator==(const ufString &string) const {
    if (string.fString == fString || (string.fLen == 0 && fLen == 0)) {
        return 1;
    }

    if (fLen == 0 || string.fLen == 0) {
        return 0;
    }

    return !strcmp(fString, string.fString);
}

int ufString::operator==(const char *lpszString) const {
    if (lpszString == fString || (lpszString == 0 && fLen == 0)) {
        return 1;
    }

    if (fLen == 0 || lpszString == 0) {
        return 0;
    }

    return !strcmp(fString, lpszString);
}

std::ostream &operator<<(std::ostream &out, const ufString &string) {
    out << string.fLen;
    out << string.fString;
    return out;
}

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
        ufCHECK_MEMORY return 0;
    }
    *lpnLenReturn = strlen(lpszString);
    char *lpszS = NEW char[*lpnLenReturn + 1];

    ufASSERT(lpszS != 0)

        if (!lpszS) {
        return 0;
    }

    strcpy(lpszS, lpszString);

    ufCHECK_MEMORY return lpszS;
}

UF_API(char *)
ufStrDup(const char *lpszString) { return ufStrCat(lpszString, 0); }

UF_API(char *)
ufStrCat(const char *lpszString, const char *lpszString2) {
    if (lpszString == 0) {
        ufCHECK_MEMORY return 0;
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

    strcpy(lpszS, lpszString);
    if (lpszString2 != 0) {
        strcat(lpszS, lpszString2);
    }

    ufCHECK_MEMORY return lpszS;
}
