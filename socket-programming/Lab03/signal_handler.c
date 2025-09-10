#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int server_fd = -1; // Global for access inside signal handler

// Signal handler for Ctrl+C (SIGINT)
void handle_sigint(int sig)
{
    printf("\nCaught SIGINT (Ctrl+C). Shutting down server gracefully...\n");
    if (server_fd != -1)
    {
        close(server_fd);
        printf("Server socket closed.\n");
    }
    exit(0);
}

int main()
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // 1. Register signal handler
    signal(SIGINT, handle_sigint);

    // 2. Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 3. Bind to IP and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Listen for connections
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("TCP Server running on port %d. Press Ctrl+C to stop.\n", PORT);

    // 5. Server loop
    while (1)
    {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0'; // Null-terminate
            printf("Client says: %s\n", buffer);
            send(client_fd, buffer, strlen(buffer), 0); // Echo back
        }
        else
        {
            printf("Client sent no data or disconnected.\n");
        }

        close(client_fd);
        printf("Client connection closed.\n");
    }

    return 0;
}
