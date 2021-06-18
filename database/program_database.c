#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h> 
#define PORT 8080

char currPath[1000]; //path untuk "../Server/"
char txtPath[1000]; //path untuk akun.txt

char buffer[1024] = ""; //untuk read
char msg[1024] = ""; //untuk send
int jumlah_akun = -1, jumlah_permission = -1, jumlah_db = -1, jumlah_file = -1;

char db_sekarang[1000];

bool isClientExist = false;

typedef struct akun {
	char id[1000];
	char pass[1000];
} akun;

typedef struct permission {
    char db[1000];
    char id[1000];
} permission;

typedef struct file {
    char db[1000];
    char namaFile[1000];
    char namakolom[1000][1000];
    char tipedata[1000][1000];
} file;

akun client_akun[1000];
permission client_permission[1000];
file client_file[1000];
char client_db[1000][1000];

// gunakan hanya di main
char *strCt(char *str1, char *str2) {
    static char buffer[255];
    sprintf(buffer, "%s%s", str1, str2);
    return buffer;
}

void createDir(char *dir) {
	struct stat st = {0};

	if (stat(dir, &st) == -1) {
		mkdir(dir, 0777);
	}
}

void writetofile(char message[], char path[]){
	FILE *f1;
	f1 = fopen(path,"a");
    fprintf(f1,"%s",message);
    fclose(f1);
}

void writetofilebaru(char db[]){
    char temp[3000];
    FILE *f1;
    f1 = fopen("/home/adr01/Documents/FP/database/USER/permission.txt","w");
    for(int b=0; b<=jumlah_permission; b++){
        memset(temp, 0, sizeof(temp));
        sprintf(temp, "%s : %s", client_permission[b].db, client_permission[b].id);
        if( strcmp(client_permission[b].db, db) == 0 || strcmp(client_permission[b].db, "permission db sudah terhapus") == 0){
            strcpy(client_permission[b].db, "permission db sudah terhapus");
            strcpy(client_permission[b].id, "permission user sudah terhapus");
        }
        else {
            f1 = fopen("/home/adr01/Documents/FP/database/USER/permission.txt","a");
            fprintf(f1,"%s",temp);
            fclose(f1);
        }
    }
}

