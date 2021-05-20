/*
 * See Readme, Notice and License files.
 */
#ifndef _ufString_h
#define _ufString_h

#include "generic.h"
#include "ufDef.h"

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

class ostream;

class istream;

class ufString {
    friend std::ostream &operator<<(std::ostream &out, const ufString &);
    //      friend          std::istream &operator >> (std::istream& in,
    //                                                 ufString &);
    ufFRIEND_ARCHIVE_OPERATORS(ufString) public : ufString();

    ufString(const ufString &string);

    ufString(const char *lpszString);

    ~ufString();

    int GetLength() const;

    void MakeEmpty();

    void LoseReference();

    char operator[](int n) const;

    ufString &operator=(const ufString &string);

    ufString &operator=(const char *lpszString);

    int operator==(const ufString &string) const;

    int operator==(const char *lpszString) const;

    int operator!=(const ufString &string) const;

    int operator!=(const char *lpszString) const;

    operator const char *();

protected:
    int fLen;
    char *fString;
};

inline int ufString::GetLength() const { return fLen; }

inline void ufString::LoseReference() {
    fString = 0;
    fLen = 0;
}

inline char ufString::operator[](int n) const { return fString[n]; }

inline ufString::operator const char *() { return fString; }

inline int ufString::operator!=(const ufString &string) const {
    return !ufString::operator==(string);
}

inline int ufString::operator!=(const char *lpszString) const {
    return !ufString::operator==(lpszString);
}

inline int operator==(const char *lpszString, const ufString &string) {
    return string.operator==(lpszString);
}

inline int operator!=(const char *lpszString, const ufString &string) {
    return !string.operator==(lpszString);
}

extern std::ostream &operator<<(std::ostream &out, const ufString &);
// extern istream &operator >> (istream& in, ufString &);
ufEXTERN_ARCHIVE_OPERATORS(ufString)

    UF_API(char *) ufStrDupLen(const char *lpszString, int *lpnLen);
UF_API(char *) ufStrDup(const char *lpszString);
UF_API(char *) ufStrCat(const char *lpszString, const char *lpszString2);

#endif
