#include <unistd.h>
#include "../include/tcp_server.h"
#include "../utils/syslogger.h"

int main() {
    init_syslog_connection();
    int server_fd = init_tcp_server();
    accept_connections(server_fd);
    close(server_fd);
    return 0;
}
