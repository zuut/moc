/*
 * See Readme, Notice and License files.
 */
#ifndef _ufDate_h_
#define _ufDate_h_

#include "ufDef.h"

class ufDate {
    ufFRIEND_ARCHIVE_OPERATORS(ufDate)
public:

    ufDate();
    ufDate(int year, int month, int date);
    ufDate(int year, int month, int date,
           int hour, int minute, int sec);

    ufDate(const ufDate &);

    ~ufDate();

    void SetToCurrentTime();

    void FormatDate(char *buf) const;

    void FormatTime(char *buf) const;

    ufCOMPARISON_MEMBER_OPERATORS(const ufDate&, Compare);
#ifdef USE_STL_STREAMS
    ufSTREAM_OUT_MEMBERS(std::ostream &,const ufDate&, WriteTo);
    //ufSTREAM_IN_MEMBERS(std::istream &, ufDate&, ReadFrom);
#endif

    ufDate &operator=(const ufDate &);

    int fYear;
    int fMonth;
    int fDay;
    int fHour;
    int fMinute;
    int fSecond;
};

#ifdef USE_STL_STREAMS
ufSTREAM_INLINE_GLOBAL_OUT(std::ostream&, const ufDate&, WriteTo);
//ufSTREAM_INLINE_GLOBAL_IN(std::istream&, ufDate&, ReadFrom);
#endif

inline ufDate::ufDate()
    : fMonth(1), fDay(1), fYear(1993), fHour(9), fMinute(0),
      fSecond(0) {
}

inline ufDate::ufDate(int year, int month, int day) :
    fYear(year), fMonth(month), fDay(day), fHour(0), fMinute(0), fSecond(0)
{
}

inline ufDate::ufDate(int year, int month, int day,
                      int hour, int min, int sec) :
    fYear(year), fMonth(month), fDay(day),
    fHour(hour), fMinute(min), fSecond(sec)
{
}

ufEXTERN_ARCHIVE_OPERATORS(ufDate)

#endif

