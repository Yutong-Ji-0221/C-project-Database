#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "host.h"
#include "database.h"
#include "strcuts.h"
#include "auxiliary.h"
#include "lock.h"
#include <sys/ioctl.h>
#include <fcntl.h>

int open_listen(const char *port, int maxConnections)
{
    struct addrinfo *ai = 0;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IP version 4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Listen on all IP addresses

    getaddrinfo("localhost", port, &hints, &ai);

    int listenFd = socket(AF_INET, SOCK_STREAM, 0); // 0 is the TCP protocol

    // Allow port number to be reused immediately
    int optVal = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int));

    if (bind(listenFd, (struct sockaddr *)ai->ai_addr, sizeof(struct sockaddr)) < 0)
    {
        fprintf(stderr, "%s\n", CANNOT_LISTEN);
        fflush(stderr);
        exit(USAGE_ERROR);
    }

    // Get the port number
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(struct sockaddr_in);

    getsockname(listenFd, (struct sockaddr *)&ad, &len);
    fprintf(stderr, "The database is listening on port: %u\n", ntohs(ad.sin_port));
    fflush(stderr);

    listen(listenFd, maxConnections);
    freeaddrinfo(ai);

    return listenFd;
}

void *report(void *reportInfo)
{
    ReportInfo *report = (ReportInfo *)reportInfo;
    sigset_t set = report->set;
    DatabaseReport *base = report->database;
    int signal;

    while (true)
    {
        // Waiting on any incoming singal
        sigwait(&set, &signal);
        if (signal == SIGHUP)
        {
            fprintf(stderr, REPORT_FORMAT, base->database->databaseName, base->connected, base->completed,
                    base->passcodeFail, base->getTimes, base->putTimes,
                    base->delTimes, base->database->numberOfRows);
            fflush(stderr);
        }
    }
}

void set_sigmask(DatabaseReport *database)
{
    pthread_t thread;
    sigset_t set;

    // Set a sigmask to response on SIGHUP signal
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // Allocate a pointer to reportInfo structure
    ReportInfo *reportInfo = (ReportInfo *)malloc(sizeof(ReportInfo));
    reportInfo->set = set;
    reportInfo->database = database;

    pthread_create(&thread, NULL, &report, reportInfo);
}

DatabaseReport *database_report_init(Database *database, sem_t *lock, char *authCode, int maxConnections)
{
    DatabaseReport *databaseReport = (DatabaseReport *)malloc(sizeof(DatabaseReport));
    // databaseReport->authCode = authCode;
    databaseReport->database = database;
    databaseReport->lock = lock;

    databaseReport->connected = 0;
    databaseReport->completed = 0;
    databaseReport->passcodeFail = 0;
    databaseReport->getTimes = 0;
    databaseReport->putTimes = 0;
    databaseReport->delTimes = 0;
    databaseReport->maxConnections = maxConnections;
    databaseReport->authCode = authCode;

    return databaseReport;
}

ClientInfo *client_info_init(int fd, DatabaseReport *databaseReport)
{
    ClientInfo *clientInfo = (ClientInfo *)malloc(sizeof(ClientInfo));
    clientInfo->fd = fd;
    clientInfo->databaseReport = databaseReport;

    return clientInfo;
}

int private_check_request(int fd, DatabaseReport *base)
{
    if (base->database->type == 0)
    {
        int bufferSize = strlen(base->authCode) + strlen(PRIVATE) + 1;
        char *buffer = (char *)malloc(sizeof(char) * bufferSize);
        sprintf(buffer, "%s %s", base->authCode, "private");
        write(fd, buffer, READ_BUFFER);
        free(buffer);

        char result[READ_BUFFER];
        read(fd, result, READ_BUFFER);
        if (!strcmp(result, "fail"))
        {
            return 0;
        }
        return 1;
    }
    else
    {
        write(fd, PUBLIC, READ_BUFFER);
        return 1;
    }
}

void handle_get_request(int fd, DatabaseReport *base)
{
    write(fd, base->database->columns[0]->columnName, READ_BUFFER); // index 0 is the key column

    take_lock(base->lock);
    char key[READ_BUFFER];
    read(fd, key, READ_BUFFER);
    int keyExisted = find_key_row_index(key, base->database);

    if (keyExisted != -1)
    {
        char *result = find_by_key(keyExisted, base->database);
        int bufferSize = strlen(SUCCESS) + strlen(result) + 1;
        char *buffer = (char *)malloc(sizeof(char) * bufferSize);
        sprintf(buffer, "%s %s", SUCCESS, result);
        write(fd, buffer, READ_BUFFER);
        free(result);
        free(buffer);
    }
    else
    {
        write(fd, FAIL, READ_BUFFER);
    }
    release_lock(base->lock);
    base->getTimes++;
}

