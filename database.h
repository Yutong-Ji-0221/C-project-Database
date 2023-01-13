#include "strcuts.h"
#define HLINE "-----------------------------------------------------------------------"
#define PRIVATE "private"
#define LOAD_BUFFER 150
#define EMPTY ""
#define DELETION_MARKER "#"

Database *database_init(User_Input_Info *userInputInfo, char *type);
Column **columns_init(char **columnNames, int columnNum);
void delete_by_key(int rowIndex, Database *database);
void free_database(Database *database);
void update(Column **columns, int rowIndex, char **values, int numColumn);
void append(Column **columns, char **values, int numColumn, int numRow);
void put(Database *database, char **values);
char *find_by_key(int rowIndex, Database *database);
int find_key_row_index(char *key, Database *database);
char *get_column_names(Database *database);
