/*
 * See Readme, Notice and License files.
 */
#include "ufDef.h"
#include "ufFileNm.h"
#include "ufString.h"

ufDEBUG_FILE

    const char *lpszDirSepString = "\\";

ufFileNm::ufFileNm() : fStartOfBaseName(0) {}

ufFileNm::ufFileNm(const ufFileNm &string)
    : ufString(string), fStartOfBaseName(string.fStartOfBaseName) {
    if (fString == 0) {
        fStartOfBaseName = 0;
    }
}

ufFileNm::ufFileNm(const ufString &string) : ufString(string) {
    RecalculateBaseName();
}

ufFileNm::ufFileNm(const char *lpszString) : ufString(lpszString) {
    RecalculateBaseName();
}

void ufFileNm::RecalculateBaseName() {
    fStartOfBaseName = 0;
    if (fString != 0) {
        for (int i = fLen - 1; i; i--) {
            if (IsDirSep(fString[i])) {
                fStartOfBaseName = i + 1;
                break;
            }
        }
    }
}

ufFileNm &ufFileNm::operator=(const ufFileNm &string) {
    ufString::operator=(string);
    fStartOfBaseName = string.fStartOfBaseName;
    return *this;
}

ufFileNm &ufFileNm::operator=(const ufString &string) {
    ufString::operator=(string);

    RecalculateBaseName();
    return *this;
}

ufFileNm &ufFileNm::operator=(const char *lpszString) {
    ufString::operator=(lpszString);

    RecalculateBaseName();
    return *this;
}

// istream  &operator >> (istream&  in, ufFileNm &) {}
#ifdef USE_MFC

CArchive &operator>>(CArchive &archive, ufFileNm &filename) {
    operator>>(archive, (ufString &)filename);
    filename.RecalculateBaseName();
    return archive;
}

CArchive &operator<<(CArchive &archive, ufFileNm &filename) {
    operator<<(archive, (ufString &)filename);
    return archive;
}

#endif
