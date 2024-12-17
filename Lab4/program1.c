#include <stdio.h>
#include "libgcf.h"     // Заголовочный файл для функций НОД
#include "liblog.h"     // Заголовочный файл для функций расчета e

int main() {
    int command;
    printf("Введите команду: ");
    
    while (scanf("%d", &command) != EOF) {
        if (command == 1) { // Подсчет НОД
            int a, b;
            scanf("%d %d", &a, &b);
            printf("НОД(%d, %d) = %d\n", a, b, GCF(a, b));
        } else if (command == 2) { // Расчет числа e
            int x;
            scanf("%d", &x);
            printf("E(%d) = %f\n", x, E(x));
        } else {
            printf("Неизвестная команда. Используйте 1 для GCF и 2 для E.\n");
        }
        printf("Введите команду: ");
    }
    
    return 0;
}