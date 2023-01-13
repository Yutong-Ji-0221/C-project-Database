#define OPERATION_ERROR "Operation has to be one of GET, PUT, DELETE"
#define PRIVATE_CHECK "You are accessing private database.\nPlease enter the passcode: "
#define DENY_ACCESS "500\nThe passcode is incorrect! Access is denied!"
#define KEY_FOUND "200: OK, the entry is found! They are:"
#define KEY_NOT_FOUND "404: Not Found, the entry is not found"
#define GET_PROMPT "Enter the searching key for [ %s ] column: "
#define DELETE_SUCCESS "200: OK, the entry has been deleted"
#define DELETE_FAIL "404: Not Found, the entry to be deleted is not found"

void validate_client_command_line(int argc, char **argv);
void validate_operation(char *operation);
int open_listen(const char *port);
char *recieve_from_host(int fd);
void private_access_check(int fd);
void handle_delete(int fd);
void handle_put(int fd);
void handle_get(int fd);
void check_put_input(char *string);
