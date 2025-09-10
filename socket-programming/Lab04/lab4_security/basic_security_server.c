#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define MAX_PENDING 5
#define BUF_SIZE 1024

// List of allowed hostnames and IPs
const char *allowed_hosts[] = { "mytrustedhost.local", NULL}; // for unauthorized access remove localhost
const char *allowed_ips[] = {"192.168.1.10", NULL};          // for unauthorized access remove "127.0.0.1"

// Check if IP is allowed
int is_ip_allowed(const char *ip)
{
    for (int i = 0; allowed_ips[i] != NULL; i++)
    {
        if (strcmp(ip, allowed_ips[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Check if hostname is allowed
int is_hostname_allowed(const char *hostname)
{
    for (int i = 0; allowed_hosts[i] != NULL; i++)
    {
        if (strcmp(hostname, allowed_hosts[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUF_SIZE];
    char client_ip[INET_ADDRSTRLEN];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, MAX_PENDING) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept client
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Convert IP to string
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Connection from IP: %s\n", client_ip);

    // Get client hostname
    struct hostent *host_entry = gethostbyaddr(&client_addr.sin_addr, sizeof(client_addr.sin_addr), AF_INET);
    if (host_entry)
    {
        printf("Resolved hostname: %s\n", host_entry->h_name);
    }
    else
    {
        printf("Hostname resolution failed\n");
    }

    // Apply simple security policy
    if ((host_entry && is_hostname_allowed(host_entry->h_name)) || is_ip_allowed(client_ip))
    {
        printf("Client authorized. Communicating...\n");

        // Simple interaction
        strcpy(buffer, "Hello from server. You are authorized.\n");
        send(client_fd, buffer, strlen(buffer), 0);
    }
    else
    {
        printf("Client not authorized. Closing connection.\n");
        strcpy(buffer, "Access denied.\n");
        send(client_fd, buffer, strlen(buffer), 0);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
