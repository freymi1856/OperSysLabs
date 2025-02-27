#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

// Флаг для сигнализации о готовности данных
volatile sig_atomic_t data_ready = 0;
// Флаг для отслеживания завершения дочернего процесса
volatile sig_atomic_t child_terminated = 0;

// Обработчик сигнала SIGUSR1
void handle_signal(int sig) {
    data_ready = 1;
}

// Обработчик сигнала SIGCHLD
void handle_child_termination(int sig) {
    child_terminated = 1;
}

void process_line(const char *line, int *results, int *count, int *error) {
    int nums[BUFFER_SIZE];
    int n = 0;

    char *token = strtok((char *)line, " ");
    while (token != NULL) {
        nums[n++] = atoi(token);
        token = strtok(NULL, " ");
    }

    if (n < 2) {
        *error = 2;  // Если чисел меньше двух
        return;
    }

    for (int i = 1; i < n; i++) {
        if (nums[i] == 0) {
            *error = 1;  // Деление на 0
            return;
        }
        results[i - 1] = nums[0] / nums[i];
    }

    *count = n - 1;
}

int main() {
    char filename[BUFFER_SIZE];
    printf("Введите имя файла: ");
    scanf("%s", filename);

    // Сощдание отображаемого файла
    int fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Ошибка создания отображаемого файла");
        return 1;
    }
    ftruncate(fd, BUFFER_SIZE * sizeof(int));

    int *shared_memory = mmap(NULL, BUFFER_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Ошибка отображения памяти");
        close(fd);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка создания процесса");
        return 1;
    }

    if (pid == 0) { // Дочерний процесс
        signal(SIGUSR1, handle_signal);

        FILE *file = fopen(filename, "r");
        if (!file) {
            perror("Ошибка открытия файла");
            exit(1);
        }

        char line[BUFFER_SIZE];
        while (fgets(line, sizeof(line), file)) {
            int count = 0, error = 0;

            process_line(line, shared_memory + 1, &count, &error);
            if (error == 1) { // Деление на 0
                fprintf(stderr, "Ошибка: деление на 0\n");
                fclose(file);
                shared_memory[0] = -1;
                kill(getppid(), SIGUSR1);
                exit(1);
            } else if (error == 2) { // Недостаточно чисел
                fprintf(stderr, "Ошибка: недостаточно чисел для деления\n");
                fclose(file);
                shared_memory[0] = -1;
                kill(getppid(), SIGUSR1);
                exit(1);
            }

            shared_memory[0] = count;
            kill(getppid(), SIGUSR1);

            while (!data_ready);
            data_ready = 0;
        }

        // Сигнализируем об окончании работы (count = 0)
        shared_memory[0] = 0;
        kill(getppid(), SIGUSR1);

        fclose(file);
        exit(0);
    } else { // Родительский процесс
        struct sigaction sa_usr1 = {0};
        sa_usr1.sa_handler = handle_signal;
        sigaction(SIGUSR1, &sa_usr1, NULL);

        struct sigaction sa_chld = {0};
        sa_chld.sa_handler = handle_child_termination;
        sigaction(SIGCHLD, &sa_chld, NULL);

        while (1) {
            
            pause(); // Ждем сигнала

            if (shared_memory[0] == 0) { // Если дочерний процесс закончил
                break; 
            }
            if (shared_memory[0] == -1) { // Обработка ошибки
                break;
            }

            printf("Результаты деления: ");
            for (int i = 0; i < shared_memory[0]; i++) {
                printf("%d ", shared_memory[i + 1]);
            }
            printf("\n");

            kill(pid, SIGUSR1); // Подтверждаем получение данных
        }

        munmap(shared_memory, BUFFER_SIZE * sizeof(int));
        shm_unlink("/shared_memory");
    }

    return 0;
}
