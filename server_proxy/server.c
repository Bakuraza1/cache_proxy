/* standard symbols */
#include <unistd.h>

/* strings */
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* booleans */
#include <stdbool.h>

/* multithread */
#include <pthread.h>

/* sockets */
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

/* general_functions */
#include "general_functions.h"

/* config file */
#include "env.h"

/* Time */
#include <time.h>

//nuevos
#include <stdlib.h>

#define MAX_LINE 2048
#define MAX_FILENAME_SIZE 1024
#define MAX_RESPONSE_SIZE 50000


/* struct to params thread*/
struct thread_connection
{
    pthread_t thid;
    int connection_id;
    struct sockaddr_in client;
};

int current_server = 0;
long t_limite = 20;
FILE *file;

/* Nuevo cache */
void savecache(char *request, char *response);
int readCache();
void writeCache();
int check(char *str, char* direc);
void split(char *str, char *delimiter, char **arr);
void del_cache(int line);
void clean_cache();

void send_request_to_server(char *message, char *response);
void *get_data_of_client(void *params);
void savelog(char *type,char *log);

void saveLog(char *type, char *log){
    fprintf(file,"%s \n %s \n %s", type, log, "--------");
    fflush(file);
}

/* function to check if there is a cache and it is valid*/

int check(char *str, char* direc){
    
    char *client_message = str;
    char *aux[20];
    //open cache file
    FILE * fpointer = fopen("./cache/cachedupla.txt", "r");
    char buffer[MAX_LINE];
    int cont = 0;
    fgets(buffer, 150, fpointer);
    //Check if Client_message is in cache
    while(!feof(fpointer)){
        cont ++;
        fgets(buffer, 150, fpointer);
        split(buffer, "=", aux);  
        if(strcmp(aux[0],client_message) == 0 ){
            long t = get_time();
            char *remaining;
            long answer;
            answer = strtol(aux[1], &remaining, 10);
            //Check if that cache is valid
            if(t-answer < t_limite){
                fclose(fpointer);
                if(strcspn(aux[2], "\n")!= strlen(aux[2])){
                aux[2][strcspn(aux[2], "\n")] = 0;
                }
                strcpy( direc, aux[2]);
                //return 1 (Cache valid)
                return 1;
                
            }
            //If it is we delete that cache
            else{
                fclose(fpointer);
                del_cache(cont);
                //return 0 (Cache not found)
                return 0;
            }
            
        }
    }
    fclose(fpointer);
    //return 0 (Cache not found)
    return 0;
}

/*Function to delete a cache entry that is no longer valid*/

void del_cache(int line){
    printf("[SERVER_PROXY]: Deleting outdated cache\n");
    saveLog("","[SERVER_PROXY]: Deleting outdated cache\n");
    char buffer[MAX_LINE]; 
    char *aux[20];
    int deleteLine = line;
    //Open cache file and temporary cache file
    FILE * fpointer = fopen("./cache/cachedupla.txt", "r");
    FILE * fpointer2 = fopen("./cache/TEMP_cachedupla.txt", "w");
    fputs("\n",fpointer2);
    bool keep_reading = true;
    int current_line = 0;
    //Copy all lines of original cache file to temporary, except the line specified  (Line with cache outdated)
    while(1){
        if(fgets(buffer, 1024, fpointer) == NULL){
            break;
        }else if (strcmp (buffer, "\n") == 0){
            
        }else if (current_line != deleteLine){
            fputs(buffer,fpointer2);
        }else{
            split(buffer, "=", aux);  
            if(strcspn(aux[2], "\n")!= strlen(aux[2])){
            aux[2][strcspn(aux[2], "\n")] = 0;
            }
            remove(aux[2]);
        }
        current_line ++;
    }
    fclose(fpointer);
    fclose(fpointer2);
    //Delete old original file
    remove("./cache/cachedupla.txt");
    //Rename temp file to new cache file
    rename("./cache/TEMP_cachedupla.txt", "./cache/cachedupla.txt");
}

