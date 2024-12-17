#include "liblog.h"

// Сумма ряда (1/n!)
float E(int x) {
    float e = 1.0f;
    float fact = 1.0f;
    for (int i = 1; i <= x; i++) {
        fact *= i;
        e += 1.0f / fact;
    }
    return e;
}