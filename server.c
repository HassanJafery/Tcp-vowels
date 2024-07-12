#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int count_vowels(char *str) {
    int count = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == 'a' || str[i] == 'e' || str[i] == 'i' || str[i] == 'o' || str[i] == 'u' ||
            str[i] == 'A' || str[i] == 'E' || str[i] == 'I' || str[i] == 'O' || str[i] == 'U') {
            count++;
        }
    }
    return count;
}

int main() {
    int server_fd, client_fds[MAX_CLIENTS], max_fd, num_clients = 0;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_len = sizeof(client_addr);
    fd_set read_fds, temp_fds;

    // Create the server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Initialize the file descriptor sets
    FD_ZERO(&read_fds);
    FD_ZERO(&temp_fds);
    FD_SET(server_fd, &read_fds);
    max_fd = server_fd;

    while (1) {
        temp_fds = read_fds;
        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Check for new client connections
        if (FD_ISSET(server_fd, &temp_fds)) {
            client_fds[num_clients] = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_fds[num_clients] < 0) {
                perror("accept");
                continue;
            }
            printf("Client %d connected: %s:%d\n", num_clients, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            FD_SET(client_fds[num_clients], &read_fds);
            if (client_fds[num_clients] > max_fd) {
                max_fd = client_fds[num_clients];
            }
            num_clients++;
        }

        // Check for client messages
        for (int i = 0; i < num_clients; i++) {
            if (FD_ISSET(client_fds[i], &temp_fds)) {
                memset(buffer, 0, BUFFER_SIZE);
                if (read(client_fds[i], buffer, BUFFER_SIZE) <= 0) {
                    printf("Client %d disconnected\n", i);
                    close(client_fds[i]);
                    FD_CLR(client_fds[i], &read_fds);
                    for (int j = i; j < num_clients - 1; j++) {
                        client_fds[j] = client_fds[j + 1];
                    }
                    num_clients--;
                    i--;
                    continue;
                }

                // Count the number of vowels in the message
                int num_vowels = count_vowels(buffer);

                // Send the result back to the client
                sprintf(buffer, "The message contains %d vowels.", num_vowels);
                if (write(client_fds[i], buffer, strlen(buffer) + 1) < 0) {
                    perror("write");
                    continue;
                }
            }
        }
    }

    // Close the server socket
    close(server_fd);

    return 0;
}

