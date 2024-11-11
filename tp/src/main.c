#include <unistd.h>
#include "tcp_server.h"

int main() {
    int server_fd = init_tcp_server();
    accept_connections(server_fd);
    close(server_fd);
    return 0;
}