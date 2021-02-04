#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <stdbool.h>
#include <string.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl; //descriptorul intors de accept
}thData;
static char *host ="localhost";
static char *user ="root";
static char *pass ="incredere1";
static char *db ="quizGame";

unsigned  int port = 3306;

static char *unix_socket = NULL;
unsigned int flag =0;
static MYSQL *con;
bool connected[100];
bool start = false;
int roluri[100];
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void continuare(void *);
void closeClient(void *);
int conect ();
void closeConn();
void startJoc(void *,int);
void printScore(void *);
void questionsQuery();
bool exist(char *, char *);
MYSQL_ROW rowAns;
MYSQL_RES *resQues;
int punctaj[100];
int max=0;
int min;
char questionAndAnsweres[100][100];
bool isStarted = false;

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    int nr;		//mesajul primit de trimis la client
    int sd;		//descriptorul de socket
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;


    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    conect();
    questionsQuery();
    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 2) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }

    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData * td; //parametru functia executata de thread
        int length = sizeof (from);

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
            perror ("[server]Eroare la accept().\n");
            continue;
        }

        td=(struct thData*)malloc(sizeof(struct thData));
        td->idThread=i++;
        td->cl=client;
        connected[td->idThread]=true;
        if(td->idThread > max){
            max = td->idThread;
        }
        if(td->idThread ==0){
            roluri[0]=1;
        }else{
            roluri[td->idThread] =0;
        }
        pthread_create(&th[i], NULL, &treat, td);

    }//while
    closeConn();
};
static void *treat(void * arg)
{
    struct thData tdL;
    tdL= *((struct thData*)arg);
    printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush (stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData*)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    return(NULL);

};


void raspunde(void *arg)
{
    int nr, i=0, rol;
    char name[100];
    char pass[100];
    int find;
    struct thData tdL;
    tdL= *((struct thData*)arg);
    if (read (tdL.cl, name,100) <= 0)
    {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");

    }
    if (read (tdL.cl, pass,100) <= 0)
    {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");

    }
    printf ("[Thread %d]Mesajul a fost receptionat...%s and %s\n",tdL.idThread, name, pass);
    if(exist(name,pass)){
      find =1;
    }else{
       find =0;
    }
    printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, find);

    int bytes;

    /* returnam mesajul clientului */
    if (write (tdL.cl, &rol, sizeof(int)) <= 0)
    {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write() catre client.\n");
    }
    else
        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);

    if (write (tdL.cl, &find, sizeof(int)) <= 0)
    {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write() catre client.\n");
    }
    else
        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
    continuare((struct thData*)arg);
}
void startJoc(void *arg, int i){
    int nr, rol;
    char name[100];
    char pass[100];
    int find=1;
    struct thData tdL;
    tdL= *((struct thData*)arg);
    int rand = tdL.idThread%3;
    write(tdL.cl,&rand,sizeof(int));
    write(tdL.cl,&max,sizeof(int));
    write(tdL.cl,&min,sizeof(int));
    while(i <= 24) {
        write(tdL.cl, questionAndAnsweres[i], 100);
        write(tdL.cl, questionAndAnsweres[i+1], 100);
        write(tdL.cl, questionAndAnsweres[i+2], 100);
        write(tdL.cl, questionAndAnsweres[i+3], 100);
        write(tdL.cl, questionAndAnsweres[i+4], 100);
        int rez;
        read(tdL.cl, &rez, sizeof(int));
        if (rez == 1) {
            punctaj[tdL.idThread] += 100;
            printf("Correct. the player with %d has %d points so far\n",tdL.idThread,punctaj[tdL.idThread]);
        } else if(rez == 5) {
            return(NULL);
        }else {
            printf("Incorrect\n");
        }
        i+=5;
    }
    sleep(10);
    printScore((struct thData*)arg);
}

void printScore(void *arg){

    struct thData tdL;
    tdL= *((struct thData*)arg);
    write(tdL.cl,punctaj,100);
}

int conect () {
    con = mysql_init(NULL);

    MYSQL_ROW row;
    if(mysql_real_connect(con, host,user,pass,db,port,unix_socket,flag)==NULL){
        fprintf(stderr,"%s\n",mysql_error(con));
        mysql_close(con);
        exit(1);
    }

}

void questionsQuery(){
    mysql_query(con,"SELECT * FROM questions");
    resQues = mysql_store_result(con);
    int i=0;
    while((rowAns = mysql_fetch_row(resQues))!=NULL){
        strcpy(questionAndAnsweres[i],rowAns[0]);
        strcpy(questionAndAnsweres[i+1],rowAns[1]);
        strcpy(questionAndAnsweres[i+2],rowAns[2]);
        strcpy(questionAndAnsweres[i+3],rowAns[3]);
        strcpy(questionAndAnsweres[i+4],rowAns[4]);
        i+=5;
    }
    mysql_free_result(resQues);
}

bool exist(char *name, char *password){
    MYSQL_RES *res;
    MYSQL_ROW row;
    mysql_query(con,"SELECT TRIM(username), TRIM(password) FROM users");
    res = mysql_store_result(con);
    while((row = mysql_fetch_row(res)) != NULL){
        if(strcmp(row[0],name) ==0 && strcmp(row[1],password)==0){
            mysql_free_result(res);
            return true;
        }
    }
    mysql_free_result(res);
    return false;

}
void continuare(void *arg){
    int nr, i=0, cont, rank, test;
    struct thData tdL;
    tdL= *((struct thData*)arg);
    if (read (tdL.cl, &cont,sizeof(int)) <= 0)
    {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");

    }
        write(tdL.cl,"You will start this game soon",100);
    while(isStarted == true){}
    if(cont == 1){
        if(tdL.idThread %3 == 0){
            min = tdL.idThread;
            rank =0;
        }else{
            rank =1;
        }
        if((tdL.idThread+1) %3 == 0){
            isStarted = true;
        }

        while(isStarted == false){}
        write(tdL.cl,"The game is now started",100);
        isStarted = false;
        startJoc((struct thData*)arg,0);
    }else{
        printf("Nu continua\n");
    }
    closeClient((struct thData*)arg);
}

void closeConn(){
    mysql_close(con);
}

void closeClient(void *arg){
    close ((intptr_t)arg);
}
