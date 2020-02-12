//
//                                 Dinning Philosophers Problem!
//                                          prouczta.cpp
//  
//
//

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define L_FILOZOF 5 //ilu jest filozofów
#define ID_FILOZOFA id_filozofa
#define LEWA_STRONA (id_filozofa==0) ? L_FILOZOF-1 : id_filozofa-1
#define PRAWA_STRONA (id_filozofa+1)%L_FILOZOF

//stany w jakich moze znajdowac sie filozof
#define MYSLI 0
#define GLODNY 1
#define SPOZYWA 2

//inicjalizacja semaforow
int semid;
void sem_init (int semnum, int value) {semctl(semid, semnum,SETVAL,value);}

static struct sembuf sops;
void podnies(int semid, int semnum) {
    sops.sem_num = semnum;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    if (semop(semid, &sops, 1) == -1)
    {
        perror("Podnoszenie semafora nieudane\n");
        exit(1);
    }
}
void opusc(int semid, int semnum) {
    sops.sem_num = semnum;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    if (semop(semid, &sops, 1) == -1)
    {
        perror("Opuszczenie semafora nieudane \n");
        exit(1);
    }
}

// przydatne zmienne
volatile int ile_zjadl = 0;
int id_filozofa;
char* status;


//wyswietlanie ile posilkow zjadl i ktory filozof
void info() {printf ("Filozof %d, zjadł %d posiłków.\n", id_filozofa, ile_zjadl);}
void spozyj(int f) {
    if (status[f] == GLODNY && status[LEWA_STRONA] != SPOZYWA && status[PRAWA_STRONA] != SPOZYWA) {
        status[f] = SPOZYWA;
        printf("Filozof %d szama zarelko\n", id_filozofa); //zmiana
        ile_zjadl++; //zmiana
        int sleep_time; //zmiana
        sleep_time = (id_filozofa+1); //zmiana
        sleep(sleep_time); //zmiana
    }
    podnies(semid, f);
}

//przechwytywanie forka przez filozofa
void zlap_widelec() {
    opusc(semid, L_FILOZOF);
    status[id_filozofa] = GLODNY;
    printf("Filozof %d zglodanial\n", id_filozofa);
    spozyj(id_filozofa);
    podnies(semid, L_FILOZOF);
    opusc(semid, id_filozofa);
}

void odloz_widelec() {
    opusc(semid, L_FILOZOF);
    status[id_filozofa] = MYSLI;
    podnies(semid, L_FILOZOF);
}

void szamaj() {
    //signal(SIGINT, info());
    info();
    printf("Filozof %d\n", id_filozofa);
    while (true) {
        info();
        printf("Filozof %d myśli nad życiem\n", id_filozofa);
        zlap_widelec();
        odloz_widelec();
    }
}

//funkcja czekaj (funkcja rodzica), podczas forka tworzy nam sie 10 procesow, ale musimy 5 z nich wstrzymac, zeby bylo tyle procesow ilu filozofow.
void czekaj(){
    wait(NULL);
}

int main (){
    
//Tworzenie pamięci wspoldzielonej
    int shmid;
    shmid = shmget (IPC_PRIVATE, L_FILOZOF, IPC_CREAT | 0600);
    status = (char*)shmat(shmid, NULL, 0);
    
    if ((long)shmid == -1){printf("Niepowodzenie przy tworzeniu shared memory\n"); exit(1);}
    if ((long)shmat == -1){printf("Niepowodzenie przy tworzeniu shared memory\n"); exit(1);}
    
    
//Tworzymy przestrzeń dla semaforkow
    semid = semget(IPC_PRIVATE, L_FILOZOF+1, IPC_CREAT | 0600);
    if (semid == -1){printf("Niepowodzenie przy tworzeniu przestrzeni dla semaforkow\n"); exit(1);}
    
//inicjalizacja semaforkow
    int i;
    for (i=0; i<L_FILOZOF; i++) {sem_init(i,0);} sem_init(i,1);
    
    
//Ustawiamy wszystkich Filozofow na status myslenie i wartosc ile_zjadl na 0
    for (i=0; i<L_FILOZOF; i++) {status[i] = MYSLI;}
    for (i=0; i<L_FILOZOF; i++) {ile_zjadl = 0;}
    
//dzielimy procesy
    int pid;
    for (id_filozofa=0; id_filozofa < L_FILOZOF; id_filozofa++) {
        pid=fork();
        if (pid ==0) break;
    }
    
    if (pid == 0) {szamaj();}
    if (pid < 0) {printf("Jestes ciota sie nie udalo\n"); exit(1);}
    else {czekaj();}
    
    
    
    return 0;
}
