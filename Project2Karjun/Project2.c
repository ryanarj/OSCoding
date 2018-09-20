/*
Author: Karshan Arjun
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHMKEY ((key_t) 5000)  
/* change the key number */
#define SHMKEY ((key_t) 7890)
#define SEMKEY ((key_t) 400L)   //Was 4500
#define NSEMS 1 // Number of semaphores

typedef struct {
     int data; 
} shared_mem;
shared_mem *process_total;

int sem_id;

typedef union
{
    int val;
    struct semid_ds *buf;
    ushort *array;
} semunion;


static struct sembuf owait = {0, -1, 0};
static struct sembuf osignal = {0, 1, 0};

struct sembuf *wait_buffer = &owait;
struct sembuf *signal_buffer = &osignal;

int semwait()
{
    int status;
    status = semop(sem_id, wait_buffer, 1);
    return status;
}


int semsignal()
{
    int status;
    status = semop(sem_id, signal_buffer, 1);
    return status;
}



void process_1(){
    int maxValue = 100000;
    int i = 0;
    while(i<maxValue){ 
        semwait();
        if(process_total->data < 600000){
            process_total->data +=  1;
        } 
        semsignal();
        i+=1;
    }
    printf ("\nFrom Process 1: counter = : %d\n", process_total->data);
}
void process_2(){
    int maxValue = 200000;
    int i = 0;
    while(i<maxValue){ 
        semwait();
        if(process_total->data < 600000){
            process_total->data +=  1;
        } 
        semsignal();
        i+=1;
    }
    printf ("\nFrom Process 2: counter = : %d\n", process_total->data);
}
void process_3(){
    int maxValue = 300000;
    int i = 0;
    while(i<maxValue){ 
        semwait();
        if(process_total->data < 600000){
            process_total->data +=  1;
        } 
        semsignal();
        i+=1;
    }
    printf ("\nFrom Process 3: counter = : %d\n", process_total->data);
}
void process_4(){
    int maxValue = 500000;
    int i = 0;
    while(i<maxValue){ 
        semwait();
        if(process_total->data < 600000){
            process_total->data +=  1;
        } 
        semsignal();
        i+=1;
    }
    printf ("\nFrom Process 4: counter = : %d\n", process_total->data);
}
int main(void){ 
    int pids[4] = {0, 0, 0, 0};
    int id;
    int shm_id;
    char *shmadd;
    shmadd = (char *) 0;
    char *shmadd;
    shmadd = (char *) 0;

/* Variables for Semaphores */
    int numsem = 0;
    int val1;
    int val2;
    semunion semctl_arg;
    semctl_arg.val = 1;

/* Semaphore Creation */
    sem_id = semget(SEMKEY, NSEMS, IPC_CREAT | 0666);
    if(sem_id < 0) printf("Error when creating semaphore.\n");

/* Semaphore Initialization*/
    val2 = semctl(sem_id, numsem, SETVAL, semctl_arg);
    val1 = semctl(sem_id, numsem, GETVAL, semctl_arg);
    if (val1 < 1) printf("Error detected with SETVAL.\n");

    // Get shared memory and also place it at the struct
    if ((shm_id = shmget(SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(1);
    }
    if ((process_total = (shared_mem *) shmat(shm_id, shmadd, 0)) == (shared_mem*) - 1){
        perror("shmat");
        exit(0);
    }
    process_total->data = 0;

    // Create all the processes.
	if ((pids[0] = fork()) == 0){ process_1();}
    if ((pids[0] != 0) && (pids[1] = fork()) == 0){ process_2();}
    if ((pids[0] != 0) && (pids[1] != 0) && (pids[2] = fork()) == 0){ process_3();}                                      
    if ((pids[0] != 0) && (pids[1] != 0) && (pids[2] != 0) &&  (pids[3] = fork()) == 0){ process_4();}          

    // Wait on each prcess to finish
    int i = 0;
    while((id = waitpid(pids[i], NULL, 0)) != -1){
        printf("Child with ID#%d has finished.\n", id);
        i++;
    }

    // Once all  the processes are done release shared memory
	if ((pids[0] != 0) && (pids[1] != 0) && (pids[2] != 0) && (pids[3] != 0)){
        if ((shmctl (shm_id, IPC_RMID, (struct shmid_ds *) 0)) == -1){
            perror("shmctl");
            exit(-1);
        }
        printf("\nEnd of Simulation\n\n");   

        /* Semaphore Deallocation*/
        semctl_arg.val = 0;
        status = semctl(sem_id, 0, IPC_RMID, semctl_arg);
        if(status < 0) printf("Error when deallocating semaphore.\n");

    }
    return 0;
}