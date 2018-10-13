/*
Author: Karshan Arjun
*/
#define _REENTRANT
#define BUFFER_SIZE 150
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define SHMKEY ((key_t) 7890)

typedef struct{ 
	char* value; 
} bufferStruct;
typedef struct{ 
	int value; 
} counterStruct;

// Globale variables
int start = 0;
int done = 0;
char charItem;
FILE* file;
sem_t emptyItems;
sem_t fullItems;
sem_t criticalItems;
bufferStruct *buffer;
counterStruct *counter;

// Used to produce the characters
void* producer(void *item){
	int index = 0;
    bool finished = false;

 	// Loop til the finshed
    while(!finished){
        sem_wait(&emptyItems);
        sem_wait(&criticalItems);
        done++;
        index = (done) % BUFFER_SIZE;
        // Critical section in Producing
        // Go into the file and then get the character
        if(fscanf(file,"%c", &charItem) != EOF){
            buffer->value[index] = charItem;
            printf("In the Producing: %c\n", charItem);
        }else{
            buffer->value[index] = '*';
            finished = true;
        }

        // Post the semaphore
        sem_post(&criticalItems);
        sem_post(&fullItems);
    }
}

// Used to consume the characters
void* consumer(void *item){
    bool finished = false;
    char v;
    int index = 0;

    // Keep consuming 
    while(!finished){
        sem_wait(&fullItems);
        sem_wait(&criticalItems);
        start++;
        index = (start) % BUFFER_SIZE;
        sleep(1);

        // Critical section in consumming
        if((v = buffer->value[index]) != '*'){
            printf("In the Consuming: %c\n", v);
            counter->value++;
        }else{
            finished = true;
        }

        // Post the semaphore
        sem_post(&criticalItems);
        sem_post(&emptyItems);
    }
}

int main(void){
   	// Thread variables
    pthread_t pro[1];      
    pthread_t con[1];      
    pthread_attr_t at;
    int i;
    int sm_id;                          
    char *shmadd;
    shmadd = (char *) 0;
    file = fopen("mytest.dat", "r");

    // Get the shared memory
    if ((sm_id = shmget (SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0){
        perror ("shmget");
        return 1;
    }

    // Get the buffer 
    if ((buffer = (bufferStruct *) shmat (sm_id, shmadd, 0)) == (bufferStruct *) -1){
        perror ("shmat");
        return 0;
    }

   	// Allocate the charater array that will hold the values
    char buff[BUFFER_SIZE];
    buffer->value = buff;

    // Allocate the memory
    counter = (counterStruct *) malloc(sizeof(counterStruct));
    counter->value = 0;

    // Initialze the variable to the buffersize 
    sem_init(&emptyItems, 0, BUFFER_SIZE);
    sem_init(&fullItems, 0, 0);
    sem_init(&criticalItems, 0, 1);
    fflush(stdout);

    // Init the attr, set teh scope and then join the pthread
    pthread_attr_init(&at);
    pthread_attr_setscope(&at, PTHREAD_SCOPE_SYSTEM); 
    pthread_create(&pro[0], &at, producer, 0);
    pthread_create(&con[0], &at, consumer, 0);
    pthread_join(pro[0], 0);
    pthread_join(con[0], 0);

    //Remove the memory of the sem t
    sem_destroy(&emptyItems);
    sem_destroy(&fullItems);
    sem_destroy(&criticalItems);

    // If a problem with 
    if ((shmctl (sm_id, IPC_RMID, (struct shmid_ds *) 0)) == -1){
        perror ("shmctl");
        return -1;
    }

	// Close out file and out the results
    fclose(file);
    printf("Counter: %d\n\n", counter->value);
    printf("End of Simulation\n");
    printf("\n\n");
    return 0;
}

