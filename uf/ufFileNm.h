/*
 * See Readme, Notice and License files.
 */
#ifndef _ufFileNfh_
#define _ufFileNfh_

#include "ufDef.h"
#include "ufString.h"

class istream;

class ufFileNm : public ufString {
    //        friend          istream  &operator >> (istream&  in,
    //                                                 ufFileNm &);
    ufFRIEND_ARCHIVE_OPERATORS(ufFileNm) public : ufFileNm();

    ufFileNm(const ufFileNm &string);

    ufFileNm(const ufString &string);

    ufFileNm(const char *lpszString);

    const char *GetBaseName() const;

    int GetBaseNameLength() const;

    void RecalculateBaseName();

    ufFileNm &operator=(const ufFileNm &string);

    ufFileNm &operator=(const ufString &string);

    ufFileNm &operator=(const char *lpszString);

protected:
    int fStartOfBaseName;
};

inline const char *ufFileNm::GetBaseName() const {
    return &fString[fStartOfBaseName];
}

inline int ufFileNm::GetBaseNameLength() const {
    return fLen - fStartOfBaseName;
}

// extern istream &operator >> (istream& in, ufFileNm &);
ufEXTERN_ARCHIVE_OPERATORS(ufFileNm)

    const char cDirSepChar = '\\';
extern const char *lpszDirSepString; //= "\\";
const char cDirsep = '\\';

inline int IsDirSep(char c) { return c == '\\' || c == ':'; }

#endif
