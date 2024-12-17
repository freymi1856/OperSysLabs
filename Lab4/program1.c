#include <stdio.h>
#include "libgcf.h"
#include "liblog.h"

int main() {
    int command;
    printf("Введите команду: ");

    while (scanf("%d", &command) != EOF) {
        if (command == 1) { // Подсчет НОД
            int a, b;
            scanf("%d %d", &a, &b);
            printf("НОД (%d, %d) - Евклид: %d\n", a, b, GCF_EUCLID(a, b));
            printf("НОД (%d, %d) - Наивный: %d\n", a, b, GCF_NATIVE(a, b));
        } else if (command == 2) { // Расчет числа e
            int x;
            scanf("%d", &x);
            printf("E(%d) - По формуле: %f\n", x, E_FORMULA(x));
            printf("E(%d) - По сумме рядов: %f\n", x, E_SUMM(x));
        } else {
            printf("Неизвестная команда. Используйте 1 для GCF и 2 для E.\n");
        }
        printf("Введите команду: ");
    }
    printf("\n");
    
    return 0;
}