// create socket, retrun fd (server)
int createServerSocket(struct sockaddr_in *address, int *addrlen) {
    int fd, opt = 1;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
    *addrlen = sizeof(*address);

    if (bind(fd, (struct sockaddr *)address, *addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return fd;
}


void* main_service(void *arg) {
    int socket = *(int *)arg;

    // cek ada client / tidak
    if(isClientExist) {
        strcpy(msg, "server_penuh");
        send(socket, msg, strlen(msg), 0);
        return NULL;
    } else {
        strcpy(msg, "server_kosong");
        send(socket, msg, strlen(msg), 0);
        isClientExist = true;
    }

    char req[100];
    char temp[3000];
    bool exit = false;

    while(!exit) {
        memset(req, 0, sizeof(req));
        read(socket, req, sizeof(req));

        if(strstr(req, "MATCH USER") != NULL){
            char *token = strtok(req, " ");
            int a = -1;
            bool flag = true, flag1 = true;
            char id[1000], password[1000];
            while( token != NULL ) {
                ++a;
                if(a == 2) strcpy(id, token);
                else if(a == 3) strcpy(password, token);
                token = strtok(NULL, " ");
            }

            if(strcmp(id, "root") == 0){
                send(socket, "Selamat datang root", strlen("Selamat datang root"), 0);
            }
            else {
                // Mencocokkan user apakah sudah ada
                for(int b=0; b<=jumlah_akun; b++){
                    if( strcmp(client_akun[b].id, id) == 0){
                        flag = false;
                    } 
                }

                if(flag) {
                    send(socket, "Maaf user belum terdaftar", strlen("Maaf user belum terdaftar"), 0);
                }
                else {
                    // Mencocokkan user dengan password
                    for(int b=0; b<=jumlah_akun; b++){
                        if( strcmp(client_akun[b].id, id) == 0 && strcmp(client_akun[b].pass, password) == 0){
                            flag1 = false;
                        } 
                    }
                    
                    if(flag1){
                        send(socket, "Maaf username dan password salah", strlen("Maaf username dan password salah"), 0);
                    }
                    else {
                        send(socket, "User berhasil login", strlen("User berhasil login"), 0);
                    }
                }
            }

        }

        else if(strstr(req, "CREATE USER") != NULL){
            char *token = strtok(req, " ");
            int a = -1;
            bool flag = true;
            char username[1000], password[1000];
            memset(username, 0, sizeof(username));
            memset(password, 0, sizeof(password));
   
            // Mengambil nama dan username
            while( token != NULL ) {
                ++a;

                if(a == 2) strcpy(username, token);
                else if(a == 5) strcpy(password, token);
                
                token = strtok(NULL, " ");
            }

            //Mengecek apakah sudah ada nama sebelumnya
            for(int b=0; b<=jumlah_akun; b++){
                if( strcmp(client_akun[b].id, username) == 0){
                    flag = false;
                } 
            }

            //Jika sudah ada 
            if(!flag){
                send(socket, "Maaf akun sudah ada", strlen("Maaf akun sudah ada"), 0);
            }
            //Menyimpan pada array dan file
            else {
                strcpy(client_akun[++jumlah_akun].id, username);
                strcpy(client_akun[jumlah_akun].pass, password);
                
                sprintf(temp, "%s : %s\n",client_akun[jumlah_akun].id, client_akun[jumlah_akun].pass);
                writetofile( temp, "/home/adr01/Documents/FP/database/USER/unamepass.txt");
                memset(temp, 0, sizeof(temp));

                send(socket, "Data berhasil ditambahkan", strlen("Data berhasil ditambahkan"), 0);
            }
        }

        else if(strstr(req, "GRANT PERMISSION") != NULL){
            char *token = strtok(req, " ");
            int a = -1;
            bool flag = true, flag1 = true;
            char db[1000], id[1000];
            memset(db, 0, sizeof(db));
            memset(id, 0, sizeof(id));
   
            // Mengambil nama dan username
            while( token != NULL ) {
                ++a;

                if(a == 2) strcpy(db, token);
                else if(a == 4) strcpy(id, token);
                
                token = strtok(NULL, " ");
            }

            //Mengecek apakah sudah ada permission sebelumnya
            for(int b=0; b<=jumlah_permission; b++){
                if( strcmp(client_permission[b].db, db) == 0 && strcmp(client_permission[b].id, id) == 0){
                    flag = false;
                } 
            }

            //Jika sudah ada 
            if(!flag){
                send(socket, "Maaf permission sudah ada", strlen("Maaf permission sudah ada"), 0);
            }
            
            else {
                // Mengecek apakah db nya sudah ada
                for(int b=0; b<=jumlah_db; b++){
                    if( strcmp(client_db[b], db) == 0){
                        flag = false;
                    } 
                }

                if(flag){
                    send(socket, "Maaf database belum ada", strlen("Maaf database belum ada"), 0);
                }                
                else {
                    // Mengecek apakah user nya sudah ada
                    for(int b=0; b<=jumlah_akun; b++){
                        if( strcmp(client_akun[b].id, id) == 0){
                            flag1 = false;
                        } 
                    }

                    if(flag1){
                        send(socket, "Maaf user belum ada", strlen("Maaf user belum ada"), 0);
                    } 

                    else {
                        //Menyimpan pada array dan file
                        strcpy(client_permission[++jumlah_permission].db, db);
                        strcpy(client_permission[jumlah_permission].id, id);

                        sprintf(temp, "%s : %s\n",client_permission[jumlah_permission].db, client_permission[jumlah_permission].id);
                        writetofile( temp, "/home/adr01/Documents/FP/database/USER/permission.txt");
                        memset(temp, 0, sizeof(temp));

                        send(socket, "Permission ditambahkan", strlen("Permission ditambahkan"), 0);
                    }
                }
            }
        }

        else if(strstr(req, "USE") != NULL){
            char *token = strtok(req, " ");
            int a = -1;
            bool flag = true, flag1 = true;
            char db[1000], id[1000];
            memset(db, 0, sizeof(db));
   
            // Mengambil db dan username
            while( token != NULL ) {
                ++a;
                if(a == 1) strcpy(db, token);
                else if(a == 2) strcpy(id, token);
                token = strtok(NULL, " ");
            }

            //Mengecek apakah db sudah ada
            for(int b=0; b<=jumlah_db; b++){
                if( strcmp(client_db[b], db) == 0){
                    flag = false;
                } 
            }

            if(flag){
                send(socket, "Maaf database masih belum ada", strlen("Maaf database masih belum ada"), 0);
            }

            else {
                // Mengecek apakah permission nya ada
                for(int b=0; b<=jumlah_permission; b++){
                    if( strcmp(client_permission[b].db, db) == 0 && strcmp(client_permission[b].id, id) == 0){
                        flag1 = false;
                    } 
                }

                if(flag1) {
                    send(socket, "Maaf tidak mempunyai permission", strlen("Maaf tidak mempunyai permission"), 0);
                }
                else {
                    strcpy(db_sekarang, db);
                    send(socket, "Database siap digunakan", strlen("Database siap digunakan"), 0);
                }
            }
        }

        else if(strstr(req, "DROP DATABASE") != NULL){
            char *token = strtok(req, " ");
            int a = -1;
            bool flag = true, flag1 = true;
            char db[1000], id[1000];
            memset(db, 0, sizeof(db));
            memset(id, 0, sizeof(id));
   
            // Mengambil db
            while( token != NULL ) {
                ++a;
                if(a == 2) strcpy(db, token);
                else if(a == 3) strcpy(id, token);
                token = strtok(NULL, " ");
            }

            // Mengecek apakah db ada
            for(int b=0; b<=jumlah_db; b++){
                if( strcmp(client_db[b], db) == 0){
                    flag = false;
                } 
            }

            if(flag){
                send(socket, "Maaf database belum ada", strlen("Maaf database belum ada"), 0);
            }
            else {
                // Mengecek apakah punya permission
                for(int b=0; b<=jumlah_permission; b++){
                    if( strcmp(client_permission[b].db, db) == 0 && strcmp(client_permission[b].id, id) == 0){
                        flag1 = false;
                    } 
                }

                if(flag1){
                    send(socket, "Maaf tidak mempunyai permission", strlen("Maaf tidak mempunyai permission"), 0);
                }
                else {
                    // Hapus semua file di dalamnya semua
                    sprintf(temp, "/home/adr01/Documents/FP/database/databases/%s",db);
                    struct dirent *dp;
   	                DIR *dir = opendir(temp);

                    while ((dp = readdir(dir)) != NULL) {
                        char pathFile[3000];
                        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                            if(dp->d_type == DT_REG) {
                                sprintf(pathFile, "/home/adr01/Documents/FP/database/databases/%s/%s",db,dp->d_name);
                                int hasil = remove(pathFile);
                                memset(pathFile, 0, sizeof(pathFile));
                            }
                        }
                    }

                    // Hapus database (rmdir dan array nya)
                    int status = rmdir(temp);
                    memset(temp, 0, sizeof(temp));


                    if(status == 0){
                        // Hapus permission di array dan file juga
                        writetofilebaru(db);
                        send(socket, "Hapus direktori sukses", strlen("Hapus direktori sukses"), 0);
                    }
                    else {
                        send(socket, "Hapus direktori gagal", strlen("Hapus direktori gagal"), 0);
                    }
                }
            }

        }

        else if(strstr(req, "CREATE TABLE") != NULL){
            char *token = strtok(req, " ");
            int a = -1, c = -1, jumlah_kolom = -1;
            bool flag = true;
            char nama_table[1000], kolom[1000], kolomcopy[1000], temp_lagi[3000];
            memset(nama_table, 0, sizeof(nama_table));
            memset(kolom, 0, sizeof(kolom));

            // Mengecek apakah db sudah ada
            if(strcmp(db_sekarang, "") == 0 || strlen(db_sekarang) == 0) {
                send(socket, "Tolong pilih database terlebih dahulu", strlen("Tolong pilih database terlebih dahulu"), 0);  
            }
            else {
                // Mengambil table 
                while( token != NULL ) {
                    ++a;
                    if(a == 2) strcpy(nama_table, token);
                    else if(a == 3) strcpy(kolom, token);
                    token = strtok(NULL, " ");
                }

                //Mengecek apakah file sudah ada
                for(int b=0; b<=jumlah_db; b++){
                    if( strcmp(client_file[b].db, db_sekarang) == 0 && strcmp(client_file[b].namaFile, nama_table) == 0){
                        flag = false;
                    } 
                }

                if(!flag){
                    send(socket, "Maaf file sudah ada sebelumnya", strlen("Maaf file sudah ada sebelumnya"), 0);
                }
                else {
                    for(int b = 0; b < strlen(kolom); b++){
                        if(kolom[b] != '(' &&  kolom[b] != ')'){
                            kolomcopy[++c] = kolom[b];
                        }
                    }
        
                    char *token2 = strtok(kolomcopy, ",");
                    ++jumlah_file;

                    // Mengambil atribut 
                    while( token != NULL ) {
                        ++jumlah_kolom;

                        char *token1 = strtok(token, " ");
                        for(int f = 0; f<2; f++){
                            if(f == 0) {
                                strcpy(client_file[jumlah_kolom].namakolom[jumlah_kolom], token1);
                                token1 = strtok(NULL, " ");
                            }
                            else strcpy(client_file[jumlah_kolom].tipedata[jumlah_kolom], token1); 
                        }
                        token2 = strtok(NULL, " ");
                    }
                    strcpy(client_file[jumlah_file].db, db_sekarang);
                    strcpy(client_file[jumlah_file].namaFile, nama_table);

                    //buat file nya
                    sprintf(temp_lagi, "/home/adr01/Documents/FP/database/databases/%s/%s.tsv",db_sekarang, nama_table);
                    FILE *f1;
                    f1 = fopen(temp_lagi,"a");
                    fclose(f1);  

                    memset(kolomcopy, 0, sizeof(kolomcopy));
                    memset(temp_lagi, 0, sizeof(temp_lagi));
                    send(socket, "Table berhasil terbuat", strlen("Table berhasil terbuat"), 0);  
                }
            }
        }

        else if(strstr(req, "DROP TABLE") != NULL){
            char *token = strtok(req, " ");
            int a = -1, c = -1, jumlah_kolom = -1;
            bool flag = true;
            char nama_table[1000];
            memset(nama_table, 0, sizeof(nama_table));

            // Mengecek apakah db sudah ada
            if(strcmp(db_sekarang, "") == 0 || strlen(db_sekarang) == 0) {
                send(socket, "Tolong pilih database terlebih dahulu", strlen("Tolong pilih database terlebih dahulu"), 0);  
            }
            else {
                // Mengambil table 
                while( token != NULL ) {
                    ++a;
                    if(a == 2) strcpy(nama_table, token);
                    token = strtok(NULL, " ");
                }

                //Mengecek apakah file sudah ada
                for(int b=0; b<=jumlah_db; b++){
                    if( strcmp(client_file[b].db, db_sekarang) == 0 && strcmp(client_file[b].namaFile, nama_table) == 0){
                        flag = false;
                    } 
                }

                if(flag){
                    send(socket, "Maaf table tidak ada", strlen("Maaf table tidak ada"), 0);
                }
                else {
                    // Menghapus file nya 
                    sprintf(temp, "/home/adr01/Documents/FP/database/databases/%s/%s.tsv", db_sekarang,nama_table);
                    int hasil = remove(temp);
                    memset(temp, 0, sizeof(temp));

                    if(hasil == 0){
                        // Menghapus array di file
                        for(int g = 0; g<jumlah_file; g++){
                            if(strcmp(client_file[g].db, db_sekarang) == 0 && strcmp(client_file[g].namaFile, nama_table) == 0){
                                strcpy(client_file[g].db, "Db sudah terhapus karena table kehapus");
                                strcpy(client_file[g].namaFile, "Table sudah terhapus karena table kehapus");
                            }
                        }
                        send(socket, "Table berhasil dihapus", strlen("Table berhasil dihapus"), 0);
                    }
                    else {
                        send(socket, "Table gagal terhapus", strlen("Table gagal terhapus"), 0);
                    }
                }
            }
        }

        else if(strstr(req, "DELETE FROM") != NULL){
            char *token = strtok(req, " ");
            int a = -1, c = -1, jumlah_kolom = -1;
            bool flag = true;
            char nama_table[1000];
            memset(nama_table, 0, sizeof(nama_table));

            // Mengecek apakah db sudah ada
            if(strcmp(db_sekarang, "") == 0 || strlen(db_sekarang) == 0) {
                send(socket, "Tolong pilih database terlebih dahulu", strlen("Tolong pilih database terlebih dahulu"), 0);  
            }
            else {
                // Mengambil table 
                while( token != NULL ) {
                    ++a;
                    if(a == 2) strcpy(nama_table, token);
                    token = strtok(NULL, " ");
                }

                //Mengecek apakah file sudah ada
                for(int b=0; b<=jumlah_db; b++){
                    if( strcmp(client_file[b].db, db_sekarang) == 0 && strcmp(client_file[b].namaFile, nama_table) == 0){
                        flag = false;
                    } 
                }

                if(flag){
                    send(socket, "Maaf table tidak ada", strlen("Maaf table tidak ada"), 0);
                }
                else {
                    // Menghapus isi file nya 
                    sprintf(temp, "/home/adr01/Documents/FP/database/databases/%s/%s.tsv", db_sekarang,nama_table);
                    
                    FILE *f1;
                    f1 = fopen(temp,"w");
                    fprintf(f1," ");
                    fclose(f1);
                    
                    memset(temp, 0, sizeof(temp));
                    send(socket, "Isi table berhasil dihapus", strlen("Isi table berhasil dihapus"), 0);

                }
            }
        }

        else if(strstr(req, "CREATE DATABASE") != NULL){
            char *token = strtok(req, " ");
            int a = -1;
            bool flag = true;
            char db[1000], id[1000], path[1000];
            memset(db, 0, sizeof(db));
            memset(id, 0, sizeof(id));
   
            // Mengambil database dan username
            while( token != NULL ) {
                ++a;
                if(a == 2) strcpy(db, token);
                else if(a == 3) strcpy(id, token);
                token = strtok(NULL, " ");
            }

            //Mengecek apakah db sudah ada
            for(int b=0; b<=jumlah_db; b++){
                if( strcmp(client_db[b], db) == 0){
                    flag = false;
                } 
            }

            //Jika sudah ada 
            if(!flag){
                send(socket, "Maaf database sudah ada", strlen("Maaf database sudah ada"), 0);
            }
            //Membuat database baru
            else {
                // Membuat database
                strcpy(client_db[++jumlah_db], db);
                strcpy(path, "/home/adr01/Documents/FP/database/databases/");
                strcat(path, db);
                createDir(path);

                // Memberi permission
                strcpy(client_permission[++jumlah_permission].db, db);
                strcpy(client_permission[jumlah_permission].id, id);

                sprintf(temp, "%s : %s\n",client_permission[jumlah_permission].db, client_permission[jumlah_permission].id);
                writetofile( temp, "/home/adr01/Documents/FP/database/USER/permission.txt");
                memset(temp, 0, sizeof(temp));


                if(strcmp(id, "root") != 0) {
                    strcpy(client_permission[++jumlah_permission].db, db);
                    strcpy(client_permission[jumlah_permission].id, "root");

                    sprintf(temp, "%s : %s\n",client_permission[jumlah_permission].db, client_permission[jumlah_permission].id);
                    writetofile( temp, "/home/adr01/Documents/FP/database/USER/permission.txt");
                }
                memset(temp, 0, sizeof(temp));
                send(socket, "Database ditambahkan", strlen("Database ditambahkan"), 0);
            }
        }

        else if(strcmp(req, "exit") == 0 || strcmp(req, "") == 0) {
            exit = true;
            isClientExist = false;
        }

    }

    return NULL;
}

int main() {
    // membuat dir USER dan DATAVASE

    getcwd(currPath, sizeof(currPath));
    int result = mkdir(strCt(currPath, "/USER"), 0777);
    int result2 = mkdir(strCt(currPath, "/databases"), 0777);
    
    
    strcpy(txtPath, currPath);
    strcat(txtPath, "/USER/unamepass.txt");
    
    // membuat unamepass.txt
    FILE *fp = fopen(txtPath, "a+");
    fclose(fp);

    // membuat file log
    fp = fopen("/home/adr01/Documents/FP/database/log.txt", "a+");

    // membuat permission.txt
    fp = fopen(strCt(currPath, "/USER/permission.txt"), "a+");
    fclose(fp);

    // membuat socket
    struct sockaddr_in address;
    int client_socket, addrlen;

    // menerima banyak client
    int server_fd = createServerSocket(&address, &addrlen);
    while((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0) {
        printf("client tersambung\n");

        // membuat thread
        pthread_t tid;
        int err = pthread_create(&(tid), NULL, &main_service, (void *)&client_socket);

        if(err != 0) printf("can't create thread : [%s]",strerror(err));
        else printf("create thread success\n");
    }

    return 0;
}
