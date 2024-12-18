// Controller Node Implementation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

#define BUFFER_SIZE 256

// Node structure for the binary tree
typedef struct Node {
    int id;
    pid_t pid;
    struct Node* left;
    struct Node* right;
} Node;

Node* root = NULL;

// Function prototypes
Node* create_node(int id, pid_t pid);
Node* insert_node(Node* root, int id, pid_t pid);
Node* find_node(Node* root, int id);
void remove_node(Node** root, int id);
void print_tree(Node* root);

// Command handlers
void handle_create(int id, int parent);
void handle_exec(int id, char* text, char* pattern);
void handle_ping(int id);

// Utility functions
void sigchld_handler(int signum);

int main() {
    signal(SIGCHLD, sigchld_handler); // Handle child process termination

    char command[BUFFER_SIZE];
    printf("Distributed System Controller\n");

    while (1) {
        printf("> ");
        fgets(command, BUFFER_SIZE, stdin);

        char* token = strtok(command, " \n");
        if (!token) continue;

        if (strcmp(token, "create") == 0) {
            int id = atoi(strtok(NULL, " \n"));
            char* parent_str = strtok(NULL, " \n");
            int parent = parent_str ? atoi(parent_str) : -1;
            handle_create(id, parent);
        } else if (strcmp(token, "exec") == 0) {
            int id = atoi(strtok(NULL, " \n"));
            char* text = strtok(NULL, "\n");
            char* pattern = strtok(NULL, "\n");
            handle_exec(id, text, pattern);
        } else if (strcmp(token, "ping") == 0) {
            int id = atoi(strtok(NULL, " \n"));
            handle_ping(id);
        } else {
            printf("Unknown command\n");
        }
    }

    return 0;
}

Node* create_node(int id, pid_t pid) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->id = id;
    new_node->pid = pid;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

Node* insert_node(Node* root, int id, pid_t pid) {
    if (!root) return create_node(id, pid);

    if (!root->left) {
        root->left = insert_node(root->left, id, pid);
    } else if (!root->right) {
        root->right = insert_node(root->right, id, pid);
    } else {
        root->left = insert_node(root->left, id, pid);
    }

    return root;
}

Node* find_node(Node* root, int id) {
    if (!root) return NULL;
    if (root->id == id) return root;

    Node* left_result = find_node(root->left, id);
    if (left_result) return left_result;

    return find_node(root->right, id);
}

void print_tree(Node* root) {
    if (!root) return;
    printf("Node ID: %d, PID: %d\n", root->id, root->pid);
    print_tree(root->left);
    print_tree(root->right);
}

void handle_create(int id, int parent) {
    if (find_node(root, id)) {
        printf("Error: Already exists\n");
        return;
    }

    Node* parent_node = parent == -1 ? root : find_node(root, parent);
    if (parent != -1 && !parent_node) {
        printf("Error: Parent not found\n");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Error");
        return;
    }

    if (pid == 0) {
        execl("./compute_node", "compute_node", NULL);
        exit(0);
    }

    if (parent == -1 && !root) {
        root = create_node(id, pid);
    } else {
        insert_node(parent_node, id, pid);
    }

    printf("Ok: %d\n", pid);
}

void handle_exec(int id, char* text, char* pattern) {
    Node* node = find_node(root, id);
    if (!node) {
        printf("Error: %d: Not found\n", id);
        return;
    }

    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    char endpoint[BUFFER_SIZE];
    sprintf(endpoint, "ipc:///tmp/node_%d", id);
    zmq_connect(requester, endpoint);

    zmq_send(requester, text, strlen(text), 0);
    zmq_send(requester, pattern, strlen(pattern), 0);

    char buffer[BUFFER_SIZE];
    zmq_recv(requester, buffer, BUFFER_SIZE, 0);
    printf("Ok:%d:%s\n", id, buffer);

    zmq_close(requester);
    zmq_ctx_destroy(context);
}

void handle_ping(int id) {
    Node* node = find_node(root, id);
    if (!node) {
        printf("Error: Not found\n");
        return;
    }

    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    char endpoint[BUFFER_SIZE];
    sprintf(endpoint, "ipc:///tmp/node_%d", id);

    if (zmq_connect(requester, endpoint) != 0) {
        printf("Ok: 0\n");
    } else {
        zmq_close(requester);
        printf("Ok: 1\n");
    }

    zmq_ctx_destroy(context);
}

void sigchld_handler(int signum __attribute__((unused))) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child process %d terminated\n", pid);
    }
}