#include "liblog.h"
#include <math.h>

// (1 + 1/x)^x
float E_FORMULA(int x) {
    return powf((1.0f + 1.0f / x), x);
}