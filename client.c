#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create the client socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Enter a message: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; // Remove the newline character

    // Send the message to the server
    if (write(client_fd, buffer, strlen(buffer) + 1) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Read the response from the server
    memset(buffer, 0, BUFFER_SIZE);
    if (read(client_fd, buffer, BUFFER_SIZE) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", buffer);

    // Close the client socket
    close(client_fd);

    return 0;
}

