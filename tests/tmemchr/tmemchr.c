#ifdef TARGET_POSIX
#define USING_LIBC
#endif

#include <useful/usefun.h>

#ifdef USING_LIBC
#include <stdio.h>
#include <string.h>
#endif

int main ()
{
    xs_rng_t rng;
    xs_init (rng, 0xaabbccdd);

    uint8_t haystack [256];
    for (unsigned alot = 0; alot < 1000000; alot++)
    {
        for (unsigned i = 0; i < ARRAY_LEN (haystack) / 4; i++)
            *(uint32_t *)&haystack [i * 4] = xs_rand (rng);

        for (unsigned t = 0; t < 100; t++)
        {
            char needle = xs_rand (rng);
            unsigned size = xs_rand (rng) & 255;
            unsigned offs = xs_rand (rng) % (ARRAY_LEN (haystack) - size);

            const void *r1 = memchr (haystack + offs, needle, size);
            const void *r2 = _memchr (haystack + offs, needle, size);
            if (r1 != r2)
            {
                printf ("memchr (%p, %d, %d) failed, %p != %p!\n",
                        haystack + offs, needle, size, r1, r2);
                return 1;
            }
        }
    }

    return 0;
}
