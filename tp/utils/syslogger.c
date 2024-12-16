#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SYSLOG_SERVER "127.0.0.1"  // Use loopback IP address
#define SYSLOG_PORT 514
#define HOSTNAME "localhost"
#define PRIORITY 163

int syslog_socket;
struct sockaddr_in syslog_addr;

void init_syslog_connection() {
    // Create a UDP socket
    syslog_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (syslog_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure the syslog server address
    memset(&syslog_addr, 0, sizeof(syslog_addr));
    syslog_addr.sin_family = AF_INET;
    syslog_addr.sin_port = htons(SYSLOG_PORT);
    if (inet_pton(AF_INET, SYSLOG_SERVER, &syslog_addr.sin_addr) <= 0) {
        perror("Invalid syslog server address");
        close(syslog_socket);
        exit(EXIT_FAILURE);
    }
}

void log_message_syslog(const char *timestamp, const char *conversation_id, const char *role, const char *message, const char *sentiment, float sentiment_value) {
    // Format the message string
    char log_msg[10000];
    snprintf(log_msg, sizeof(log_msg), "<%d> %s %s %s %s %s (sentiment: %s sentiment_value: %.1f)",
             PRIORITY, timestamp, HOSTNAME, conversation_id, role, message, sentiment, sentiment_value);

    // Send the message to the syslog server
    int bytes_sent = sendto(syslog_socket, log_msg, strlen(log_msg), 0, (struct sockaddr *)&syslog_addr, sizeof(syslog_addr));
    if (bytes_sent < 0) {
        perror("Error sending syslog message");
    } else {
        printf("Syslog message sent: %s\n", log_msg);
    }
}

// ...existing code...
