#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <atomic>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define NAME_LEN 32

using namespace std;

static atomic_uint cli_count(0);
static int uid = 10;

// Struktura klienta
typedef struct{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[NAME_LEN];
} client_t;

client_t* clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_overwrite_stdout(){
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length){
    for(int i=0; i<length; i++){
        if(arr[i] == '\n'){
            arr[i] = '\0';
            break;
        }
    }
}

void send_rem(char name[]){
    pthread_mutex_lock(&clients_mutex);
    char command[NAME_LEN + 16];

    for(int i = 0; i<MAX_CLIENTS; i++){
        if(!clients[i]) continue;
        else{
            sprintf(command, "/USERS remove %s\n", name);
            write(clients[i]->sockfd, command, strlen(command));
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_user_names(int sockfd){         //wysyla nazwy wszystkich uzytkownikow jeden po drugim
    pthread_mutex_lock(&clients_mutex);
    char usernames[NAME_LEN + 16];
    char name[NAME_LEN];

    for(int i=0; i<MAX_CLIENTS; i++){
        if(!clients[i]){
            continue;
        }
        else{
            if(sockfd == clients[i]->sockfd) {
                sprintf(name, "%s", clients[i]->name);
                continue;
            }
            sprintf(usernames, "/USERS add %s\n", clients[i]->name);
            write(sockfd, usernames, strlen(usernames));
        }
    }
    for(int i=0; i<MAX_CLIENTS; i++){
        if(!clients[i]){
            continue;
        }
        else{
            sprintf(usernames, "/USERS add %s\n", name);
            write(clients[i]->sockfd, usernames, strlen(usernames));
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_add(client_t* cl){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; i++){
        if(!clients[i]){
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients[i]){
            if(clients[i]->uid == uid){
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void print_ip_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d", addr.sin_addr.s_addr & 0xff, (addr.sin_addr.s_addr & 0xff00) >> 8, (addr.sin_addr.s_addr & 0xff0000) >> 16, (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void send_message(char* s){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]){
            if(write(clients[i]->sockfd, s, strlen(s)) < 0){
                printf("ERROR: blad wysylania wiadomosci\n");
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// Obsługa klientów
void* handle_client(void* arg){
    char buffer[BUFFER_SZ];
    char name[NAME_LEN];
    char sendname[NAME_LEN+16];
    int leave_flag = 0;
    cli_count++;

    client_t* cli = (client_t*)arg;

    //Nazwa klienta
    if(recv(cli->sockfd, name, NAME_LEN, 0) <= 0 || strlen(name) < 2 || strlen(name) >= NAME_LEN - 1){
        printf("Wprowadz poprawna nazwe\n");
        leave_flag = 1;
    }
    else{
        strcpy(cli->name, name);
        sprintf(buffer, "%s dolaczyl do serwera\n", cli->name);
        printf("%s", buffer);

        send_message(buffer);

        send_user_names(cli->sockfd);
    }

    bzero(buffer, BUFFER_SZ);

    while(1){
        if(leave_flag){
            break;
        }

        int receive = recv(cli->sockfd, buffer, BUFFER_SZ, 0);

        if(receive > 0){
            if(strlen(buffer) > 0){
                send_message(buffer);
                str_trim_lf(buffer, strlen(buffer));
                printf("%s -> %s", buffer, cli->name);
            }
        }
        else if(receive = 0 || strcmp(buffer, "exit") == 0){
            sprintf(buffer, "%s oposcil serwer\n", cli->name);
            printf("%s", buffer);
            send_message(buffer);
            leave_flag = 1;
        }
        else{
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buffer, BUFFER_SZ);
    }

    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    cli_count--;

    send_rem(name);

    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv){
    if(argc != 3){
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char* ip = argv[1];
    int port = atoi(argv[2]);

    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    //Ustawienia socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    //Signal
    signal(SIGPIPE, SIG_IGN);

    if(setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0){
        printf("ERROR: setsockopt\n");
        return EXIT_FAILURE;
    }

    //Bind
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) <0 ){
        printf("ERROR: bind\n");
        return EXIT_FAILURE;
    }

    //Listen
    if(listen(listenfd, 10) < 0){
        printf("ERROR: listen\n");
        return EXIT_FAILURE;
    }

    printf("------- WITAJ NA SERWERZE --------\n");

    //Główna pętla
    while(1){
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        //Sprawdzenie limitu klientów
        if((cli_count + 1) == MAX_CLIENTS){
            printf("Osiągnięto limit. Połączenie odrzucone.\n");
            print_ip_addr(cli_addr);
            close(connfd);
            continue;
        }

        //Ustawienia klienta
        client_t* cli = (client_t*)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

        //Dodawanie klienta
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        sleep(1);
    }

    return EXIT_SUCCESS;
}

