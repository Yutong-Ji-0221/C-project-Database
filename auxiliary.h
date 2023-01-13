#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "strcuts.h"

#define MIN_ARGUMENTS 4
#define MAX_ARGUMENTS 5
#define USAGE_ERROR 0
#define SUCCESS_USAGE 1
#define USAGE_ERROR_MESSAGE "Error : InvalidUsageError\nCorrect Usage: \
./host authfile connections type [port number]"
#define MIN_PORT_NUM 1024
#define MAX_PORT_NUM 65535
#define CANNOT_OBTAIN_CODE "Error: InvalidFileError, fail to obtain \
the authorisation information"
#define CANNOT_OPEN_FILE "Error: InvalidFileError, fail to open the file"
#define PORT_RANGE_ERROR "Error: InvalidPortNumberError, port number \
should be in range [1024, 65535]"
#define CONNECTION_ERROR "Error: InvalidConnectionError, connection number \
should be greater than 0"
#define TYPE_ERROR "Error: InvalidTypeError, type must be either public \
or private"
#define PUBLIC "public"
#define PRIVATE "private"
#define READ_BUFFER 100
#define DATABASE_NAME_PROMPT "Database Name: "
#define COLUMN_NUMBER_PROMPT "Number of columns: "
#define INCORRECT_NUMBER "Error: InvalidNumberOfColumnsError, Number of \
columns has to greater or equal to 1"
#define KEY_COLUMN_PROMPT "Key field column name: "
#define NORMAL_COLUMN_PROMPT "Column name: "
#define INVALID_INPUT_ERROR "Error: InvalidInputError, input cannot be empty or \
contains # in the leading or ending position"
#define INVALID_COLUMN_NAME_ERROR "Error: InvalidColumnNameError, column name \
has repeated"
#define CANNOT_LISTEN "Error: ConnectionError, unable to open socket for listening"
#define REPORT_FORMAT "Database Name: %s\nTotal number of connecting clients:%d\nTotal number of \
clients that completed their operations:%d\nTotal number of failing access private database:%d\n\
Total number of GET operations:%d\nTotal number of PUT operations:%d\nTotal number of DELETE operations:%d\n\
Total entries: %d\n"
#define CLIENT_NUM_ARGS 3
#define CLIENT_USAGE_ERROR_MESSAGE "Error : InvalidUsageError\nCorrect Usage: \
./client portNumber operation."
#define GET "GET"
#define PUT "PUT"
#define DELETE "DELETE"
#define TYPE "TYPE"
#define SUCC_CONNECT_MESSAGE "Successfully connecting to the host!"
#define HOST_EXCEEDS "Server is not avaiable right now, due to reaching maximum clients"
#define SUCCESS "200"
#define FAIL "404"
#define SEP "#"

char *my_strcat(char *str1, char *str2);
void handle_usage_error(char *message);
bool is_non_negative(char *string);
void validate_connections(char *connections);
void validate_port_number(char *portNum);
void validate_type(char *type);
bool is_good_type(char *type);
void validate_command_line(int argc, char **argv);
char *obtain_auth_code(char *file);
char *get_user_input(void);
bool has_duplicated_column_name(char **columnNames, int index, char *columnName);
char **get_user_column_input(int columns);
void free_column_names(char **columnNames, int columnNum);
User_Input_Info *prompt_column_name(void);
void free_user_info(User_Input_Info *userInputInfo);
