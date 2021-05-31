/*
 * See Readme, Notice and License files.
 */

#ifdef USE_MFC
#include <afxwin.h>
#else

#include <stdio.h>
#include <stdlib.h>

#endif

#include "ufDate.h"
#include "ufDef.h"
#include "ufString.h"

ufDEBUG_FILE

ufDate::ufDate(const ufDate &date)
    : fMonth(date.fMonth), fDay(date.fDay), fYear(date.fYear),
      fHour(date.fHour), fMinute(date.fMinute),
      fSecond(date.fSecond) {
    ufCHECK_MEMORY
}

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
    ufASSERT(buf)
    ufCHECK_MEMORY
    // modified for Y2k
    
#ifdef USE_MFC
    wsprintf(buf, "%d/%d/%4d", fMonth, fDay, fYear);
#else
    sprintf(buf, "%d/%d/%4d", fMonth, fDay, fYear );
#endif
}

void ufDate::FormatTime(char *buf) const {
    ufASSERT(buf)
    ufCHECK_MEMORY

#ifndef USE_MFC
    sprintf(buf, "%d:%02d %s", (fHour > 12 ? fHour - 12 : fHour), fMinute,
            (fHour > 12 ? "PM" : "AM"));
#else
    wsprintf(buf, "%d:%02d %s", (fHour > 12 ? fHour - 12 : fHour), fMinute,
             (fHour > 12 ? "PM" : "AM"));
#endif
}

#define COMPARE(x) if ((cmp = x - date. x ) != 0) { return cmp; }
int ufDate::Compare(const ufDate &date) const {
    int cmp;
    COMPARE(fYear);
    COMPARE(fMonth);
    COMPARE(fDay);
    COMPARE(fHour);
    COMPARE(fMinute);
    COMPARE(fSecond);
    return 0;
}
#undef COMPARE

ufDate &ufDate::operator=(const ufDate &other) {
    ufCHECK_MEMORY

    fYear = other.fYear;
    fMonth = other.fMonth;
    fDay = other.fDay;
    fHour = other.fHour;
    fMinute = other.fMinute;
    fSecond = other.fSecond;

    ufCHECK_MEMORY
    return *this;
}

#ifdef USE_STL_STREAMS
std::ostream & ufDate::WriteTo(std::ostream & out) const {
    ufTmpBuf tmp(200);
    ufTmpBuf tmp2(200);

    FormatDate(tmp.fBuf);
    FormatTime(tmp2.fBuf);
    tmp.Concat(" ");
    tmp.ConcatBuf(tmp2);

    ufStringLiteral dateTime(tmp.fBuf);
    out << dateTime;
    return out;
}

#if 0
std::istream& ufDate::ReadFrom(std::istream& in) {
    ufString dateTime;
    in >> dateTime;
    ufASSERT(0);
    // TBD: parse date time
    return in;
}
#endif
#endif // USE_STL_STREAMS


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

#ifdef ufDateTEST
#include <string.h>

int main(int argc, char *argv[]) {
    ufTmpBufInit();
    std:: cout << "TESTING ufDate " << std::endl;
    int numFailures = 0;
    {
        ufDate date;
        date.fMonth = 6;
        date.fDay = 28;
        date.fYear = 1968;
        date.fHour = 11;
        date.fMinute = 12;
        date.fSecond = 13;
        ufTmpBuf buf(100);
        date.FormatDate(buf.fBuf);
        std::cout << "date " << date << " formatted to '"
                  << buf.fBuf << "'" << std::endl;
        ufVERIFY_RESULT(strcmp("6/28/1968", buf.fBuf)==0, TRUE, numFailures);
        date.FormatTime(buf.fBuf);
        std::cout << "date " << date << " formatted to '"
                  << buf.fBuf << "'" << std::endl;
        ufVERIFY_RESULT(strcmp("11:12 AM", buf.fBuf)==0, TRUE, numFailures);
    }
    {
        ufDate date1(1993, 10, 1 );
        ufDate date2(1993, 10, 2 );
        ufDate date3(1993, 10, 1 );
        ufVERIFY_RESULT(date1 < date2, TRUE, numFailures);
        ufVERIFY_RESULT(date1 <= date2, TRUE, numFailures);
        ufVERIFY_RESULT(date2 > date1, TRUE, numFailures);
        ufVERIFY_RESULT(date2 >= date1, TRUE, numFailures);
        ufVERIFY_RESULT(date1 != date2, TRUE, numFailures);
        ufVERIFY_RESULT(date1 == date3, TRUE, numFailures);
    }

    
    std::cout << "TEST DONE. Total Failures: " << numFailures << "\n";

    ufTmpBufDestroy();
    return (numFailures);
}
#endif
