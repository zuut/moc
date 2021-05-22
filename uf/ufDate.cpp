/*
 * See Readme, Notice and License files.
 */
#include <iostream>

#ifdef USE_MFC
#include <afxwin.h>
#else

#include <stdio.h>
#include <stdlib.h>

#endif

#include "ufDate.h"
#include "ufDef.h"

ufDEBUG_FILE

ufDate::ufDate()
    : fMonth(1), fDay(1), fYear(93), fHour(9), fMinute(0),
      fSecond(0){ufCHECK_MEMORY}

      ufDate::ufDate(const ufDate &date)
    : fMonth(date.fMonth), fDay(date.fDay), fYear(date.fYear),
      fHour(date.fHour), fMinute(date.fMinute),
      fSecond(date.fSecond){ufCHECK_MEMORY}

      ufDate::~ufDate() {
    ufCHECK_MEMORY
}

void ufDate::SetToCurrentTime() {
    ufCHECK_MEMORY

#ifndef USE_MFC
        // this is not implemented
        ufASSERT(0)
#else
        CTime currentTime = CTime::GetCurrentTime();

    fYear = currentTime.GetYear();
    fMonth = currentTime.GetMonth();
    fDay = currentTime.GetDay();
    fHour = currentTime.GetHour();
    fMinute = currentTime.GetMinute();
    fSecond = currentTime.GetSecond();
#endif
            ufCHECK_MEMORY
}

void ufDate::FormatDate(char *buf) const {
    ufASSERT(buf) ufCHECK_MEMORY
#ifdef USE_MFC
        wsprintf(buf, "%d/%d/%2d", fMonth, fDay, (fYear % 100));
#else
        sprintf(buf, "%d/%d/%2d", fMonth, fDay, (fYear % 100));
#endif
}

void ufDate::FormatTime(char *buf) const {
    ufASSERT(buf) ufCHECK_MEMORY

#ifndef USE_MFC
        sprintf(buf, "%d:%02d:%s", (fHour > 12 ? fHour - 12 : fHour), fMinute,
                (fHour > 12 ? "PM" : "AM"));
#else
        wsprintf(buf, "%d:%02d:%s", (fHour > 12 ? fHour - 12 : fHour), fMinute,
                 (fHour > 12 ? "PM" : "AM"));
#endif
}

BOOL ufDate::operator>(const ufDate &other) const {
    ufCHECK_MEMORY

        if (fYear > other.fYear) {
        return TRUE;
    }
    else if (fYear < other.fYear) {
        return FALSE;
    }
    else if (fMonth > other.fMonth) {
        return TRUE;
    }
    else if (fMonth < other.fMonth) {
        return FALSE;
    }
    else if (fDay > other.fDay) {
        return TRUE;
    }
    else if (fDay < other.fDay) {
        return FALSE;
    }
    else if (fHour > other.fHour) {
        return TRUE;
    }
    else if (fHour < other.fHour) {
        return FALSE;
    }
    else if (fMinute > other.fMinute) {
        return TRUE;
    }
    else if (fMinute < other.fMinute) {
        return FALSE;
    }
    else if (fSecond > other.fSecond) {
        return TRUE;
    }

    return FALSE;
}

BOOL ufDate::operator<(const ufDate &other) const {
    ufCHECK_MEMORY

        if (fYear < other.fYear) {
        return TRUE;
    }
    else if (fYear > other.fYear) {
        return FALSE;
    }
    else if (fMonth < other.fMonth) {
        return TRUE;
    }
    else if (fMonth > other.fMonth) {
        return FALSE;
    }
    else if (fDay < other.fDay) {
        return TRUE;
    }
    else if (fDay > other.fDay) {
        return FALSE;
    }
    else if (fHour < other.fHour) {
        return TRUE;
    }
    else if (fHour > other.fHour) {
        return FALSE;
    }
    else if (fMinute < other.fMinute) {
        return TRUE;
    }
    else if (fMinute > other.fMinute) {
        return FALSE;
    }
    else if (fSecond < other.fSecond) {
        return TRUE;
    }

    return FALSE;
}

BOOL ufDate::operator==(const ufDate &other) const {
    ufCHECK_MEMORY

            return fYear == other.fYear &&
        fMonth == other.fMonth && fDay == other.fDay && fHour == other.fHour &&
        fMinute == other.fMinute && fSecond == other.fSecond;
}

ufDate &ufDate::operator=(const ufDate &other) {
    ufCHECK_MEMORY

        fYear = other.fYear;
    fMonth = other.fMonth;
    fDay = other.fDay;
    fHour = other.fHour;
    fMinute = other.fMinute;
    fSecond = other.fSecond;

    ufCHECK_MEMORY return *this;
}

std::ostream &operator<<(std::ostream &out, const ufDate &date) {
    ufTmpBuf tmp(200);

    date.FormatDate(tmp.fBuf);
    out << tmp.fBuf << " ";
    date.FormatTime(tmp.fBuf);
    out << tmp.fBuf;
    return out;
}

// istream &operator >> (istream& in, ufDate &);

#ifdef USE_MFC
CArchive &operator<<(CArchive &out, const ufDate &date) {
    out << (WORD &)date.fYear;
    out << (WORD &)date.fMonth;
    out << (WORD &)date.fDay;
    out << (WORD &)date.fHour;
    out << (WORD &)date.fMinute;
    out << (WORD &)date.fSecond;
    return out;
}

CArchive &operator>>(CArchive &in, ufDate &date) {
    in >> (WORD &)date.fYear;
    in >> (WORD &)date.fMonth;
    in >> (WORD &)date.fDay;
    in >> (WORD &)date.fHour;
    in >> (WORD &)date.fMinute;
    in >> (WORD &)date.fSecond;
    return in;
}

#endif
