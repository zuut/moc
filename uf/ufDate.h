/*
 * See Readme, Notice and License files.
 */
#ifndef _ufDate_h_
#define _ufDate_h_

#include "ufDef.h"

class ostream;

class istream;

class ufDate {
    friend ostream &operator<<(ostream &out, const ufDate &);
    //      friend          istream &operator >> (istream& in,
    //                                            ufDate &);
    ufFRIEND_ARCHIVE_OPERATORS(ufDate) public : ufDate();

    ufDate(const ufDate &);

    ~ufDate();

    void SetToCurrentTime();

    void FormatDate(char *buf) const;

    void FormatTime(char *buf) const;

    BOOL operator>(const ufDate &) const;

    BOOL operator<(const ufDate &) const;

    BOOL operator==(const ufDate &) const;

    BOOL operator!=(const ufDate &) const;

    ufDate &operator=(const ufDate &);

    int fMonth;
    int fDay;
    int fYear;
    int fHour;
    int fMinute;
    int fSecond;
};

extern std::ostream &operator<<(std::ostream &out, const ufDate &);
// extern        istream &operator >> (istream& in, ufDate &);
ufEXTERN_ARCHIVE_OPERATORS(ufDate)

    inline BOOL ufDate::operator!=(const ufDate &other) const {
    return !(operator==(other));
}

#endif
