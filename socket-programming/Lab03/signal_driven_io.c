#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int server_fd = -1;
int client_fd = -1;

// SIGIO handler
void sigio_handler(int signo)
{
    char buffer[BUFFER_SIZE];

    if (client_fd == -1)
    {
        // Accept new connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1)
        {
            if (errno != EWOULDBLOCK && errno != EAGAIN)
                perror("accept error");
            return;
        }

        printf("Client connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Set client_fd to signal-driven mode
        fcntl(client_fd, F_SETOWN, getpid());
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC);
    }
    else
    {
        // Read from client
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes > 0)
        {
            printf("Client says: %s\n", buffer);
            send(client_fd, buffer, bytes, 0); // Echo
        }
        else if (bytes == 0)
        {
            printf("Client disconnected.\n");
            close(client_fd);
            client_fd = -1;
        }
        else
        {
            if (errno != EWOULDBLOCK && errno != EAGAIN)
            {
                perror("read error");
                close(client_fd);
                client_fd = -1;
            }
        }
    }
}

int main()
{
    struct sockaddr_in server_addr;

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set to non-blocking + signal-driven
    fcntl(server_fd, F_SETOWN, getpid());
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC);

    // 3. Setup signal handler
    struct sigaction sa;
    sa.sa_handler = sigio_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGIO, &sa, NULL);

    // 4. Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 5. Listen
    if (listen(server_fd, 5) == -1)
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Signal-driven TCP server listening on port %d...\n", PORT);

    // 6. Keep running until killed (signal handles I/O)
    while (1)
    {
        pause(); // Wait for signals
    }

    return 0;
}
