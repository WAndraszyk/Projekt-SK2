#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LENGTH 2048
#define NAME_LEN 32

bool flag = 0;
int sockfd = 0;
char name[NAME_LEN];

void str_overwrite_stdout() {
  printf("> ");
  fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;

  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void *send_msg_handler(void*){
    char message[LENGTH] = {};
    char buffer[LENGTH + NAME_LEN] = {};

    while(1){
        str_overwrite_stdout();
        fgets(message, LENGTH, stdin);
        str_trim_lf(message, LENGTH);

        if (strcmp(message, "exit") == 0){
            break;
        }
        else{
            sprintf(buffer, "%s: %s\n", name, message);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        memset(message, 0, LENGTH);
        memset(buffer, 0, LENGTH + NAME_LENGTH);
    }
    catch_ctrl_c_and_exit(2);
}

void *recv_msg_handler(void*){
    char message[LENGTH] = {};
    int receive;

    while (1){
        receive = recv(sockfd, message, LENGTH, 0);

        if (receive > 0){
            printf("%s", message);
            str_overwrite_stdout();
        }
        else if (receive == 0){
            break;
        }
        memset(message, 0, sizeof(message));
    }
}

int main(int argc, char **argv){
    if(argc != 3){
        printf("Sposob: %s <adres IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* ip = atos(argv[1]);
    int port = atoi(argv[2]);

    signal(SIGINT, catch_ctrl_c_and_exit);

    printf("Podaj nazwe uzytkownika: ");
    fgets(name, NAME_LENGTH, stdin);
    str_trim_lf(name, strlen(name));


    if (strlen(name) > NAME_LENGTH || strlen(name) < 2){
        printf("Nazwa musi zawierac pomiedzy 2, a %d znakow.\n", NAME_LENGTH);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    /* Ustawianie Socketa */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);


    // Laczenie ze serwerem
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    // Przesylamy nazwe uzytkownika serwerowi
    send(sockfd, name, NAME_LENGTH, 0);

    printf("=== WITAJ W POKOJU ===\n");

    pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread, NULL, &send_msg_handler, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread, NULL, &recv_msg_handler, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    while (1){
        if(flag){
            printf("\nDo zobaczenia!\n");
            break;
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}
