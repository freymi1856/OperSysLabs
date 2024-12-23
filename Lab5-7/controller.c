#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Узел бинарного дерева
typedef struct Node {
    int id;
    pid_t pid;
    char endpoint[256];
    struct Node *left;
    struct Node *right;
} Node;

// Глобальный корень идеально сбалансированного дерева
Node *root = NULL;

// Создание узла
Node* create_balanced_tree(int *ids, int start, int end) {
    if (start > end) return NULL;
    int mid = (start + end) / 2;

    Node *node = (Node*)malloc(sizeof(Node));
    node->id = ids[mid];
    snprintf(node->endpoint, sizeof(node->endpoint), "ipc:///tmp/node_%d", ids[mid]);
    node->pid = -1; // Процесс создаётся позже
    node->left = create_balanced_tree(ids, start, mid - 1);
    node->right = create_balanced_tree(ids, mid + 1, end);

    return node;
}

// Поиск узла по ID
Node* find_node(Node *root, int id) {
    if (!root) return NULL;
    if (id == root->id) return root;
    if (id < root->id) return find_node(root->left, id);
    return find_node(root->right, id);
}

// Функция отправки запросов узлу
int send_request(void *context, const char *endpoint, const char *request, char *reply, size_t reply_size) {
    void *socket = zmq_socket(context, ZMQ_REQ);
    if (!socket) return -1;

    int timeout = 1000; 
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(int));

    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        return -1;
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
        return -1;
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
    void *context = zmq_ctx_new();
    if (!context) {
        fprintf(stderr, "Error: cannot create ZMQ context\n");
        return 1;
    }

    // Создаём идеально сбалансированное дерево
    int ids[] = {10, 20, 30, 40, 50}; // Пример ID узлов
    int n = sizeof(ids) / sizeof(ids[0]);
    root = create_balanced_tree(ids, 0, n - 1);

    // Запускаем процессы для узлов
    for (int i = 0; i < n; i++) {
        Node *node = find_node(root, ids[i]);
        if (!node) continue;

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            execl("./worker", "./worker", node->endpoint, (char*)NULL);
            perror("exec");
            exit(1);
        }
        node->pid = pid;
    }

    char line[1024];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;

        char *cmd = strtok(line, " \t\n");
        if (!cmd) continue;

        if (strcmp(cmd, "exec") == 0) {
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

            text_string[strcspn(text_string, "\n")] = '\0';  // Убираем \n
            pattern_string[strcspn(pattern_string, "\n")] = '\0'; // Убираем \n

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
                printf("Ok: 0");
                continue;
            }
            printf("%s\n", reply);

        } else if (strcmp(cmd, "exit") == 0) {
            break;
        } else {
            printf("Error: Unknown command\n");
        }
    }

    // Завершение работы
    for (int i = 0; i < n; i++) {
        Node *node = find_node(root, ids[i]);
        if (node && node->pid > 0) {
            kill(node->pid, SIGKILL);
        }
    }

    zmq_ctx_destroy(context);
    return 0;
}