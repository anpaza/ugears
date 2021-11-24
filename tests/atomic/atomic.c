/**
 * Test features of atomic.h in a friendly environment.
 */
#include <stdint.h>
#include <stdio.h>

#define _ATOMIC_IRQ_STATE

// No generic cross-platform way to handle IRQ state, so no-ops

/// A datatype to hold the current IRQ enabled state
typedef uint8_t atomic_irq_state_t;

/// A value meaning "IRQs are enabled"
#define ATOMIC_IRQ_STATE_ENA 1
/// A value meaning "IRQs are disabled"
#define ATOMIC_IRQ_STATE_DIS 0

static atomic_irq_state_t g_irq_state;

/**
 * Get current IRQ enabled state
 * @return a value of atomic_irq_state_t type
 */
static inline atomic_irq_state_t atomic_irq_get_state ()
{
    printf ("\t%s -> %u\n", __FUNCTION__, g_irq_state);
    return g_irq_state;
}

/**
 * Set IRQ enabled state according to passed value.
 * @param state The new IRQ enabled state
 */
static inline void atomic_irq_set_state (atomic_irq_state_t state)
{
    g_irq_state = state;
    printf ("\t%s -> %u\n", __FUNCTION__, state);
}

#include "useful/atomic.h"

int main ()
{
    for (unsigned val = 0; val < 2; val++)
    {
        printf ("--- SET IRQ STATE TO %u\n", g_irq_state = val);

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
