#ifdef TARGET_POSIX
#define USE_LIBC
#endif

#include <useful/clike.h>
#include <useful/usefun.h>

#ifdef USE_LIBC
#include <stdio.h>
#include <string.h>
#endif

int main ()
{
    printf ("Please stand by ...\n");

    xs_rng_t rng;
    xs_init (rng, 0xaabbccdd);

    uint8_t haystack [256];
    for (unsigned alot = 0; alot < 1000000; alot++)
    {
        for (unsigned i = 0; i < ARRAY_LEN (haystack) / 4; i++)
            *(uint32_t *)&haystack [i * 4] = xs_rand (rng);

        for (unsigned t = 0; t < 100; t++)
        {
            uint8_t needle = xs_rand (rng);
            unsigned size = xs_rand (rng) & 255;
            unsigned offs = xs_rand (rng) % (ARRAY_LEN (haystack) - size);

            const void *r1 = memchr (haystack + offs, needle, size);
            const void *r2 = _memchr (haystack + offs, needle, size);
            if (r1 != r2)
            {
                printf ("memchr (%p, 0x%02x, %d) failed, %p != %p!\n",
                        haystack + offs, needle, size, r1, r2);
                return 1;
            }

            r1 = memrchr (haystack + offs, needle, size);
            r2 = _memrchr (haystack + offs, needle, size);
            if (r1 != r2)
            {
                printf ("memrchr (%p, 0x%02x, %d) failed, %p != %p!\n",
                        haystack + offs, needle, size, r1, r2);
                return 1;
            }
        }
    }

    printf ("Tests complete\n");

    return 0;
}
