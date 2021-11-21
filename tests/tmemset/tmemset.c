#define USING_LIBC

#include <useful/usefun.h>
#include <stdio.h>
#include <string.h>

int main ()
{
    xs_rng_t rng;
    xs_init (rng, 0xaabbccdd);

    uint8_t haystack [256], haystack_copy [256];
    for (unsigned alot = 0; alot < 1000000; alot++)
    {
        for (unsigned i = 0; i < ARRAY_LEN (haystack); i++)
            haystack [i] = xs_rand (rng);
        memcpy (haystack_copy, haystack, sizeof (haystack_copy));

        char needle = xs_rand (rng);
        unsigned size = xs_rand (rng) & 255;
        unsigned offs = xs_rand (rng) % (ARRAY_LEN (haystack) - size);

        memset (haystack + offs, needle, size);
        _memset (haystack_copy + offs, needle, size);

        if (memcmp (haystack, haystack_copy, sizeof (haystack)) != 0)
        {
            printf ("memset (%p, %d, %d) failure!\n", haystack_copy, needle, size);
            return 1;
        }
    }

    return 0;
}
