#include <stdio.h>
#include <dlfcn.h>

int main() {
    char lib_gcf[256], lib_log[256];
    printf("Введите путь к библиотеке НОД: ");
    scanf("%s", lib_gcf);
    printf("Введите путь к библиотеке числа е: ");
    scanf("%s", lib_log);

    // Загрузка библиотек
    void *gcf_lib = dlopen(lib_gcf, RTLD_LAZY);
    void *log_lib = dlopen(lib_log, RTLD_LAZY);

    if (!gcf_lib || !log_lib) {
        printf("Ошибка загрузки библиотеки: %s\n", dlerror());
        return 1;
    }

    int (*GCF)(int, int) = dlsym(gcf_lib, "GCF");
    float (*E)(int) = dlsym(log_lib, "E");
    char *error;
    if ((error = dlerror()) != NULL) {
        printf("Ошибка поиска символов: %s\n", error);
        return 1;
    }

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

    dlclose(gcf_lib);
    dlclose(log_lib);
    return 0;
}