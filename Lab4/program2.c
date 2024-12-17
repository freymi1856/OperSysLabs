#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

int main() {
    void *gcf_lib, *log_lib;
    int (*GCF_EUCLID)(int, int), (*GCF_NATIVE)(int, int);
    float (*E_FORMULA)(int), (*E_SUMM)(int);

    gcf_lib = dlopen("./libgcf.so", RTLD_LAZY);
    if (!gcf_lib) {
        fprintf(stderr, "Ошибка при загрузке libgcf.so: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    log_lib = dlopen("./liblog.so", RTLD_LAZY);
    if (!log_lib) {
        fprintf(stderr, "Ошибка при загрузке liblog.so: %s\n", dlerror());
        dlclose(gcf_lib);
        exit(EXIT_FAILURE);
    }

    GCF_EUCLID = dlsym(gcf_lib, "GCF_EUCLID");
    if (!GCF_EUCLID) {
        fprintf(stderr, "Ошибка загрузки GCF_EUCLID: %s\n", dlerror());
        dlclose(gcf_lib);
        dlclose(log_lib);
        exit(EXIT_FAILURE);
    }

    GCF_NATIVE = dlsym(gcf_lib, "GCF_NATIVE");
    if (!GCF_NATIVE) {
        fprintf(stderr, "Ошибка загрузки GCF_NATIVE: %s\n", dlerror());
        dlclose(gcf_lib);
        dlclose(log_lib);
        exit(EXIT_FAILURE);
    }

    E_FORMULA = dlsym(log_lib, "E_FORMULA");
    if (!E_FORMULA) {
        fprintf(stderr, "Ошибка загрузки E_FORMULA: %s\n", dlerror());
        dlclose(gcf_lib);
        dlclose(log_lib);
        exit(EXIT_FAILURE);
    }

    E_SUMM = dlsym(log_lib, "E_SUMM");
    if (!E_SUMM) {
        fprintf(stderr, "Ошибка загрузки E_SUMM: %s\n", dlerror());
        dlclose(gcf_lib);
        dlclose(log_lib);
        exit(EXIT_FAILURE);
    }

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

        } else if (command[0] == '2') { //Подсчет числа e
            int x;
            if (sscanf(command, "2 %d", &x) != 1){
                printf("Некорректный ввод. Введите: 2 <int>\n\n");
                continue;
            }
        
            printf("Число e (%d) - по формуле: %.6f\n", x, E_FORMULA(x));
            printf("Число e (%d) - по сумме рядов: %.6f\n", x, E_SUMM(x));
            printf("\n");

        } else if (command[0] == 'q') { //Выход
            break;
        } else {
            printf("Неизвестная команда. Используйте 1 для НОД и 2 для E.\n\n");
        }
    }

    printf("\n");

    dlclose(gcf_lib);
    dlclose(log_lib);

    return 0;
}