/*Function to read cache*/
int readCache(char *str, char *response ){
    char res[MAX_RESPONSE_SIZE];
    char direc[MAX_FILENAME_SIZE];
    printf("[SERVER_PROXY]: Searching for request in cache\n");
    saveLog("","[SERVER_PROXY]: Searching for request in cache\n");
    //Check if clients request is in cache
    if(check(str, direc) == 1){
        char *peticion = str;
        FILE * fpointer2 = fopen(direc, "r");
        printf("[SERVER_PROXY]: Request found in cache\n");
        saveLog("","[SERVER_PROXY]: Request found in cache\n");
        fseek(fpointer2, 0, SEEK_END);
        int length = ftell(fpointer2);
        fseek(fpointer2, 0, SEEK_SET);
        char c;
        int i = 0;
        char string[length];
        while((c = fgetc(fpointer2)) != EOF){
            string[i] = c;
            i++;
        }
        // if clients request is in cache copy answer to response
        string[i] = '\0';
        strcpy(response, string);
        fclose(fpointer2);
        return 0;
    }else{
        // if clients request is not in cache return 1
        printf("[SERVER_PROXY]: Request not in cache\n");
        saveLog("","[SERVER_PROXY]: Request not in cache\n");
        return 1;
    }
}

/* Function to write clients request onto cache file */
void writeCache(char *str, char *resp){
    //Open cache file
    FILE * fpointer = fopen("./cache/cachedupla.txt", "a");
    char *respuesta = resp;
    char dir[MAX_FILENAME_SIZE] = "./cache/archivos/";
    strcat(dir, str);
    FILE * fpointer2 = fopen(dir, "w");
    fprintf(fpointer2, "%s\n", respuesta);
    //fwrite(respuesta, 4, MAX_RESPONSE_SIZE, fpointer2);
    long t = get_time();
    //Copy clients request in format "request"-"time of request"-"direction of answer"
    fprintf(fpointer, "\n%s=%ld=%s", str, t, dir);
    fclose(fpointer);
    fclose(fpointer2);
    printf("[SERVER_PROXY]: Cache of %s saved\n", str);
    saveLog("\n[SERVER_PROXY]: Cache of %s saved\n", str);
}

/* function to get data and processing client information */
void *get_data_of_client(void *params)
{
    struct thread_connection thread_con = *(struct thread_connection *)params;

    char client_message[LEN_BUFFER_RECP_CLIENT];
    char message_response[LEN_BUFFER_RECP_SERVER];

    // Read Data
    int data_reception = recv(thread_con.connection_id, client_message, LEN_BUFFER_RECP_CLIENT, 0);
    if (data_reception == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error obtaining client information\n");
        // Show message error: Error obtaining customer information
        printf("[SERVER_PROXY]: Error obtaining client information\n");
        close(thread_con.connection_id);
        pthread_exit(&thread_con.thid);
    }
    saveLog("\n[SERVER_PROXY]: Client request:",client_message);
    printf("[SERVER_PROXY]: Client request:\n %s \n", client_message);

    // Check if you already have the resource and return it

    char *pet = "HTTP/1.1";
    char *newLine = strstr(client_message, pet);
    int until = (newLine-client_message+strlen(pet) + 1);
    char client_cache[until];
    for(int i = 0; i < until-1; i++){
        client_cache[i] = client_message[i];
    }
    client_cache[until -1] = '\0';
    char replace1 = ' ', replace2 = '/', newchar1 = '_', newchar2 = '-';
    replace_char(client_cache, replace1, newchar1);
    replace_char(client_cache, replace2,newchar2);
    if(readCache(client_cache, message_response)== 1){
        //Obtain the resource from the server
        send_request_to_server(client_message, message_response);
        //store in cache
        writeCache(client_cache, message_response);
    }

    // Send response to Client
    int send_response_client_status = send(thread_con.connection_id, message_response, LEN_BUFFER_RECP_SERVER, 0);
    if (send_response_client_status == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error sending response to client\n");
        // Show message error: Error sending response to client
        printf("[SERVER_PROXY]: Error sending response to client\n");

        close(thread_con.connection_id);
        pthread_exit(&thread_con.thid);
    }
    saveLog("\n[SERVER_PROXY]: Server response:",message_response);
    printf("[SERVER_PROXY]: Server response:\n %s \n", message_response);

    printf("[SERVER_PROXY]: client socket closed \n\n");

    // Close connection
    close(thread_con.connection_id);

    pthread_exit(NULL);
}

