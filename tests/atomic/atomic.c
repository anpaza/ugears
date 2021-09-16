/**
 * Test features of atomic.h in a friendly environment.
 */
#include <stdint.h>
#include <stdio.h>

#define _JOIN2(a,b)		a##b
/// Join two tokens together and interpret the result as a new token
#define JOIN2(a,b)		_JOIN2(a, b)

// 0: interrupts enabled, 1: disabled
unsigned primask = 0;

static void __enable_irq ()
{
    primask = 0;
    printf ("\t%s -> %u\n", __FUNCTION__, primask);
}

static void __disable_irq ()
{
    primask = 1;
    printf ("\t%s -> %u\n", __FUNCTION__, primask);
}

static uint32_t __get_PRIMASK ()
{
    printf ("\t%s -> %u\n", __FUNCTION__, primask);
    return primask;
}

static __inline void __set_primask (uint32_t *val)
{
    primask = *val & 1;
    printf ("\trestoring PRIMASK to %d\n", primask);
}

// fool atomic.h
#define HARDWARE_H <stdint.h>
#define __set_primask __set_primask_
#include "useful/atomic.h"
#undef __set_primask

int main ()
{
    for (unsigned val = 0; val < 2; val++)
    {
        printf ("--- SET PRIMASK TO %u\n", primask = val);

        printf ("--- ATOMIC_BLOCK (RESTORE)\n");
        ATOMIC_BLOCK (RESTORE)
        {
            printf ("\tinterrupts off\n");
        }

        printf ("--- ATOMIC_BLOCK (FORCEON)\n");
        ATOMIC_BLOCK (FORCEON)
        {
            printf ("\tinterrupts off\n");
        }

        printf ("--- NONATOMIC_BLOCK (RESTORE)\n");
        NONATOMIC_BLOCK (RESTORE)
        {
            printf ("\tinterrupts on\n");
        }

        printf ("--- NONATOMIC_BLOCK (FORCEOFF)\n");
        NONATOMIC_BLOCK (FORCEOFF)
        {
            printf ("\tinterrupts on\n");
        }
    }

    return 0;
}
