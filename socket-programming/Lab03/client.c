#include <stdio.h>         // For printf(), perror()
#include <stdlib.h>        // For exit()
#include <string.h>        // For strlen(), memset()
#include <unistd.h>        // For read(), close()
#include <arpa/inet.h>     // For inet_pton(), sockaddr_in
#define PORT 8080          // Use 8081 for multithreaded or 8082 for multiplexing
int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    sock = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // Set server port
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported\n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
    send(sock, hello, strlen(hello), 0);
    printf("Message sent to server\n");

    valread = read(sock, buffer, 1024);
    printf("Server: %s\n", buffer);

    close(sock);
    return 0;
}
