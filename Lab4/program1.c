#include <stdio.h>
#include "libgcf.h"
#include "liblog.h"

int main() {
    int command;
    printf("Введите команду: ");
    scanf("%d", &command);

    if (command == 1) {
        int a, b;
        printf("Введите два числа: ");
        scanf("%d %d", &a, &b);
        printf("НОД(%d, %d) = %d\n", a, b, GCF(a, b));
    } else if (command == 2) {
        int x;
        printf("Введите x: ");
        scanf("%d", &x);
        printf("E(%d) = %.6f\n", x, E(x));
    } else {
        printf("Неизвестная команда\n");
    }

    return 0;
}