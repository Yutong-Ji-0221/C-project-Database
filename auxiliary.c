#include "auxiliary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

char *my_strcat(char *str1, char *str2)
{
    char *concat = malloc(strlen(str1) + strlen(str2) + 1);
    int index1, index2 = 0;
    for (index1 = 0; (concat[index2] = str1[index1]) != '\0'; ++index1, ++index2)
    {
    }

    for (index1 = 0; (concat[index2] = str2[index1]) != '\0'; ++index1, ++index2)
    {
    }
    return concat;
}

void handle_usage_error(char *message)
{
    fprintf(stderr, "%s\n", message);
    fflush(stderr);
    exit(USAGE_ERROR);
}

bool is_non_negative(char *string)
{
    if (strlen(string) == 0)
    {
        return true;
    }

    char *rest;
    int number;
    number = strtol(string, &rest, 10); // Number should be base 10

    if (strlen(rest) > 0 || number < 0)
    {
        return true;
    }

    return false;
}

void validate_connections(char *connections)
{
    if (is_non_negative(connections))
    {
        handle_usage_error(CONNECTION_ERROR);
    }
}

void validate_port_number(char *portNum)
{
    if (is_non_negative(portNum))
    {
        handle_usage_error(PORT_RANGE_ERROR);
    }

    int port = atoi(portNum);
    // 0 is the emprical port
    if (!(port == 0 || (port >= MIN_PORT_NUM && port <= MAX_PORT_NUM)))
    {
        handle_usage_error(PORT_RANGE_ERROR);
    }
}

void validate_type(char *type)
{
    if (!is_good_type(type))
    {
        handle_usage_error(TYPE_ERROR);
    }
}

bool is_good_type(char *type)
{
    if (!strcmp(type, PUBLIC) || !strcmp(type, PRIVATE))
    {
        return true;
    }

    return false;
}

char *obtain_auth_code(char *file)
{
    char *authFile = file;
    FILE *stream = fopen(authFile, "r");

    if (stream == NULL)
    {
        handle_usage_error(CANNOT_OPEN_FILE);
    }

    if (feof(stream)) // if file has no content
    {
        handle_usage_error(CANNOT_OBTAIN_CODE);
    }

    char authCode[READ_BUFFER];
    fgets(authCode, 100, stream);
    fclose(stream);

    char *p = malloc(sizeof(char) * (strlen(authCode) + 1));

    for (int i = 0; i < strlen(authCode); i++)
    {
        p[i] = authCode[i];
    }

    return p;
}

void validate_command_line(int argc, char **argv)
{
    if (argc < MIN_ARGUMENTS || argc > MAX_ARGUMENTS)
    {
        handle_usage_error(USAGE_ERROR_MESSAGE);
    }

    validate_connections(argv[2]); // Index position 2 is the connections
    validate_type(argv[3]);        // Index position 3 is the type

    if (argc == MAX_ARGUMENTS)
    {                                  // Port number is given
        validate_port_number(argv[4]); // Index 4 is the port number
    }
}

char *get_user_input(void)
{
    char buffer[READ_BUFFER];
    fgets(buffer, READ_BUFFER, stdin);

    if (buffer[0] == '\n')
    {
        handle_usage_error(INVALID_INPUT_ERROR);
    }

    char *p = malloc(sizeof(char) * (strlen(buffer)));
    for (int i = 0; i < strlen(buffer) - 1; i++)
    {
        p[i] = buffer[i];
    }

    p[strlen(buffer) - 1] = '\0'; // remove tailing new line

    return p;
}

bool has_duplicated_column_name(char **columnNames, int index, char *columnName)
{
    for (int i = 0; i < index; i++)
    {
        if (!strcmp(columnNames[i], columnName))
        {
            return true;
        }
    }
    return false;
}

char **get_user_column_input(int columns)
{
    char **columnNames = malloc(sizeof(char *) * 0);
    for (int i = 0; i < columns; i++)
    {
        if (i == 0)
        {
            printf(KEY_COLUMN_PROMPT);
        }
        else
        {
            printf(NORMAL_COLUMN_PROMPT);
        }

        char *columnName = get_user_input();

        if (has_duplicated_column_name(columnNames, i, columnName))
        {
            free_column_names(columnNames, i);
            handle_usage_error(INVALID_COLUMN_NAME_ERROR);
        }

        columnNames = realloc(columnNames, sizeof(char *) * (i + 1));
        columnNames[i] = columnName;
    }

    return columnNames;
}

void free_column_names(char **columnNames, int columnNum)
{
    for (int i = 0; i < columnNum; i++)
    {
        free(columnNames[i]);
    }

    free(columnNames);
}

void free_user_info(User_Input_Info *userInputInfo)
{
    free(userInputInfo->databaseName);
    free_column_names(userInputInfo->columnNames, userInputInfo->columnNum);
    free(userInputInfo);
}

User_Input_Info *prompt_column_name(void)
{
    User_Input_Info *userInputs = malloc(sizeof(User_Input_Info));

    printf(DATABASE_NAME_PROMPT);
    char *databaseName = get_user_input();
    // add checker for legit name
    userInputs->databaseName = databaseName;

    printf(COLUMN_NUMBER_PROMPT);
    char *columnNum = get_user_input();
    if (is_non_negative(columnNum))
    {
        handle_usage_error(INCORRECT_NUMBER);
    }

    userInputs->columnNum = atoi(columnNum);
    free(columnNum);

    char **columnNames = get_user_column_input(userInputs->columnNum);
    userInputs->columnNames = columnNames;

    return userInputs;
}