void handle_put_request(int fd, DatabaseReport *base)
{
    take_lock(base->lock);
    char *columnNames = get_column_names(base->database);
    write(fd, columnNames, READ_BUFFER);
    free(columnNames);

    char values[READ_BUFFER];
    read(fd, values, READ_BUFFER);

    if (!strcmp(values, EMPTY))
    {
        release_lock(base->lock);
        return;
    }

    char **putValues = malloc(sizeof(char *) * base->database->numberOfColumns);
    int i = 0;

    char *value = strtok(values, SEP);
    while (value != NULL)
    {
        putValues[i] = strdup(value);
        i++;
        value = strtok(NULL, SEP);
    }

    put(base->database, putValues);
    release_lock(base->lock);
    base->putTimes++;
}

void handle_delete_request(int fd, DatabaseReport *base)
{
    write(fd, base->database->columns[0]->columnName, READ_BUFFER); // index 0 is the key column
    take_lock(base->lock);
    char key[READ_BUFFER];
    read(fd, key, READ_BUFFER);
    int keyExisted = find_key_row_index(key, base->database);

    if (keyExisted != -1)
    {
        delete_by_key(keyExisted, base->database);
        write(fd, SUCCESS, READ_BUFFER);
    }
    else
    {
        write(fd, FAIL, READ_BUFFER);
    }
    release_lock(base->lock);
    base->delTimes++;
}

void *handle_client(void *clientInfo)
{
    ClientInfo *client = (ClientInfo *)clientInfo;
    DatabaseReport *base = client->databaseReport;
    int fd = client->fd;
    write(fd, SUCC_CONNECT_MESSAGE, READ_BUFFER);

    while (1)
    {
        char buffer[READ_BUFFER];
        read(fd, buffer, READ_BUFFER);
        if (!strcmp(buffer, TYPE))
        {
            int success = private_check_request(fd, base);
            if (!success)
            {
                base->passcodeFail++;
                return NULL;
            }
        }
        else if (!strcmp(buffer, GET))
        {
            base->connected++;
            handle_get_request(fd, base);
            base->connected--;
            base->completed++;
            return NULL;
        }
        else if (!strcmp(buffer, PUT))
        {
            base->connected++;
            handle_put_request(fd, base);
            base->connected--;
            base->completed++;
            return NULL;
        }
        else if (!strcmp(buffer, DELETE))
        {
            base->connected++;
            handle_delete_request(fd, base);
            base->connected--;
            base->completed++;
            return NULL;
        }
    }
}

void send_server_unavaiable(int fd)
{
    char *response = HOST_EXCEEDS;
    write(fd, response, strlen(response));
}

void process_connections(int fdServer, DatabaseReport *database)
{
    int fd;
    struct sockaddr_in fromAddr;
    socklen_t fromAddrSize;

    while (true)
    {
        fromAddrSize = sizeof(struct sockaddr_in);

        // Block -  waiting for a new connection.
        fd = accept(fdServer, (struct sockaddr *)&fromAddr, &fromAddrSize);

        if (database->connected + 1 > database->maxConnections &&
            database->maxConnections != 0)
        {
            send_server_unavaiable(fd);
            close(fd);
            continue;
        }

        ClientInfo *clientInfo = client_info_init(fd, database);

        pthread_t threadId;
        pthread_create(&threadId, NULL, handle_client, (void *)clientInfo);
        pthread_detach(threadId);
    }
}

int main(int argc, char **argv)
{
    validate_command_line(argc, argv);

    char *authCode;
    if (!strcmp(argv[3], PRIVATE))
    { // if type is private
        authCode = obtain_auth_code(argv[1]);
    }
    User_Input_Info *userInputInfo = prompt_column_name();
    Database *database = database_init(userInputInfo, argv[3]);

    sem_t lock;
    init_lock(&lock);
    DatabaseReport *databaseReport;
    if (!strcmp(argv[3], PRIVATE))
    {
        databaseReport = database_report_init(database, &lock, authCode, atoi(argv[2]));
    }
    else
    {
        databaseReport = database_report_init(database, &lock, NULL, atoi(argv[2]));
    }
    set_sigmask(databaseReport);

    int fdServer;
    if (argc == MAX_ARGUMENTS)
    {
        // Index 4 is port number, and index 2 is connections
        fdServer = open_listen(argv[4], atoi(argv[2]));
    }
    else
    { // Open an ephemeral, if the port number not specified
        fdServer = open_listen("0", atoi(argv[2]));
    }

    process_connections(fdServer, databaseReport);

    return 0;
}
