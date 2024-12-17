#include <stdio.h>
#include "libgcf.h"
#include "liblog.h"

int main() {
    char command[256];
    printf("Команды:\n");
    printf("1 <int> <int> для подсчета НОД.\n");
    printf("2 <int> для подсчета E.\n");
    printf("q для выхода.\n");
    printf("\n");

    while (1) {
        printf("Введите команду: ");
        fgets(command, sizeof(command), stdin);

        if (command[0] == '1') { // Подсчет НОД
            int a, b;
            if (sscanf(command, "1 %d %d", &a, &b) != 2){
                printf("Некорректный ввод. Введите: 1 <int> <int>\n\n");
                continue;
            }
            printf("НОД (%d, %d) - Евклид: %d\n", a, b, GCF_EUCLID(a, b));
            printf("НОД (%d, %d) - Наивный: %d\n", a, b, GCF_NATIVE(a, b));
            printf("\n");
        } else if (command[0] == '2') { // Расчет числа e
            int x;
            if (sscanf(command, "%d", &x) != 1){
                printf("Некорректный ввод. Введите: 2 <int>\n\n");
                continue;
            }
            printf("E(%d) - По формуле: %f\n", x, E_FORMULA(x));
            printf("E(%d) - По сумме рядов: %f\n", x, E_SUMM(x));
            printf("\n");
        } else if (command[0] == 'q'){ // Выход
            break;
        } else {
            printf("Неизвестная команда. Используйте 1 для НОД и 2 для E.\n\n");
        }
    }
    printf("\n");
    
    return 0;
}