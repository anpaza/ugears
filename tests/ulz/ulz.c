#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "useful/ulz.h"
#include "useful/bitstream.h"

static const char *g_program;
static int g_verbose = 0;

static void display_version ()
{
    printf ("uLZ test program\n");
}

static void display_help ()
{
    display_version ();
    printf ("\nUsage: %s [option...] [file...]\n\n", g_program);
    printf ("  -v  --verbose  Increase verbosity level\n");
    printf ("  -V  --version  Display program version number\n");
    printf ("  -h  --help     Show this info\n");
}

static bool process (const char *fn)
{
    if (g_verbose)
    {
        printf ("%s ... ", fn);
        fflush (stdout);
    }

    bool compress = true;
    const char *dot = strrchr (fn, '.');
    if (dot)
        compress = (strcmp (dot, ".ulz") != 0);

    FILE *inf = fopen (fn, "rb");
    if (!inf)
    {
        if (g_verbose)
            printf ("ERROR\n");
        fprintf (stderr, "%s: Can't open file: '%s'\n", g_program, fn);
        return false;
    }

    fseek (inf, 0, SEEK_END);
    unsigned inf_size = ftell (inf);
    fseek (inf, 0, SEEK_SET);

    void *inf_buf = malloc (inf_size);
    unsigned bytes_read = fread (inf_buf, 1, inf_size, inf);
    fclose (inf);

    if (bytes_read != inf_size)
    {
        free (inf_buf);
        if (g_verbose)
            printf ("ERROR\n");
        fprintf (stderr, "%s: Can't read %u bytes from file '%s'\n",
                 g_program, inf_size, fn);
        return false;
    }

    char ofn [FILENAME_MAX + 1];
    void *outf_buf;
    unsigned outf_size = 0;
    if (compress)
    {
        snprintf (ofn, sizeof (ofn), "%s.ulz", fn);
        outf_buf = malloc (outf_size = inf_size);
        if (!ulz_compress (inf_buf, inf_size, outf_buf, &outf_size))
        {
            free (inf_buf);
            free (outf_buf);
            if (g_verbose)
                printf ("incompressible\n");
            else
                fprintf (stderr, "%s: File '%s' does not compress\n", g_program, fn);
            return false;
        }
    }
    else
    {
        snprintf (ofn, sizeof (ofn), "%.*s", (int)(dot - fn), fn);

    }

    free (inf_buf);

    if (g_verbose)
        printf ("%.1f%%\n", (100.0 * outf_size) / inf_size);

    FILE *outf = fopen (ofn, "wb");
    if (!outf)
    {
        free (outf_buf);
        fprintf (stderr, "%s: Can't open '%s' for writing!\n", g_program, fn);
        return false;
    }

    ulong bytes_written = fwrite (outf_buf, 1, outf_size, inf);
    fclose (outf);
    free (outf_buf);

    if (bytes_written != outf_size)
    {
        fprintf (stderr, "%s: Can't write %u bytes to file '%s'\n",
                 g_program, outf_size, ofn);
        return false;
    }

    return true;
}

#if 0
static void test ()
{
    static const char *test_data = "\nerawdrah ruoy tceleS #";
    bitstream_t ibs;
    bs_init (&ibs, (void *)test_data, strlen (test_data));

    bitstream_t obs;
    uint8_t obs_buff [30];
    bs_init (&obs, obs_buff, sizeof (obs_buff));

    int i;
    /*for (i = 1; i < 10; i++)
    {
        printf ("reading %d bytes: ", i);
        bs_read (&bs, odata, i);
        for (int j = 0; j < i; j++)
            printf ("%02x", ((uint8_t *)odata) [j]);
        printf ("\n");
    }*/

    for (i = 1; i < 32; i++)
    {
        printf ("reading %d bits: ", i);
        uint32_t x = bs_read_bits (&ibs, i);
        if (ibs.exhausted)
        {
            printf ("BUMM ibs!\n");
            break;
        }
        printf ("%08x\n", x);

        bs_write_bits (&obs, i, x);
        if (obs.exhausted)
        {
            printf ("BUMM obs!\n");
            break;
        }
    }

    if (!obs.exhausted)
    {
        unsigned sz = bs_write_finish (&obs, obs_buff, sizeof (obs_buff));
        printf ("[%u] [%.*s]\n", sz, sz, obs.ptr);
    }
}
#endif

int main (int argc, char *const *argv)
{
    //test (); return 0;

    static struct option long_options [] =
    {
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };

    g_program = argv [0];

    int c;
    while ((c = getopt_long (argc, argv, "vhV", long_options, 0)) != EOF)
        switch (c)
        {
            case '?':
                // unknown option
                return EXIT_FAILURE;

            case 'v':
                g_verbose++;
                break;

            case 'h':
                display_help ();
                return EXIT_FAILURE;

            case 'V':
                display_version ();
                return EXIT_FAILURE;

            default:
                // oops!
                abort ();
        }

    if (optind >= argc)
    {
        display_help ();
        return EXIT_FAILURE;
    }

    for (; optind < argc; optind++)
        if (!process (argv [optind]))
            return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
