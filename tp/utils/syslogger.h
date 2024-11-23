#ifndef SYSLOGGER_H
#define SYSLOGGER_H

void log_message_syslog(const char *timestamp, const char *conversation_id, const char *role, const char *message, const char *sentiment, float sentiment_value);

#endif // SYSLOGGER_H