/* Function to send request to server*/
void send_request_to_server(char *message, char *response)
{
    // Define to with server to make the request: Round Robin

    // Obtenemos la lista de servidores
    char server_list[sizeof(SERVER_LIST)] = SERVER_LIST;
    char *servers[20];
    split(server_list, "/", servers);

    // Obtener un servidor en específico
    char *split_address[10];
    split(servers[current_server++], ":", split_address);
    char *server_host = split_address[0];
    int server_port = atoi(split_address[1]);

    printf("[SERVER_PROXY]: Connecting with server: Host(%s) - Port(%d)\n", server_host, server_port);

    // Reiniciar contador
    if (current_server == 3)
        current_server = 0;

    // Create socket address
    struct sockaddr_in sock_address;
    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = inet_addr(server_host);
    sock_address.sin_port = htons(server_port);

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error when creating socket like client\n");
        // Show message error: Error when creating socket like client"
        printf("[SERVER_PROXY]: Error when creating socket like client\n");

        return;
    }

    // Connect to socket
    int client_connection = connect(sock, (struct sockaddr *)&sock_address, sizeof(sock_address));
    if (client_connection == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error connecting to server socket as client\n");
        // Show message error: Error connecting to server socket as client"
        printf("[SERVER_PROXY]: Error connecting to server socket as client\n");

        return;
    }

    // Send request
    int send_status = send(sock, message, strlen(message), 0);
    if (send(sock, message, strlen(message), 0) == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error sending request to server socket as client\n");
        // Show message error: Error sending request to server socket as client"
        printf("[SERVER_PROXY]: Error sending request to server socket as client\n");
        close(sock);
        return;
    }



    // Get response

    int response_status = recv(sock, response, 50000, MSG_WAITALL);

    if (response_status == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error getting response to server socket as client\n");
        // Show message error: Error getting response to server socket as client"
        printf("[SERVER_PROXY]: Error getting response to server socket as client\n");
        close(sock);
        return;
    }
    close(sock);
}

int main(int argc, char *argv[])
{
    // Create socket address
    struct sockaddr_in sock_address;
    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = inet_addr(SERVER_HOST);
    sock_address.sin_port = htons(SERVER_PORT);

    //Open log file
    char name_file[] = "log.txt";
    file = fopen(name_file, "w");
    if (file == NULL){
	  printf("Error opening the log file\n");
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error when creating socket\n");
        // Show message error: Error when creating socket
        printf("[SERVER_PROXY]: Error when creating socket\n");

        return -1;
    }

    // Perform bind
    int sock_bind = bind(sock, (struct sockaddr *)&sock_address, sizeof(sock_address));
    if (sock_bind == -1)
    {
        // Save log
        saveLog("","[SERVER_PROXY]: Error when performing bind\n");
        // Show message error: Error when performing bind
        printf("[SERVER_PROXY]: Error when performing bind\n");

        return -1;
    }

    // Listen
    int socket_listen = listen(sock, MAX_PENDING_SERVER_CONNECTIONS);
    if (socket_listen != 0)
    {
        printf("[SERVER_PROXY]: Error when listening to clients\n");
        return -1;
    }
    else
    {
        printf("[SERVER_PROXY]: Listening...\n");
    }

    // Conections
    while (1)
    {
        // Create client socket address
        struct sockaddr_in client_address;
        int len_client = sizeof(client_address);

        // Accept connection
        int sock_accept = accept(sock, (struct sockaddr *)&client_address, &len_client);
        if (sock_accept == -1)
        {
            // Save log
            saveLog("","[SERVER_PROXY]: Error connecting to client\n");
            // Show message error: Error connecting to client
            printf("[SERVER_PROXY]: Error connecting to client\n");

            return -1;
        }
        else
        {
            // Create thread and params
            pthread_t tid0;
            struct thread_connection thr_conn;
            thr_conn.thid = tid0;
            thr_conn.connection_id = sock_accept;
            pthread_create(&tid0, NULL, get_data_of_client, &thr_conn);
        }
    }
    pthread_exit(NULL);
    fclose(file);
}