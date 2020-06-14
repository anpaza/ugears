/*
    Date and time manipulation routines

    Public Domain code, source:
    http://stackoverflow.com/questions/1274964/how-to-decompose-unix-time-in-c
*/

#include <useful/datetime.h>

#define YEAR_TO_DAYS(y) ((y)*365 + (y)/4 - (y)/100 + (y)/400)

/*
 * My apologies for all the magic numbers. 367*month/12 is a neat trick to generate
 * the 30/31 day sequence of the calendar. The calculation works with years that
 * start in March until the fixup at the end, which makes things easy because
 * then the leap day falls at the end of a "year".
 */
void ut2dt (uint32_t unixtime, datetime_t *dt)
{
    /* First take out the hour/minutes/seconds - this part is easy. */

    dt->sec = unixtime % 60;
    unixtime /= 60;

    dt->min = unixtime % 60;
    unixtime /= 60;

    dt->hour = unixtime % 24;
    unixtime /= 24;

    /* day 0 was a thursday */
    dt->wday = (unixtime + 4) % 7;

    /* unixtime is now days since 01/01/1970 UTC
     * Rebaseline to the Common Era */

    unixtime += 719499;

    /* Roll forward looking for the year.  This could be done more efficiently
     * but this will do.  We have to start at 1969 because the year we calculate here
     * runs from March - so January and February 1970 will come out as 1969 here.
     */
    unsigned year;
    for (year = 1969; unixtime > YEAR_TO_DAYS (year + 1) + 30; year++)
        ;

    /* OK we have our "year", so subtract off the days accounted for by full years. */
    unixtime -= YEAR_TO_DAYS (year);

    /* unixtime is now number of days we are into the year (remembering that March 1
     * is the first day of the "year" still). */

    /* Roll forward looking for the month.  1 = March through to 12 = February. */
    unsigned mon;
    for (mon = 1; mon < 12 && unixtime > 367 * (mon + 1) / 12; mon++)
        ;

    /* Subtract off the days accounted for by full months */
    unixtime -= 367 * mon / 12;

    /* unixtime is now number of days we are into the month */

    /* Adjust the month/year so that 0 = January, and years start where we
     * usually expect them to. */
    mon++;
    if (mon > 11)
    {
        mon -= 12;
        year++;
    }

    dt->year = year;
    dt->mon = mon;
    dt->day = unixtime;
}

uint32_t dt2ut (datetime_t *dt)
{
    uint32_t unixtime = dt->day;
    unsigned mon = dt->mon;
    unsigned year = dt->year;

    // Convert 0(Jan)-11(Dec) to 1(March)-12(Feb)
    if (mon < 2)
    {
        mon += 11;
        year--;
    }
    else
        mon--;

    /* Add the days accounted for by full months */
    unixtime += 367 * mon / 12;

    /* Add the days accounted for by full years. */
    unixtime += YEAR_TO_DAYS (year);

    /* Rebaseline from Common Era to unixtime (days since 01/01/1970 UTC) */
    unixtime -= 719499;

    /* Convert days to seconds, adding the in-day offset */
    unixtime = ((unixtime * 24 + dt->hour) * 60 + dt->min) * 60 + dt->sec;

    return unixtime;
}

#ifdef _MAIN

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main ()
{
    int i;

    srand (time (NULL));

    for (i = 0; i < 1000000; i++)
    {
        datetime_t d;
        time_t t = rand ();
        ut2dt (t, &d);

        struct tm *tm = gmtime (&t);

        if ((d.year != 1900 + tm->tm_year) ||
            (d.mon != tm->tm_mon) ||
            (d.day != tm->tm_mday) ||
            (d.hour != tm->tm_hour) ||
            (d.min != tm->tm_min) ||
            (d.sec != tm->tm_sec))
        {
            printf ("0) %ld\n", t);
            printf ("1) %d/%d/%d - %d:%d:%d (%d)\n",
                d.year, d.mon + 1, d.day, d.hour, d.min, d.sec, d.wday);
            printf ("2) %d/%d/%d - %d:%d:%d (%d)\n",
                1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_wday);
        }

        uint32_t nt = dt2ut (&d);
        if (t != nt)
            printf ("t = %ld, nt = %ld\n", t, nt);
    }

    return 0;
}

#endif
