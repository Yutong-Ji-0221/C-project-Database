#include "strcuts.h"
int open_listen(const char *port, int maxConnections);
void *report(void *reportInfo);
DatabaseReport *database_report_init(Database *database, sem_t *lock, char *authCode, int maxConnections);
ClientInfo *client_info_init(int fd, DatabaseReport *databaseReport);
void *handle_client(void *clientInfo);
void send_server_unavaiable(int fd);
void process_connections(int fdServer, DatabaseReport *database);
int private_check_request(int fd, DatabaseReport *base);
void handle_get_request(int fd, DatabaseReport *base);
void handle_put_request(int fd, DatabaseReport *base);
void handle_delete_request(int fd, DatabaseReport *base);
