#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE] = {0};

    if (argc != 2)
    {
        printf("Usage: %s <trusted|untrusted>\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *simulated_hostname = NULL;
    const char *simulated_ip = NULL;

    // Simulated identity
    if (strcmp(mode, "trusted") == 0)
    {
        simulated_hostname = "localhost"; // match server allowed list
        simulated_ip = "127.0.0.1";
    }
    else if (strcmp(mode, "untrusted") == 0)
    {
        simulated_hostname = "hacker.badguy.com";
        simulated_ip = "10.0.0.66";
    }
    else
    {
        printf("Invalid mode. Use 'trusted' or 'untrusted'.\n");
        return 1;
    }

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert localhost IP to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address/Address not supported");
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        return 1;
    }

    // Send simulated identity to server (optional, in case server reads message)
    snprintf(buffer, BUF_SIZE, "HELLO FROM: Hostname=%s, IP=%s\n", simulated_hostname, simulated_ip);
    send(sock, buffer, strlen(buffer), 0);

    // Receive server response
    int bytes_read = read(sock, buffer, BUF_SIZE - 1);
    buffer[bytes_read] = '\0';
    printf("Server response: %s\n", buffer);

    close(sock);
    return 0;
}
