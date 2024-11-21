#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

// Функция для обработки строки с числами
void process_line(const char *line, int *results, int *count, int *error) {
    int nums[BUFFER_SIZE];
    int n = 0;

    char *token = strtok((char *)line, " ");
    while (token != NULL) {
        nums[n++] = atoi(token);
        token = strtok(NULL, " ");
    }

    if (n < 2) {
        *error = 1;  // Ошибка, если чисел меньше двух
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

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Ошибка создания pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка создания процесса");
        return 1;
    }

    // Дочерний процесс
    if (pid == 0) {
        close(pipe_fd[0]);
        FILE *file = fopen(filename, "r");
        if (!file) {
            perror("Ошибка открытия файла");
            close(pipe_fd[1]);
            exit(1);
        }

        char line[BUFFER_SIZE];
        int results[BUFFER_SIZE];
        while (fgets(line, sizeof(line), file)) {
            int count = 0;
            int error = 0;

            process_line(line, results, &count, &error);
            if (error) {
                fprintf(stderr, "Ошибка: деление на 0\n");
                fclose(file);
                close(pipe_fd[1]);
                exit(1);
            }

            if (write(pipe_fd[1], &count, sizeof(int)) == -1) {
                perror("Ошибка записи в pipe");
                fclose(file);
                close(pipe_fd[1]);
                exit(1);
            }
            if (write(pipe_fd[1], results, count * sizeof(int)) == -1) {
                perror("Ошибка записи в pipe");
                fclose(file);
                close(pipe_fd[1]);
                exit(1);
            }
        }

        fclose(file);
        close(pipe_fd[1]);
        exit(0);
    } else {
        // Родительский процесс
        close(pipe_fd[1]);

        int status;
        wait (&status);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0){
            fprintf(stderr, "Дочерний процесс завершился с ошибкой. Родительский процесс завершен\n");
            close(pipe_fd[0]);
            return 1;
        }

        int count;
        int results[BUFFER_SIZE];
        while (read(pipe_fd[0], &count, sizeof(int)) > 0) {
            if (read(pipe_fd[0], results, count * sizeof(int)) > 0) {
                printf("Результаты деления: ");
                for (int i = 0; i < count; i++) {
                    printf("%d ", results[i]);
                }
                printf("\n");
            }
        }

        close(pipe_fd[0]);
        wait(NULL);
    }

    return 0;
}
