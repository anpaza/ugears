#include "hw.h"
#include "usefun.h"

// size is expected a power of two
char test [256], test2 [256];
unsigned r, tno;

void check_area (int tidx, char *data, unsigned size, char val)
{
    for (unsigned i = 0; i < size; i++)
        if (data [i] != val)
            printf ("test %u.%d, rnd 0x%08x: 0x%x + %d contains '%02x', expected '%02x'\r\n",
                tno, tidx, r, data, i, data [i], val);
}

void cmp_area (int tidx, char *data, char *cmp, unsigned size)
{
    for (unsigned i = 0; i < size; i++)
        if (data [i] != cmp [i])
            printf ("test %u.%d, rnd 0x%08x: 0x%x + %d contains '%02x', expected '%02x'\r\n",
                tno, tidx, r, data, i, data [i], cmp [i]);
}

int main (void)
{
    usart1_init ();
    puts ("libugears test started");

    srand (0xdeadbaba);

    puts ("Running tests at high speed, will barf if something goes wrong");
    for (tno = 0; ; tno++)
    {
        memset (test, 0xEA, sizeof (test));
        check_area (1, test, sizeof (test), 0xEA);

        r = rand ();
        unsigned d = r & 127;
        unsigned l = (r >> 7) & 127;
        uint8_t b = r >> 14;

        memset (test + d, b, l);
        check_area (2, test, d, 0xEA);
        check_area (2, test + d, l, b);
        check_area (2, test + d + l, sizeof (test) - (l + d), 0xEA);

        memset (test2, 0xD5, sizeof (test2));
        check_area (3, test2, sizeof (test2), 0xD5);

        r = rand ();
        d = r & (sizeof (test2) - 1);
        l = (r >> 8) & (sizeof (test) - 1);
        b = (r >> 16) & 127;
        if (b > sizeof (test2) - d)
            b = sizeof (test2) - d;
        if (b > sizeof (test) - l)
            b = sizeof (test) - l;

        memcpy (test2 + d, test + l, b);

        check_area (4, test2, d, 0xD5);
        cmp_area (4, test2 + d, test + l, b);
        check_area (4, test2 + d + b, sizeof (test2) - (d + b), 0xD5);
    }
}
