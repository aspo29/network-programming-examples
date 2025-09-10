#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define PORT 8082
#define MAX_CLIENTS 10

int main()
{
    int server_fd, client_fds[MAX_CLIENTS], new_client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];
    fd_set readfds;
    int max_fd, i;

    // Initialize client list
    for (i = 0; i < MAX_CLIENTS; i++)
        client_fds[i] = -1;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        // Add existing client sockets to readfds
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] > 0)
            {
                FD_SET(client_fds[i], &readfds);
                if (client_fds[i] > max_fd)
                    max_fd = client_fds[i];
            }
        }

        // Wait for activity
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        // New connection
        if (FD_ISSET(server_fd, &readfds))
        {
            new_client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
            printf("New client connected: %d\n", new_client_fd);

            // Add to client list
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_fds[i] < 0)
                {
                    client_fds[i] = new_client_fd;
                    break;
                }
            }
        }

        // Check each client
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = client_fds[i];
            if (fd > 0 && FD_ISSET(fd, &readfds))
            {
                memset(buffer, 0, sizeof(buffer));
                int bytes = read(fd, buffer, sizeof(buffer));
                if (bytes <= 0)
                {
                    printf("Client %d disconnected\n", fd);
                    close(fd);
                    client_fds[i] = -1;
                }
                else
                {
                    printf("Client %d: %s\n", fd, buffer);
                    send(fd, buffer, strlen(buffer), 0); // Echo back
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
