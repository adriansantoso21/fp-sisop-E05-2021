#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define PORT 8080

char buffer[1024];
char msg[1024];
bool isLogin = false;

void writetofile(const char username[], char message[], char path[]){
	FILE *f = fopen(path, "a");
	time_t t;
	struct tm *tmp;
	char timeBuff[100];

	time(&t);
	tmp = localtime(&t);
	strftime(timeBuff, sizeof(timeBuff), "%Y-%m-%d %H:%M:%S", tmp);
	fprintf(f, "%s:%s:%s\n", timeBuff, username, message);
	fclose(f);
}

int createClientSocket(struct sockaddr_in *address, struct sockaddr_in *serv_addr) {
    int fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(serv_addr, '0', sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &(serv_addr->sin_addr)) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if (connect(fd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    return fd;
}

void command(char *msg, int client_fd, const char *user_name) {
	char temp[1000];
    int iter;
    memset(temp, 0, sizeof(temp));

    for(int a =0; a < strlen(msg); a++){
        if(msg[a] == ';') break;
        temp[a] = msg[a];
    }

    if(strstr(temp, "CREATE USER") != NULL ){
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strstr(temp, "GRANT PERMISSION") != NULL ){
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strstr(temp, "USE") != NULL ){
        strcat(temp, " ");
        strcat(temp, user_name);
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strstr(temp, "CREATE DATABASE") != NULL ){
        strcat(temp, " ");
        strcat(temp, user_name);
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strstr(temp, "CREATE TABLE") != NULL ){
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strstr(temp, "DROP DATABASE") != NULL ){
        strcat(temp, " ");
        strcat(temp, user_name);
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strstr(temp, "DROP TABLE") != NULL ){
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strstr(temp, "DELETE FROM") != NULL ){
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    else if(strcmp(temp, "exit") == 0) {
        send(client_fd, temp, strlen(temp), 0);
    }

    else {
        printf("command salah.\n");
    }
}

int main(int argc, char const *argv[]) {
    char temp[1000];
    struct sockaddr_in address, serv_addr;
    int client_fd = createClientSocket(&address, &serv_addr);

    // cek server penuh / kosong
    memset(buffer, 0, sizeof(buffer));
    read(client_fd, buffer, 1024);

    if(strcmp(buffer, "server_penuh") == 0) {
        printf("Mohon maaf server penuh\n");
        return 0;
    }

    

    // Untuk root
    if(geteuid() == 0){
        // Mengecek apakah sudah terdaftar
        strcpy(temp, "MATCH USER root root");
        send(client_fd, temp, strlen(temp), 0); 
        read(client_fd, buffer, 1024);
        printf("Selamat datang di aplikasi database\n");
        printf("%s\n", buffer);

       while(strcmp(msg, "exit") != 0) {
	 	    scanf(" %[^\n]s", msg);
            writetofile("root", msg, "/home/adr01/Documents/FP/database/log.txt");
	 	    command(msg, client_fd, "root");
	 	}
    }

    // Untuk user
    else {
        if(argc < 5) printf("Maaf argumen kurang\n");
        else {
            // Mengecek apakah sudah terdaftar

            sprintf(temp, "MATCH USER %s %s", argv[2], argv[4]);
            send(client_fd, temp, strlen(temp), 0); 
            read(client_fd, buffer, 1024);

            if(strcmp(buffer, "Maaf username dan password salah") == 0 || strcmp(buffer, "Maaf user belum terdaftar") == 0) {
                printf("%s\n", buffer);
            }

            else {
                printf("Selamat datang di aplikasi database\n");
                printf("%s\n", buffer);
                while(strcmp(msg, "exit") != 0) {
                    scanf(" %[^\n]s", msg);
                    if(strstr(msg, "CREATE USER") != NULL || strstr(msg, "GRANT PERMISSION")!= NULL){
                        printf("Maaf perintah ini hanya untuk root\n");
                    }
                    else command(msg, client_fd, argv[2]);
                    writetofile(argv[2], msg, "/home/adr01/Documents/FP/database/log.txt");
                }
            }

        }
    }
    
    return 0;
}
