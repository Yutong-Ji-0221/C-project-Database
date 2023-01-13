
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "client.h"
#include "auxiliary.h"
#include <sys/ioctl.h>

void validate_client_command_line(int argc, char **argv)
{
    if (argc != CLIENT_NUM_ARGS)
    {
        handle_usage_error(CLIENT_USAGE_ERROR_MESSAGE);
    }

    validate_port_number(argv[1]); // index position 1 is the port number
    validate_operation(argv[2]);
}

void validate_operation(char *operation)
{
    if (!(!strcmp(operation, GET) || !strcmp(operation, PUT) || !strcmp(operation, DELETE)))
    {
        handle_usage_error(OPERATION_ERROR);
    }
}

int open_listen(const char *port)
{
    // construct and set up the struct used to store address information
    struct addrinfo *ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // Using IPv4
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(NULL, port, &hints, &ai))
    { // Trying to get address
        freeaddrinfo(ai);
        fprintf(stderr, CANNOT_LISTEN);
        exit(0);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol

    if (connect(fd, (struct sockaddr *)ai->ai_addr, sizeof(struct sockaddr)))
    {
        freeaddrinfo(ai);
        fprintf(stderr, CANNOT_LISTEN);
        exit(0);
    }

    freeaddrinfo(ai);
    return fd;
}

char *recieve_from_host(int fd)
{
    char buffer[READ_BUFFER];
    read(fd, buffer, READ_BUFFER);

    char *message = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    for (int i = 0; i < strlen(buffer); i++)
    {
        message[i] = buffer[i];
    }

    return message;
}

void private_access_check(int fd)
{
    write(fd, TYPE, READ_BUFFER);
    char *message = recieve_from_host(fd);
    if (strcmp(message, PUBLIC))
    {
        char *authCode = strtok(message, " ");

        printf(PRIVATE_CHECK);
        char *userInput = get_user_input();

        if (strcmp(userInput, authCode))
        {
            free(message);
            free(userInput);
            write(fd, "fail", READ_BUFFER);
            handle_usage_error(DENY_ACCESS);
        }
        write(fd, "succ", READ_BUFFER);
        free(userInput);
    }
    free(message);
}

void handle_get(int fd)
{
    write(fd, GET, READ_BUFFER);
    char *keyColumnName = recieve_from_host(fd);

    printf(GET_PROMPT, keyColumnName);
    char *searchKey = get_user_input();
    check_put_input(searchKey);
    write(fd, searchKey, READ_BUFFER);
    free(searchKey);

    char *result = recieve_from_host(fd);

    char *status = strtok(result, " ");

    if (!strcmp(status, SUCCESS))
    {
        printf("%s\n", KEY_FOUND);
        char *token = strtok(NULL, SEP);
        while (token != NULL)
        {
            printf(" %s\n", token); // printing each token
            token = strtok(NULL, SEP);
        }
        exit(SUCCESS_USAGE);
    }
    else
    {
        printf("%s\n", KEY_NOT_FOUND);
        exit(SUCCESS_USAGE);
    }
}

void check_put_input(char *string)
{
    int lastIndex = strlen(string) - 1;
    if (!strcmp(string, "#"))
    {
        handle_usage_error(INVALID_INPUT_ERROR);
    }
    else if (strlen(string) > 1 &&
             (string[0] == '#' || string[lastIndex] == '#'))
    {
        handle_usage_error(INVALID_INPUT_ERROR);
    }
}

void handle_put(int fd)
{
    write(fd, PUT, READ_BUFFER);
    char *columnNames = recieve_from_host(fd);

    char *values = "";
    char *columnName = strtok(columnNames, SEP);

    while (columnName != NULL)
    {
        printf("Enter the value for [ %s ] column: ", columnName);
        char *value = get_user_input();
        check_put_input(value);
        values = my_strcat(values, value);
        values = my_strcat(values, SEP);
        columnName = strtok(NULL, SEP);
    }

    write(fd, values, READ_BUFFER);
    free(values);
}

void handle_delete(int fd)
{
    write(fd, DELETE, READ_BUFFER);
    char *keyColumnName = recieve_from_host(fd);

    printf(GET_PROMPT, keyColumnName);
    char *searchKey = get_user_input();
    write(fd, searchKey, READ_BUFFER);
    free(searchKey);

    char *result = recieve_from_host(fd);
    if (!strcmp(result, SUCCESS))
    {
        printf("%s\n", DELETE_SUCCESS);
    }
    else if (!strcmp(result, FAIL))
    {
        printf("%s\n", DELETE_FAIL);
    }
}

void handle_operation(char *operation, int fd)
{
    if (!strcmp(operation, GET))
    {
        handle_get(fd);
    }
    else if (!strcmp(operation, PUT))
    {
        handle_put(fd);
    }
    else if (!strcmp(operation, DELETE))
    {
        handle_delete(fd);
    }
}

int main(int argc, char **argv)
{
    validate_client_command_line(argc, argv);

    int fd = open_listen(argv[1]);
    char *status = recieve_from_host(fd);
    printf("%s\n", status);
    if (!strcmp(status, HOST_EXCEEDS))
    {
        return 0;
    }
    private_access_check(fd);
    handle_operation(argv[2], fd);

    return 0;
}
