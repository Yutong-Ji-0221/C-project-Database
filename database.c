#include <stdio.h>
#include <stdlib.h>
#include "database.h"
#include "strcuts.h"
#include <string.h>
#include <stdbool.h>
#include "auxiliary.h"

Database *database_init(User_Input_Info *userInputInfo, char *type)
{
    Database *database = malloc(sizeof(Database));
    database->databaseName = userInputInfo->databaseName;
    database->numberOfColumns = userInputInfo->columnNum;
    database->numberOfRows = 0;
    database->columns = columns_init(userInputInfo->columnNames,
                                     userInputInfo->columnNum);

    if (!strcmp(type, PRIVATE))
    {
        database->type = 0;
    }
    else
    {
        database->type = 1;
    }

    return database;
}

Column **columns_init(char **columnNames, int columnNum)
{
    Column **columns = malloc(sizeof(Column *) * columnNum);

    for (int i = 0; i < columnNum; i++)
    {
        columns[i] = malloc(sizeof(Column));
        columns[i]->columnName = columnNames[i];
        columns[i]->value = malloc(sizeof(char *) * 0); // initially, no value stored
    }

    return columns;
}

// pre-condition columns and values has same amount of pointers
void update(Column **columns, int rowIndex, char **values, int numColumn)
{
    for (int i = 0; i < numColumn; i++)
    {

        columns[i]->value[rowIndex] = calloc((strlen(values[i]) + 1), sizeof(char));
        columns[i]->value[rowIndex] = values[i];
    }
}

void append(Column **columns, char **values, int numColumn, int numRow)
{
    for (int i = 0; i < numColumn; i++)
    {
        columns[i]->value = realloc(columns[i]->value, sizeof(char *) * (numRow + 1));
        columns[i]->value[numRow] = malloc(sizeof(char) * (strlen(values[i]) + 1));
        columns[i]->value[numRow] = values[i];
    }
}

void put(Database *database, char **values)
{
    char *key = values[0];
    Column *keyColumn = database->columns[0];

    for (int i = 0; i < database->numberOfRows; i++)
    {
        // if key exists in the current column, update it
        if (!strcmp(key, keyColumn->value[i]) || !strcmp(DELETION_MARKER, keyColumn->value[i]))
        {
            update(database->columns, i, values, database->numberOfColumns);
            return;
        }
    }

    // if key is not existed, we simply append it
    append(database->columns, values, database->numberOfColumns, database->numberOfRows);
    database->numberOfRows++;
}

// return -1 means, key is not found
int find_key_row_index(char *key, Database *database)
{
    int rowIndex = -1; // default by not found
    Column *keyColumn = database->columns[0];
    for (int i = 0; i < database->numberOfRows; i++)
    {
        if (!strcmp(key, keyColumn->value[i]))
        {
            rowIndex = i;
            break;
        }
    }

    return rowIndex;
}

char *find_by_key(int rowIndex, Database *database)
{
    char *values = "";
    for (int i = 0; i < database->numberOfColumns; i++)
    {
        values = my_strcat(values, database->columns[i]->columnName);
        values = my_strcat(values, ": ");
        values = my_strcat(values, database->columns[i]->value[rowIndex]);
        values = my_strcat(values, "#"); // # is consider as a separator
    }

    return values;
}

void delete_by_key(int rowIndex, Database *database)
{
    char *deletionMarker = "#";
    for (int i = 0; i < database->numberOfColumns; i++)
    {
        database->columns[i]->value[rowIndex] = calloc((strlen(deletionMarker) + 1), sizeof(char));
        database->columns[i]->value[rowIndex] = strdup(deletionMarker);
    }
}

char *get_column_names(Database *database)
{
    char *values = "";
    for (int i = 0; i < database->numberOfColumns; i++)
    {
        values = my_strcat(values, database->columns[i]->columnName);
        values = my_strcat(values, "#"); // # is consider as a separator
    }

    return values;
}

void free_database(Database *database)
{
    // free column
    for (int i = 0; i < database->numberOfColumns; i++)
    {
        for (int j = 0; i < database->numberOfRows; i++)
        {
            free(database->columns[i]->value[j]);
        }
        free(database->columns[i]->value);
        free(database->columns[i]);
    }

    free(database->columns);
    free(database);
}
