#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <ctype.h>

void find_pattern(const char *text, const char *pattern, char *result) {
    size_t text_len = strlen(text);
    size_t pat_len = strlen(pattern);

    if (pat_len > text_len) {
        strcpy(result, "-1");
        return;
    }

    char buffer[512] = {0};
    int found = 0;

    for (size_t i = 0; i <= text_len - pat_len; i++) {
        if (strncmp(&text[i], pattern, pat_len) == 0) {
            if (found > 0) strcat(buffer, ";");
            char pos[16];
            snprintf(pos, sizeof(pos), "%zu", i);
            strcat(buffer, pos);
            found++;
        }
    }

    if (found == 0) {
        strcpy(result, "-1");
    } else {
        strcpy(result, buffer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: worker <endpoint>\n");
        return 1;
    }

    const char *endpoint = argv[1];

    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REP);
    if (zmq_bind(socket, endpoint) != 0) {
        fprintf(stderr, "Error: cannot bind to %s\n", endpoint);
        return 1;
    }

    while (1) {
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        int rc = zmq_msg_recv(&msg, socket, 0);
        if (rc == -1) {
            zmq_msg_close(&msg);
            break;
        }

        char *req_str = malloc(rc + 1);
        memcpy(req_str, zmq_msg_data(&msg), rc);
        req_str[rc] = '\0';
        zmq_msg_close(&msg);

        char *cmd = strtok(req_str, " ");
        if (strcmp(cmd, "exec") == 0) {
            char *text = strtok(NULL, " ");
            char *pattern = strtok(NULL, " ");
            char result[512];

            if (!text || !pattern) {
                zmq_send(socket, "Error: Invalid command", 22, 0);
            } else {
                find_pattern(text, pattern, result);
                char reply[516];
                snprintf(reply, sizeof(reply), "Ok:%s", result);
                zmq_send(socket, reply, strlen(reply), 0);
            }

        } else if (strcmp(cmd, "ping") == 0) {
            zmq_send(socket, "pong", 4, 0);
        } else {
            zmq_send(socket, "Error: Unknown command", 23, 0);
        }

        free(req_str);
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}