#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

int main() {
    void *gcf_lib, *log_lib;
    int (*GCF_EUCLID)(int, int), (*GCF_NATIVE)(int, int);
    float (*E_FORMULA)(int), (*E_SUMM)(int);

    // Загружаем библиотеки
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

    // Загружаем функции для НОД
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

    // Загружаем функции для числа e
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

    // Обработка команды
    char command[100];
    printf("Введите команду: ");
    fgets(command, 100, stdin);

    if (strncmp(command, "1", 1) == 0) {
        // Ввод чисел для НОД
        int a, b;
        sscanf(command, "1 %d %d", &a, &b);
        
        // Вывод результата НОД для двух методов
        printf("НОД (%d, %d) - Евклид: %d\n", a, b, GCF_EUCLID(a, b));
        printf("НОД (%d, %d) - Наивный: %d\n", a, b, GCF_NATIVE(a, b));

    } else if (strncmp(command, "2", 1) == 0) {
        // Ввод числа для вычисления e
        int x;
        sscanf(command, "2 %d", &x);
        
        // Вывод результата вычисления числа e для двух методов
        printf("Число e (%d) - по формуле: %.6f\n", x, E_FORMULA(x));
        printf("Число e (%d) - по сумме рядов: %.6f\n", x, E_SUMM(x));
    } else {
        printf("Неизвестная команда.\n");
    }

    dlclose(gcf_lib);
    dlclose(log_lib);

    return 0;
}