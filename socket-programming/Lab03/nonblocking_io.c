#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
    int server_fd, client_fd = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set server socket to non-blocking
    if (set_nonblocking(server_fd) == -1)
    {
        perror("Failed to set non-blocking");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Bind to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 5) == -1)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Non-blocking server listening on port %d...\n", PORT);

    while (1)
    {
        // 5. Try to accept new connection (non-blocking)
        if (client_fd == -1)
        {
            client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd == -1)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                    perror("Accept failed");
                else
                    printf("No clients yet...\n");
            }
            else
            {
                set_nonblocking(client_fd);
                printf("Client connected: %s:%d\n",
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));
            }
        }

        // 6. Try to read from client (non-blocking)
        if (client_fd != -1)
        {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(client_fd, buffer, BUFFER_SIZE);

            if (n > 0)
            {
                buffer[n] = '\0';
                printf("Client says: %s\n", buffer);
                send(client_fd, buffer, strlen(buffer), 0); // Echo
            }
            else if (n == 0)
            {
                printf("Client disconnected\n");
                close(client_fd);
                client_fd = -1;
            }
            else
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    perror("Read error");
                    close(client_fd);
                    client_fd = -1;
                }
                else
                {
                    printf("No data yet...\n");
                }
            }
        }

        sleep(1); // mimic your 5 sec pause idea with faster feedback
    }

    close(server_fd);
    return 0;
}
