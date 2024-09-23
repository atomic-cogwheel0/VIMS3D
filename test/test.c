#include <stdio.h>
#include "vec.h"

int main(void) {
    printf("%f\n", f2float(cos_f(float2f(1000))));
    printf("%f\n", f2float(mod_f(float2f(-11), float2f(-2))));
    printf("%f\n", f2float(floor_f(float2f(-65.4))));

    fixed f, u;

    /*for (int i = -100; i < 100; i++) {
        f = float2f((float)i/10.f);
        u = sin_f(f);
        for(int j = 0; j < (int)(f2float(u)*10.0f); j++)
            printf(" ");
        printf("# (%f)\n", f2float(u));
    }*/
}
