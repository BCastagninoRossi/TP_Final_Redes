#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>

#define HOSTNAME "localhost" 
#define PRIORITY 163          

void log_message_syslog(const char *conversation_id, const char *role, const char *message, const char *sentiment, float sentiment_value) {
    // Get current time
    time_t rawtime;
    struct tm * timeinfo;
    char timestamp[20]; // For timestamp: e.g., "Sep 16 03:14:16"
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%b %d %H:%M:%S", timeinfo);
    
    // Open syslog with your desired options
    openlog("ChatSyslog", LOG_PID | LOG_CONS, LOG_USER);

    // Format the message string
    char log_msg[10000];
    snprintf(log_msg, sizeof(log_msg), "<%d> %s %s %s %s %s (sentiment: %s sentiment_value: %.1f)",
             PRIORITY, timestamp, HOSTNAME, conversation_id, role, message, sentiment, sentiment_value);

    // Send to syslog
    syslog(LOG_INFO, "%s", log_msg);

    // Close syslog
    closelog();
}

int main() {
    // Example usage
    log_message_syslog("chat-0001", "AGENT", "Esto es un mensaje en syslog protocol", "positive", 0.8);
    log_message_syslog("chat-0002", "CLIENT", "Esto es otro mensaje en syslog protocol", "negative", -0.8);

    return 0;
}
