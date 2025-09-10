#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define PORT 8080

void handle_client(int new_socket)
{
    char buffer[1024] = {0};
    char *hello = "Hello from server.";
    int valread;

    valread = read(new_socket, buffer, 1024);
    printf("Received from client: %s\n", buffer);

    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent to client\n");

    close(new_socket);
    exit(0); // Terminate child process after handling client
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Ignore SIGCHLD to avoid zombie processes
    signal(SIGCHLD, SIG_IGN);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow socket reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configure address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Server loop to accept and fork for each client
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            continue;
        }

        // Fork a new process to handle the client
        pid_t pid = fork();
        if (pid == 0)
        {
            // In child process
            close(server_fd); // Child doesn't need listening socket
            handle_client(new_socket);
        }
        else if (pid > 0)
        {
            // In parent process
            close(new_socket); // Parent doesn't need client socket
        }
        else
        {
            perror("fork failed");
            close(new_socket);
        }
    }

    // Never reached, but good practice
    close(server_fd);
    return 0;
}
// Compile with: gcc -o server server.c
// Run with: ./server
// To test, you can use a simple client program or tools like `telnet` or `nc` (netcat).
// Example client command: echo "Hello" | nc localhost 8080
// This server will respond with "Hello from server." to any message it receives.
// Note: Ensure that the server is running before testing with a client.
// The server will handle multiple clients by forking a new process for each client connection.
// The server will ignore SIGCHLD to prevent zombie processes from child termination.
// The server will run indefinitely until manually stopped (e.g., with Ctrl+C).     