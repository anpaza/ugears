/**
 * Test features of atomic.h in a friendly environment.
 */
#include <stdint.h>
#include <stdio.h>

#define _JOIN2(a,b)		a##b
/// Join two tokens together and interpret the result as a new token
#define JOIN2(a,b)		_JOIN2(a, b)

static void __enable_irq ()
{
    printf ("\t%s\n", __FUNCTION__);
}

static void __disable_irq ()
{
    printf ("\t%s\n", __FUNCTION__);
}

static uint32_t __get_PRIMASK ()
{
    printf ("\t%s -> 1\n", __FUNCTION__);
    return 1;
}


/**
 * Creates a block of code that is executed atomically.
 * Upon entering the block the PRIMASK bit in PRIMASK register is set,
 * blocking any exceptions and interrupts except NMI.
 *
 * When leaving the block exceptions and interrupts are either restored 
 * to the state they were before entering the block (if ATOMIC_RESTORESTATE
 * keyword is used as argument to ATOMIC_BLOCK), or unconditionally enabled,
 * if ATOMIC_FORCEON is used as argument to ATOMIC_BLOCK.
 *
 * Since this is effectively a 'for' loop you may use 'break' to jump
 * to the end of the block, if needed.
 *
 * Usage example:
 * @verbatim
 * ATOMIC_BLOCK (ATOMIC_FORCEON)
 * {
 *      // Access var without possible interference from a interrupt handler
 *      if (flags & 0x100)
 *      {
 *          flag_8_count++;
 *          flags &= ~0x100;
 *      }
 *      if (flags & 0x200)
 *          break;
 *      if (flags & 0x400)
 *      {
 *          flag_10_count++;
 *          flags &= 0x400;
 *      }
 * }
 * @endverbatim
 *
 * @param type ATOMIC_RESTORESTATE or ATOMIC_FORCEON
 */
#define ATOMIC_BLOCK(type) \
    for (type | (__disable_irq (), 0), __pass = 1; __pass ; __pass = 0)

/**
 * Creates a block of code that is executed non-atomically.
 * Upon entering the block the PRIMASK bit in PRIMASK register is cleared,
 * allowing any exceptions and interrupts.
 *
 * When leaving the block exceptions and interrupts are either restored 
 * to the state they were before entering the block (if NONATOMIC_RESTORESTATE
 * keyword is used as argument to NONATOMIC_BLOCK), or unconditionally disabled,
 * if NONATOMIC_FORCEOFF is used as argument to NONATOMIC_BLOCK.
 *
 * @param type NONATOMIC_RESTORESTATE or NONATOMIC_FORCEOFF
 */
#define NONATOMIC_BLOCK(type) \
    for (type | (__enable_irq (), 0), __pass = 1; __pass ; __pass = 0)

static __inline void __set_primask (uint32_t *val)
{
    if (*val & 1)
        __disable_irq ();
    else
        __enable_irq ();
}

#define ATOMIC_RESTORESTATE \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = __get_PRIMASK ()
#define ATOMIC_FORCEON \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = 0
#define NONATOMIC_RESTORESTATE \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = __get_PRIMASK ()
#define NONATOMIC_FORCEOFF \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = 1


int main ()
{
    printf ("--- ATOMIC_BLOCK (ATOMIC_RESTORESTATE)\n");
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE)
    {
        printf ("\tinterrupts off\n");
    }

    printf ("--- ATOMIC_BLOCK (ATOMIC_FORCEON)\n");
    ATOMIC_BLOCK (ATOMIC_FORCEON)
    {
        printf ("\tinterrupts off\n");
    }

    printf ("--- NONATOMIC_BLOCK (NONATOMIC_RESTORESTATE)\n");
    NONATOMIC_BLOCK (NONATOMIC_RESTORESTATE)
    {
        printf ("\tinterrupts on\n");
    }

    printf ("--- NONATOMIC_BLOCK (NONATOMIC_FORCEOFF)\n");
    NONATOMIC_BLOCK (NONATOMIC_FORCEOFF)
    {
        printf ("\tinterrupts on\n");
    }

    return 0;
}
