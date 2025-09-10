#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

void handle_sigterm(int sig)
{
    FILE *f = fopen("/tmp/daemon_log.txt", "a+");
    if (f)
    {
        fprintf(f, "\nDaemon shutting down due to SIGTERM...\n");
        fclose(f);
    }
    exit(0);
}

int main()
{
    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS); // Parent exits

    // Become session leader
    setsid();

    // Optional: Set umask to 0
    umask(0);

    // Change working directory to root
    chdir("/");

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Register SIGTERM handler
    signal(SIGTERM, handle_sigterm);

    // Daemon main loop
    while (1)
    {
        FILE *f = fopen("/tmp/daemon_log.txt", "a+");
        if (f)
        {
            fprintf(f, "Daemon running in background... ");
            fclose(f);
        }
        sleep(10);
    }

    return 0;
}
