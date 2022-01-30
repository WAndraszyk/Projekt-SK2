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
char* ownername;

// Struktura klienta
typedef struct{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[NAME_LEN];
    bool owner;
    bool kicked = false;
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
            send(clients[i]->sockfd, command, strlen(command), MSG_NOSIGNAL);
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
            send(sockfd, usernames, strlen(usernames), MSG_NOSIGNAL);
        }
    }
    for(int i=0; i<MAX_CLIENTS; i++){
        if(!clients[i]){
            continue;
        }
        else{
            sprintf(usernames, "/USERS add %s\n", name);
            send(clients[i]->sockfd, usernames, strlen(usernames), MSG_NOSIGNAL);
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

bool checkIfNameTaken(char name[NAME_LEN]){
    for(int i = 0; i<MAX_CLIENTS; i++){
        if(clients[i]){
            printf("%s", clients[i]->name);
            if(strcmp(name, clients[i]->name) == 0){
                return true;            
            }
        }
    }
    return false;
}

void send_message(char* s){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]){
            if(send(clients[i]->sockfd, s, strlen(s), MSG_NOSIGNAL) < 0){
                printf("ERROR: blad wysylania wiadomosci\n");
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void kick_user(char name[NAME_LEN]){
    pthread_mutex_lock(&clients_mutex);
    int index = 0;

    for(int i=0; i<MAX_CLIENTS; i++){
        if(!clients[i]){
            continue;
        }
        else if(strcmp(clients[i]->name, name) == 0){
            if(clients[i]->owner) return;
            clients[i]->kicked = true;

            index = i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    char buffer[BUFFER_SZ];
    sprintf(buffer, "/kick %s\n", name);
    printf("%s", buffer);
    send_message(buffer);

    close(clients[index]->sockfd);
    queue_remove(clients[index]->uid);

    free(clients[index]);
    cli_count--;

    send_rem(name);
}

void leave(char name[NAME_LEN]){
    pthread_mutex_lock(&clients_mutex);
    int index = 0;

    for(int i=0; i<MAX_CLIENTS; i++){
        if(!clients[i]){
            continue;
        }
        else if(strcmp(clients[i]->name, name) == 0){
            if(clients[i]->owner) return;
            clients[i]->kicked = true;

            index = i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    char buffer[BUFFER_SZ];
    sprintf(buffer, "/left %s\n", name);
    printf("%s", buffer);
    send_message(buffer);

    close(clients[index]->sockfd);
    queue_remove(clients[index]->uid);

    free(clients[index]);
    cli_count--;

    send_rem(name);
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
        if(checkIfNameTaken(name)){;
            sprintf(buffer, "/nametaken\n");
            printf("%s", buffer);

            pthread_mutex_lock(&clients_mutex);
            send(cli->sockfd, buffer, BUFFER_SZ, MSG_NOSIGNAL);
            pthread_mutex_unlock(&clients_mutex);

            leave_flag = 1; 

            close(cli->sockfd);
            queue_remove(cli->uid);
            free(cli);
            cli_count--;

            pthread_detach(pthread_self());

            return NULL;
        }
        else {
            sprintf(buffer, "/ok\n");
            printf("%s", buffer);
            send(cli->sockfd, buffer, BUFFER_SZ, MSG_NOSIGNAL);
        }

        strcpy(cli->name, name);
        sprintf(buffer, "/join %s\n", cli->name);
        printf("%s", buffer);

        send_message(buffer);

        if(strcmp(name, ownername) == 0) cli->owner = true;
        else {
            cli->owner = false;
        }

        send_user_names(cli->sockfd);
        
        sprintf(buffer, "/owner %s\n", ownername);

        send(cli->sockfd, buffer, BUFFER_SZ, MSG_NOSIGNAL);
    }

    bzero(buffer, BUFFER_SZ);

    while(1){
        if(leave_flag){
            break;
        }

        if(cli->kicked == true) break;
        int receive = recv(cli->sockfd, buffer, BUFFER_SZ, 0);

        if(receive > 0 && cli->kicked == false){
            if(strlen(buffer) > 0){
                if(strncmp(buffer, "/kick ", 6) == 0 && cli->owner == true){
                    char kickname[NAME_LEN] = {};
                    sscanf(buffer, "/kick %s", kickname);
                    if(strcmp(kickname, ownername) == 0) continue;
                    else{
                        kick_user(kickname);
                        continue;
                    }
                }
                else if(strncmp(buffer, "/leaving", 8) == 0){
                    printf("%s", buffer);
                    leave_flag = 1;
                    break;
                }
                send_message(buffer);
                str_trim_lf(buffer, strlen(buffer));
                printf("%s -> %s", buffer, cli->name);
            }
        }
        else{
            leave_flag = 1;
        }

        bzero(buffer, BUFFER_SZ);
    }

    if(cli->owner){
        sprintf(buffer, "Wlasciciel opuscil serwer!\n \nPokoj nieczynny.\n");
        printf("%s", buffer);

        sprintf(buffer, "/left %s\n", cli->name);
        printf("%s", buffer);
        send_message(buffer);

        pthread_detach(pthread_self());
        std::exit(EXIT_SUCCESS);
    }

    leave(cli->name);

    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv){
    if(argc != 4){
        printf("Usage: %s <ip> <port> <nazwa_wlasciciela>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char* ip = argv[1];
    int port = atoi(argv[2]);
    ownername = argv[3];

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

