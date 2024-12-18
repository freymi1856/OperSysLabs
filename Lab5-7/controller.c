#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <zmq.h>

#define MAX_NODES 1024

typedef struct {
    int id;
    pid_t pid;
} node_t;

node_t nodes[MAX_NODES];
int node_count = 0;

// Find a node by ID
node_t* find_node(int id) {
    for (int i = 0; i < node_count; ++i) {
        if (nodes[i].id == id) {
            return &nodes[i];
        }
    }
    return NULL;
}

// Add a new node
int add_node(int id, pid_t pid) {
    if (node_count >= MAX_NODES) {
        return -1; // Error: Node limit reached
    }
    nodes[node_count].id = id;
    nodes[node_count].pid = pid;
    ++node_count;
    return 0; // Success
}

// Remove a node by PID
void controller_remove_node(pid_t pid) {
    for (int i = 0; i < node_count; ++i) {
        if (nodes[i].pid == pid) {
            // Shift remaining nodes
            for (int j = i; j < node_count - 1; ++j) {
                nodes[j] = nodes[j + 1];
            }
            --node_count;
            printf("Node with PID %d removed\n", pid);
            return;
        }
    }
}

// Signal handler for SIGCHLD
void sigchld_handler(int signum __attribute__((unused))) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child process %d terminated\n", pid);
        controller_remove_node(pid);
    }
}

int main() {
    signal(SIGCHLD, sigchld_handler);

    char buffer[256];
    printf("> ");
    while (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        char command[16];
        if (sscanf(buffer, "%15s", command) != 1) {
            printf("Error: Invalid command\n");
            continue;
        }

        if (strcmp(command, "create") == 0) {
            int id, parent_id;
            int args = sscanf(buffer + strlen(command), "%d %d", &id, &parent_id);

            if (find_node(id) != NULL) {
                printf("Error: Already exists\n");
                continue;
            }

            if (args == 2 && find_node(parent_id) == NULL) {
                printf("Error: Parent not found\n");
                continue;
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                continue;
            }

            if (pid == 0) {
                // Child process (compute node)
                char id_str[16];
                snprintf(id_str, sizeof(id_str), "%d", id);
                execl("./compute_node", "compute_node", id_str, NULL);
                perror("execl");
                exit(1);
            }

            // Parent process
            if (add_node(id, pid) == -1) {
                printf("Error: Node limit reached\n");
                continue;
            }

            printf("Ok: %d\n", pid);
        } else if (strcmp(command, "ping") == 0) {
            int node_id;
            if (sscanf(buffer + strlen(command), "%d", &node_id) == 1) {
                node_t *node = find_node(node_id);
                if (node == NULL) {
                    printf("Error: Not found\n");
                } else if (kill(node->pid, 0) == 0) { // Check if process exists
                    printf("Ok: 1\n");
                } else {
                    printf("Ok: 0\n");
                }
            } else {
                printf("Error: Invalid command format\n");
            }
        } else if (strcmp(command, "exec") == 0) {
            int id;
            char text[128], pattern[128];
            if (sscanf(buffer + strlen(command), "%d %127s %127s", &id, text, pattern) == 3) {
                node_t *node = find_node(id);
                if (!node) {
                    printf("Error: %d: Not found\n", id);
                    continue;
                }

                // Send exec command to compute_node
                void* context = zmq_ctx_new();
                void* requester = zmq_socket(context, ZMQ_REQ);
                char endpoint[64];
                snprintf(endpoint, sizeof(endpoint), "ipc:///tmp/node_%d", id);
                if (zmq_connect(requester, endpoint) != 0) {
                    printf("Error: Failed to connect to node %d\n", id);
                } else {
                    zmq_send(requester, text, strlen(text), 0);
                    zmq_send(requester, pattern, strlen(pattern), 0);

                    char result[256];
                    zmq_recv(requester, result, sizeof(result), 0);
                    printf("Ok: %d: %s\n", id, result);
                }
                zmq_close(requester);
                zmq_ctx_destroy(context);
            } else {
                printf("Error: Invalid command format\n");
            }
        } else if (strcmp(command, "exit") == 0){
            printf("finishing the program...\n");
            break;
        } else {
            printf("Unknown command\n");
        }

        printf("> ");
    }

    return 0;
}