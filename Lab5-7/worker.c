#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <ctype.h>

// Функция для поиска шаблона в строке
void find_pattern(const char *text, const char *pattern, char *result, size_t result_size) {
    size_t text_len = strlen(text);
    size_t pat_len = strlen(pattern);

    if (pat_len > text_len) {
        strncpy(result, "-1", result_size);
        return;
    }

    char buffer[512] = {0};
    int found = 0;

    for (size_t i = 0; i <= text_len - pat_len; i++) {
        if (strncmp(&text[i], pattern, pat_len) == 0) {
            if (found > 0) strncat(buffer, ";", sizeof(buffer) - strlen(buffer) - 1);
            char pos[16];
            snprintf(pos, sizeof(pos), "%zu", i);
            strncat(buffer, pos, sizeof(buffer) - strlen(buffer) - 1);
            found++;
        }
    }

    if (found == 0) {
        strncpy(result, "-1", result_size);
    } else {
        strncpy(result, buffer, result_size - 1);
        result[result_size - 1] = '\0';
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: worker <endpoint>\n");
        return 1;
    }

    const char *endpoint = argv[1];

    // Создание контекста и сокета
    void *context = zmq_ctx_new();
    if (!context) {
        fprintf(stderr, "Error: Failed to create ZMQ context\n");
        return 1;
    }

    void *socket = zmq_socket(context, ZMQ_REP);
    if (!socket) {
        fprintf(stderr, "Error: Failed to create ZMQ socket\n");
        zmq_ctx_destroy(context);
        return 1;
    }

    if (zmq_bind(socket, endpoint) != 0) {
        fprintf(stderr, "Error: Failed to bind to %s\n", endpoint);
        zmq_close(socket);
        zmq_ctx_destroy(context);
        return 1;
    }

    while (1) {
        zmq_msg_t request;
        zmq_msg_init(&request);

        // Получение запроса
        int rc = zmq_msg_recv(&request, socket, 0);
        if (rc == -1) {
            zmq_msg_close(&request);
            break; // Ошибка или завершение
        }

        size_t req_size = zmq_msg_size(&request);
        char *req_str = malloc(req_size + 1);
        if (!req_str) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            zmq_msg_close(&request);
            break;
        }

        memcpy(req_str, zmq_msg_data(&request), req_size);
        req_str[req_size] = '\0';
        zmq_msg_close(&request);

        char *cmd = strtok(req_str, " ");

        if (strcmp(cmd, "exec") == 0) {
            char *text = strtok(NULL, " ");
            char *pattern = strtok(NULL, " ");
            char result[512];

            if (!text || !pattern) {
                zmq_send(socket, "Error: Invalid command", 22, 0);
            } else {
                find_pattern(text, pattern, result, sizeof(result));
                char reply[516];
                snprintf(reply, sizeof(reply), "Ok:%s", result);
                zmq_send(socket, reply, strlen(reply), 0);
            }
        } else if (strcmp(cmd, "ping") == 0) {
            zmq_send(socket, "Ok: 1", 6, 0);
        } else {
            zmq_send(socket, "Error: Unknown command", 23, 0);
        }

        free(req_str);
    }

    // Завершение работы
    zmq_close(socket);
    zmq_ctx_destroy(context);
    printf("Worker at %s terminated\n", endpoint);
    return 0;
}