#ifndef RNG_H_B0D8921E93B562BC
#define RNG_H_B0D8921E93B562BC

#ifndef NOPICO
#include "pico/rand.h"
#else
#include <stdlib.h>
#include <time.h>
#endif


static void shuffle_array(uint8_t *array, int size)
{
#ifdef NOPICO
    srand(time(NULL));
#endif
    for (int i = size - 1; i > 0; i--) {
#ifndef NOPICO
        uint32_t r = get_rand_32();
#else
        uint32_t r = (uint32_t)rand();
#endif
        uint32_t j = r % (i + 1);
        uint8_t temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}


#endif // RNG_H_B0D8921E93B562BC
