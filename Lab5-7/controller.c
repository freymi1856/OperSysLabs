#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

// Узел бинарного дерева
typedef struct Node {
    int id;
    pid_t pid;
    char endpoint[256];
    struct Node *left;
    struct Node *right;
} Node;

// Глобальный корень дерева
Node *root = NULL;

// Создание нового узла
Node* add_node(Node *root, int id) {
    if (!root) {
        Node *node = (Node*)malloc(sizeof(Node));
        node->id = id;
        snprintf(node->endpoint, sizeof(node->endpoint), "ipc:///tmp/node_%d", id);
        node->pid = -1;
        node->left = node->right = NULL;
        return node;
    }
    if (id < root->id) {
        root->left = add_node(root->left, id);
    } else if (id > root->id) {
        root->right = add_node(root->right, id);
    }
    return root;
}

// Поиск узла по ID
Node* find_node(Node *root, int id) {
    if (!root) return NULL;
    if (id == root->id) return root;
    if (id < root->id) return find_node(root->left, id);
    return find_node(root->right, id);
}

void sigchld_handler(int signum) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Wait for all child processes to terminate
    }
    errno = saved_errno;
}

// Освобождение дерева
void free_tree(Node *root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// Завершение процессов и очистка ресурсов
void cleanup_nodes(Node *root) {
    if (!root) return;

    if (root->pid > 0) {
        if (kill(root->pid, SIGTERM) == 0) {
            waitpid(root->pid, NULL, 0);
        }
        unlink(root->endpoint + 6); // Удаление файла сокета
    }

    cleanup_nodes(root->left);
    cleanup_nodes(root->right);
}

// Отправка запросов узлу
int send_request(void *context, const char *endpoint, const char *request, char *reply, size_t reply_size) {
    void *socket = zmq_socket(context, ZMQ_REQ);
    if (!socket) return -1;

    int timeout = 1000;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(int));
    zmq_setsockopt(socket, ZMQ_LINGER, &timeout, sizeof(int));

    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        return -1; // Узел недоступен
    }

    zmq_msg_t msg;
    zmq_msg_init_size(&msg, strlen(request));
    memcpy(zmq_msg_data(&msg), request, strlen(request));
    if (zmq_msg_send(&msg, socket, 0) == -1) {
        zmq_msg_close(&msg);
        zmq_close(socket);
        return -1;
    }
    zmq_msg_close(&msg);

    zmq_msg_t resp;
    zmq_msg_init(&resp);
    int rc = zmq_msg_recv(&resp, socket, 0);
    if (rc == -1) {
        zmq_msg_close(&resp);
        zmq_close(socket);
        return -1; // Ошибка при получении ответа
    }
    int len = rc;
    if (len < (int)reply_size) {
        memcpy(reply, zmq_msg_data(&resp), len);
        reply[len] = '\0';
    } else {
        memcpy(reply, zmq_msg_data(&resp), reply_size-1);
        reply[reply_size-1] = '\0';
    }

    zmq_msg_close(&resp);
    zmq_close(socket);
    return 0;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    void *context = zmq_ctx_new();
    if (!context) {
        fprintf(stderr, "Error: cannot create ZMQ context\n");
        return 1;
    }

    char line[1024];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;

        char *cmd = strtok(line, " \t\n");
        if (!cmd) continue;

        if (strcmp(cmd, "create") == 0) {
            char *id_str = strtok(NULL, " \t\n");
            if (!id_str) {
                printf("Error: Invalid command\n");
                continue;
            }
            int node_id = atoi(id_str);

            Node *node = find_node(root, node_id);
            if (node) {
                printf("Error: Already exists\n");
                continue;
            }

            root = add_node(root, node_id);
            node = find_node(root, node_id);

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                printf("Error: Cannot create node\n");
                continue;
            }
            if (pid == 0) {
                execl("./worker", "./worker", node->endpoint, (char*)NULL);
                perror("exec");
                exit(1);
            }

            node->pid = pid;
            printf("Ok: %d\n", pid);

        } else if (strcmp(cmd, "exec") == 0) {
            char *id_str = strtok(NULL, " \t\n");
            if (!id_str) {
                printf("Error: Invalid command\n");
                continue;
            }
            int node_id = atoi(id_str);
            Node *node = find_node(root, node_id);
            if (!node) {
                printf("Error:%d: Not found\n", node_id);
                continue;
            }

            printf("> ");
            fflush(stdout);
            char text_string[109];
            if (!fgets(text_string, sizeof(text_string), stdin)) continue;

            printf("> ");
            fflush(stdout);
            char pattern_string[109];
            if (!fgets(pattern_string, sizeof(pattern_string), stdin)) continue;

            text_string[strcspn(text_string, "\n")] = '\0';
            pattern_string[strcspn(pattern_string, "\n")] = '\0';

            char request[256];
            snprintf(request, sizeof(request), "exec %s %s", text_string, pattern_string);

            char reply[256];
            if (send_request(context, node->endpoint, request, reply, sizeof(reply)) != 0) {
                printf("Error:%d: Node is unavailable\n", node_id);
                continue;
            }

            printf("%s\n", reply);

        } else if (strcmp(cmd, "ping") == 0) {
            char *id_str = strtok(NULL, " \t\n");
            if (!id_str) {
                printf("Error: Invalid command\n");
                continue;
            }
            int node_id = atoi(id_str);
            Node *node = find_node(root, node_id);
            if (!node) {
                printf("Error:%d: Not found\n", node_id);
                continue;
            }

            char reply[256];
            if (send_request(context, node->endpoint, "ping", reply, sizeof(reply)) != 0) {
                printf("Ok: 0\n");
                continue;
            }
            printf("%s\n", reply);

        } else if (strcmp(cmd, "exit") == 0) {
            break;
        } else {
            printf("Error: Unknown command\n");
        }
    }

    cleanup_nodes(root);
    free_tree(root);
    zmq_ctx_shutdown(context);
    zmq_ctx_destroy(context);

    printf("Exiting...\n");

    return 0;
}