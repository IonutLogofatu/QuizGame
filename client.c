#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>


/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

void startJoc(int);

int main (int argc, char *argv[])
{
    int sd;			// descriptorul de socket
    struct sockaddr_in server;	// structura folosita pentru conectare
    // mesajul trimis
    int nr=0;
    char buf[10];
    char user[100];
    char pass[100];
    int rasp;
    int rol;
    int find;

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi (argv[2]);

    /* cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons (port);

    /* ne conectam la server */
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    /* citirea mesajului */
    printf ("[client]Enter the username:  ");
    fflush (stdout);
    read (0, &user, sizeof(user));
    //scanf("%d",&nr);
    strtok(user, "\n");

    /* trimiterea mesajului la server */
    if (write (sd,user,sizeof(user)) <= 0)
    {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }

    printf ("[client]Enter the password:  ");
    fflush (stdout);
    read (0, &pass, sizeof(pass));
    strtok(pass, "\n");

    /* trimiterea mesajului la server */
    if (write (sd,pass,sizeof(pass)) <= 0) {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
    }

    if (read (sd, &rol, sizeof(int)) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    if (read (sd, &find,sizeof(int)) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    char msg[100];
    int rank;
    int test;
    if(find ==1){
        printf ("[client]Doriti sa continuati?\n 'Da' 1 \n 'Nu' 2 \n Raspunsul: ");
        fflush (stdout);
        scanf("%d",&rasp);
        printf("%d\n",rasp);
        if(rasp ==1){
            if (write (sd,&rasp,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
            if(rank == 0){
                read(sd,msg,100);
                printf("%s\n",msg);
                read(sd,msg,100);
                printf("%s\n",msg);
                startJoc(sd);
            }else{
                printf("Wait to be 3 players\n");
                read(sd,msg,100);
                printf("%s\n",msg);
                read(sd,msg,100);
                printf("%s\n",msg);
                startJoc(sd);
            }
        }else{
            if (write (sd,&rasp,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
        }
    }else{
        rasp = 2;
        if (write (sd,&rasp,sizeof(int)) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }
    }
    /* inchidem conexiunea, am terminat */
    close (sd);
}

void sort(int arr[100],int id[100],int n, int m){
    for(int i =m; i < n; i++){
        for(int j = i+1; j < n; j++){
            if(arr[i]<arr[j]){
                int tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
                tmp = id[j];
                id[j] = id[i];
                id[i] = tmp;
            }
        }
    }
}

void printScore(int sd, int max,int min){

    int score[100], id[100];
    read(sd,score,100);
    for(int i = min; i<=max; i++){
        id[i] = i;
    }
    sort(score,id,max+1,min);
    for(int i =min; i <= max; i++){
        printf("Place %d, with id %d obtained %d points\n",(i%3)+1,id[i],score[i]);
    }
}


void startJoc(int sd){
    struct timeval timeout;
    int rv;
    fd_set readfds;
    char intrebare[100];
    char raspuns1[100];
    char raspuns2[100];
    char raspuns3[100];
    char raspuns4[100];
    int randul;
    int answer;
    char raspunsuri[5][100];
    read(sd,&randul,sizeof(int));
    int max,min;
    read(sd,&max,sizeof(int));
    read(sd,&min,sizeof(int));
    printf("\nYou will answer the question in %d seconds\n\n\n\n\n", randul*10);
    int seconds = randul * 10;
    int i =0;
    srand(time(NULL));
    while(i < 5) {
        FD_ZERO( &readfds );
        FD_SET( STDIN_FILENO, &readfds );
        timeout.tv_sec = 10;
        timeout.tv_usec = 10000;
        while (seconds != 0) {
            printf("%d\n", seconds);
            seconds--;
            sleep(1);
        }
        seconds = randul *10;
        printf("\e[1;1H\e[2J");
        printf("Now you can answer at this question. You have 10 sec to answer\n\n\n\n");

        read(sd, intrebare, 100);
        read(sd, raspuns1, 100);
        read(sd, raspuns2, 100);
        read(sd, raspuns3, 100);
        read(sd, raspuns4, 100);
        strcpy(raspunsuri[0],raspuns1);
        strcpy(raspunsuri[1],raspuns2);
        strcpy(raspunsuri[2],raspuns3);
        strcpy(raspunsuri[3],raspuns4);
        char raspunsuriNoi[4][100];
        printf("\n%s\n", intrebare);
        int l =0;
        bool checked[4];
        while(l <4){
            int j;
            j = rand()%(4-0);

            if(checked[j]==false){
                printf("%d:   %s       ",l+1,raspunsuri[j]);
                strcpy(raspunsuriNoi[l],raspunsuri[j]);
                checked[j]=true;
                l++;
            }
        }
        for(int k = 0; k< 4; k++){
            checked[k] = false;
        }
        char buf[100];
        printf("\n\n\n");
        int com = select (sd+1, &readfds, NULL, NULL, &timeout);
        if(com == -1){
            return(NULL);
        }else if(com == 0){
            answer = 6;
        }else{
            read(0,buf,100);
        }
        strtok(buf,"\n");
        if(strcmp(buf,"quit")==0){
            answer = 5;
        }else {
            answer = atoi(buf) - 1;
            if (strcmp(raspunsuriNoi[answer], raspunsuri[0]) == 0) {
                answer = 1;
                printf("\n\nCorrect answer\n\n");
            }else{
                printf("\n\nIncorrect answer\n\n");
            }
        }
        write(sd, &answer, sizeof(int));
        if(answer == 5){
            return(NULL);
        }
        int secRemain= ((max%3)-randul)*10;
        printf("You will move on to the next question in %d seconds\n\n\n\n\n", secRemain);
        while (secRemain != 0) {
            printf("%d\n", secRemain);
            secRemain--;
            sleep(1);
        }
        i++;
    }
    seconds = 20;
    printf("The game is over. The results will be displayed in %d seconds\n\n\n\n\n", seconds);
    while (seconds != 0) {
        printf("%d\n", seconds);
        seconds--;
        sleep(1);
    }
    printScore(sd, max,min);
}
