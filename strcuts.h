#include <semaphore.h>
#include <signal.h>
#ifndef EXIT_CODES
#define EXIT_CODES
typedef struct
{
    char *columnName;
    char **value;
} Column;

typedef struct
{
    char *databaseName;
    Column **columns;
    int numberOfColumns;
    int numberOfRows;
    int type; // 0 is private, 1 is public
} Database;

typedef struct
{
    char *databaseName;
    int columnNum;
    char **columnNames;
} User_Input_Info;

typedef struct
{
    Database *database;
    sem_t *lock;
    int connected;
    int completed;
    int passcodeFail;
    int getTimes;
    int putTimes;
    int delTimes;
    char *authCode;
    int maxConnections;
} DatabaseReport;

// Structure that contains the file descriptor and the server's database that
// used to send back response to the client
typedef struct
{
    int fd;
    DatabaseReport *databaseReport;
} ClientInfo;

// Structure that contains the set for the sigmask and server's databse that
// used to report on the server.
typedef struct
{
    sigset_t set;
    DatabaseReport *database;
} ReportInfo;

#endif
