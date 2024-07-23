#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>

void error(char* text);
void* handle_thread(void* args);

int clients[3];
int client_count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        error("No such port");
    }

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_size = sizeof(client_address);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0) {
        error("opening socket");
    }

    if (bind(sock_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        error("bind");
    }

    listen(sock_fd, 3);
    pthread_t tid;
    
    while (1) {
        int new_fd = accept(sock_fd, (struct sockaddr *)&client_address, &client_size);
        
        if (new_fd < 0) {
            error("accept");
        }

        printf("the client connected...\n");
        clients[client_count++] = new_fd;
        pthread_create(&tid, NULL, handle_thread, &new_fd);
    }

    close(sock_fd);
}

void error(char* text) {
    perror(text);
    exit(EXIT_FAILURE);
}

void* handle_thread(void* args) {
    int new_fd = *(int*)args;
    char buffer[256];

    while (1) {
        int recv_num = recv(new_fd, buffer, sizeof(buffer), 0);
        if (recv_num < 0) {
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < client_count; ++i) {
                if (clients[i] == new_fd) {
                    for (int j = i; j < client_count - 1; ++j) {
                        clients[j] = clients[j + 1];
                    }
                }
                --client_count;
                break;
            }
            pthread_mutex_unlock(&mutex);
            close(new_fd);
            pthread_exit(NULL);
        }

        buffer[recv_num] = '\0';
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < client_count; ++i) {
            if (clients[i] != new_fd) {
                send(clients[i], buffer, strlen(buffer), 0);
            }
        }
        
        pthread_mutex_unlock(&mutex);
        if (strcmp(buffer, "Exit") == 0) {
            break;
        }
    }

    return NULL;
}
