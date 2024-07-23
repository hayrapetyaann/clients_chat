#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"

void error(const char* text);
void* read_thread(void* arg) ;
void* write_thread(void* arg);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        error("No such host\n");
    }
    
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(atoi(argv[1]));
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0) {
        error("socket opening: ");
    }

    if (connect(sock_fd, (struct sockaddr *)&server, len) < 0) {
        error("connect");
    }
    
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, read_thread, &sock_fd);
    pthread_create(&tid2, NULL, write_thread, &sock_fd);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    close(sock_fd);
    
    return 0;
}

void error(const char* text) {
    perror(text);
    exit(EXIT_FAILURE);
}

void* read_thread(void* arg) {
    int new_fd = *(int*)arg;
    char buffer[1024];

    while (1) {
        int rd = read(new_fd, buffer, sizeof(buffer));
        if (rd <= 0) {
            perror("read failed");
            close(new_fd);
            exit(EXIT_FAILURE);
        }
        buffer[rd] = '\0';
        printf("MSG: %s\n", buffer);
    }
}

void* write_thread(void* arg) {
    int new_fd = *(int*)arg;
    char buffer[1024];

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        int len = strlen(buffer);
        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        int wr = write(new_fd, buffer, len);

        if (wr < 0) {
            perror("write failed");
            close(new_fd);
            exit(EXIT_FAILURE);
        }
    }
}
