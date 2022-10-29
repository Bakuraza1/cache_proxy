/* Param describing the host address of the server host*/
#define SERVER_HOST "10.0.2.15"
/* Param describing the port of the server host*/
#define SERVER_PORT 8080
/* Param describing the different host and ports of server farm */
#define SERVER_LIST "10.0.2.15:3000/10.0.2.15:3001/10.0.2.15:3002"
/* Param describing number of servers writings SERVER_LIST params*/
#define SERVER_NUMBERS 3
/* Param to socket listen method: Does not define the maximum of server connections */
#define MAX_PENDING_SERVER_CONNECTIONS  10 
/* Param describing the size of the data reception buffer by the client*/
#define LEN_BUFFER_RECP_CLIENT 100000
/* Param describing the size of the data reception buffer by the server of server farm*/
#define LEN_BUFFER_RECP_SERVER 50000
