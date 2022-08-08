/**
* Satoru Yamamoto
* run command: gcc -std=c11 main.c -o main -pthread
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<pthread.h>
#include<semaphore.h>
#include <unistd.h>


void printUsage();
void producerConsumer();
void *producer(void *arg);
void *consumer(void *arg);


void diningPhilosopher();
void* philosopher(void *arg);
void think_eat(int id, int mode);
void test(int id);
void take_fork(int id);
void put_fork(int id);


void potionBrewers();

void *master(void *arg);

void *agentA(void *arg);
void *agentB(void *arg);
void *agentC(void *arg);

void *brewerA(void *arg);
void *brewerB(void *arg);
void *brewerC(void *arg);

void *pusherA(void *arg);
void *pusherB(void *arg);
void *pusherC(void *arg);


#define BUFFSIZE 5
#define ITEM 5

#define PHILOSOPHER 5
#define EAT 1             // # of eat to complete for each philosopher
//https://en.wikipedia.org/wiki/Dining_philosophers_problem
#define RIGHT (id+PHILOSOPHER-1)%PHILOSOPHER
#define LEFT (id+1)%PHILOSOPHER

#define BREWERS 3
#define MASTER 1
#define TASK 3
#define POSION 10       // total potion to create

pthread_mutex_t pusherMutex, brewerMutex;
int num_potion = 0;
int ingredients[TASK];

sem_t semAgent;        // agent semaphore
sem_t bezoars;
sem_t horns;
sem_t berry;
sem_t semBezoars;
sem_t semHorns;
sem_t semBerry;


int num_item = 0;
int num_producer = 0;
int num_consumer = 0;

int state[PHILOSOPHER];   // state for each philosopher
sem_t mutex;              // table mutex
sem_t phil_mutex[PHILOSOPHER];       // semaphore per philosopher

enum {
  THINKING=0,             // philosopher is thinking
  HUNGRY=1,               // philosopher is trying to get forks
  EATING=2                // philosopher is eating
};



// https://docs.oracle.com/cd/E19455-01/806-2732/6jbu8v6os/index.html
typedef struct {
    int buffer[BUFFSIZE];
    sem_t empty;            // # of empty buffer slots
    sem_t occupied;         // # of items in buffer
    pthread_mutex_t mutex;  // mutual exclution for critical section
    int nextin;             // next index to put data
    int nextout;            // next index to consume data
} buffer_t;
buffer_t b_inf;





int main(int argc, char * argv[])
{
    int isCorrectArg = 1;                       // 0 if command line argument is invalid
    // mode 1: producer/consumer
    // mode 2: dining philosopher's problem
    // mode 3: potion brewers problem
    int mode = 0;
    if (argc <= 1){
        isCorrectArg = 0;
    }else if(argc == 2){
        //printf("%s\n", argv[1]);
        if(strcmp("-d", argv[1]) == 0){
            mode = 2;
        }else if(strcmp("-b", argv[1]) == 0){
            mode = 3;
        }else{
            isCorrectArg = 0;
        }
        //printf("%d\n", mode);
    }else if(argc == 6){
        if(strcmp("-p", argv[1]) == 0){

            if(strcmp("-n", argv[2]) == 0){
                num_producer = atoi(argv[3]);
            }else if(strcmp("-n", argv[4]) == 0){
                num_producer = atoi(argv[5]);
            }

            if(strcmp("-c", argv[2]) == 0){
                num_consumer = atoi(argv[3]);
            }else if(strcmp("-c", argv[4]) == 0){
                num_consumer = atoi(argv[5]);
            }
        }
        if(num_producer <= 1 || num_consumer <= 1){
            isCorrectArg = 0;
        }else{
            mode = 1;
        }
    }else{
        isCorrectArg = 0;
    }

    // display usage
    if(!isCorrectArg){
        printUsage();
        return EXIT_FAILURE;
    }
    /*
        printf(" == num_pro: %d\n == num_con: %d\n", num_producer, num_consumer);
        printf("mode%d",mode);*/
    if(mode == 1){
        //printf(" == num_pro: %d\n == num_con: %d\n", num_producer, num_consumer);
        // producer/consumer problem
        producerConsumer();
    }else if(mode == 2){
        // dining philosopher's problem
        diningPhilosopher();
    }else if(mode == 3){
        // potion brewers problem
        potionBrewers();
    }

    return 0;

}

void printUsage(){
  printf("-p: run the producer/consumer problem\n");
  printf("\t-n: number of producers (required if using -p) -- your solution must work with more than one producer!\n");
  printf("\t-c: number of consumers (required if using -p) -- your solution must work with more than one consumer!\n");
  printf("-d: dining philosopher's problem\n");
  printf("-b: potion brewers problem\n");
}

/*
* Solution:
* producer check if the buffer is full. If so, it wait for consumer to consume
* consumer checks if the buffer is empty. If so, it waits for producer to produce
**********/
void producerConsumer()
{

    pthread_t producers[num_producer], consumers[num_consumer];
    int producer_ID[num_producer], consumer_ID[num_consumer];

    // init mutex
    pthread_mutex_init(&b_inf.mutex, NULL);
    // init semaphore
    // 0: the semaphore is shared between the threads of a process
    sem_init(&b_inf.empty,0,BUFFSIZE);
    sem_init(&b_inf.occupied,0,0);
    b_inf.nextin = 0;
    b_inf.nextout = 0;
    num_item = ITEM * num_producer * num_consumer;

    // https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-pthread-create-create-thread
    // https://www.youtube.com/watch?v=caFjPdWsJDU&t=3s
    for(int i = 0; i < num_producer; ++i){
        // create threads for producer, pass producer ID
        producer_ID[i] = i;
        pthread_create(&producers[i], NULL, (void *)producer, (void *)&producer_ID[i]);
    }
    for(int i = 0; i < num_consumer; ++i){
        // create threads for consumer, pass producer ID
        consumer_ID[i] = i;
        pthread_create(&consumers[i], NULL, (void *)consumer, (void *)&consumer_ID[i]);
    }

    for(int i = 0; i < num_producer; ++i){
        // wait for the target pruducer thread to end.
        pthread_join(producers[i], NULL);
    }
    for(int i = 0; i < num_consumer; ++i){
        // wait for the target consumer thread to end.
        pthread_join(consumers[i], NULL);
    }
    // destroy semaphore and mutex to avoid memory leak
    pthread_mutex_destroy(&b_inf.mutex);
    sem_destroy(&b_inf.empty);
    sem_destroy(&b_inf.occupied);
}

/*
* producer function that produce randum value
* wait consumer by semaphore value
* lock when accessing shared space
****************************/
void *producer(void *arg)
{
    int num_loop = num_item/num_producer;           // number of item producer produces
    int item;
    // loop to produce item
    for(int i = 0; i < num_loop; ++i) {
        // Produce an random item
        item = rand()%BUFFSIZE + 1;

        // if empty is 0, it is locked. If not, it will decrement semaphore value and return immediately
        // empty is 0 if buffer is full
        sem_wait(&b_inf.empty);
        // block accesses from other threads. If it is already blocked, it waits,
        pthread_mutex_lock(&b_inf.mutex);
        // Add item into buffer
        b_inf.buffer[b_inf.nextin] = item;
        printf(" == Producer ID %d: Producerd Item %d Index %d\n", *((int *)arg)+1, b_inf.buffer[b_inf.nextin], b_inf.nextin);
        // move to next index ( this value is shared with other thread )
        b_inf.nextin = (b_inf.nextin+1)%BUFFSIZE;
        // unlock the lock.
        pthread_mutex_unlock(&b_inf.mutex);
        // inclease semaphore value
        // produced item, so inclement occupied value
        sem_post(&b_inf.occupied);
    }
}
/*
* consumer function that consume produced value
* wait producer by semaphore value
* lock when accessing shared space
*************************************************/
void *consumer(void *arg)
{
    int num_loop = num_item/num_consumer;       // number of item consumer consumers
    // loop to consume item
    for(int i = 0; i < num_loop; ++i) {
        // if empty is 0, it is locked. If not, it will decrement semaphore value and return immediately
        // occupied is 0 if buffer is empty
        sem_wait(&b_inf.occupied);
        // block accesses from other threads. If it is already blocked, it waits,
        pthread_mutex_lock(&b_inf.mutex);
        // consume item from buffer
        int item = b_inf.buffer[b_inf.nextout];
        printf(" ** Consumer ID %d: Consumerd Item %d Index %d\n",*((int *)arg)+1,item, b_inf.nextout);
        // move to next index ( this value is shared with other thread )
        b_inf.nextout = (b_inf.nextout+1)%BUFFSIZE;
        // unlock the lock.
        pthread_mutex_unlock(&b_inf.mutex);
        // inclease semaphore value
        // consumed item, so inclement empty value
        sem_post(&b_inf.empty);
    }

}

/*
* Solution:
* Use mutex and one semaphore per philosopher and
* one state variable per philosopher to avoid deadlock situation
* Class material said five plates, five forks, and Five philosophers.
*
* https://legacy.cs.indiana.edu/classes/p415-sjoh/hw/project/dining-philosophers/index.htm
* https://www.geeksforgeeks.org/dining-philosopher-problem-using-semaphores/
******************************************************************/
void diningPhilosopher()
{

    pthread_t philosophers[PHILOSOPHER];
    int phil_ID[PHILOSOPHER];
    // init semaphore
    // mutex starts from 1 because it allows first philosopher to take fork
    sem_init(&mutex, 0, 1);
    // set each semaphore value to 0
    for(int i = 0; i < PHILOSOPHER; ++i){
        sem_init(&phil_mutex[i], 0, 0);
    }

    for(int i =0; i < PHILOSOPHER;++i){
        phil_ID[i] = i;
        // create philosopher threads
        pthread_create(&philosophers[i], NULL, (void *)philosopher,  (void *)&phil_ID[i]);
    }

    for(int i =0; i < PHILOSOPHER;++i){
        pthread_join(philosophers[i], NULL);
    }

    // destroy semaphores to avoid memory leak
    sem_destroy(&mutex);
    for(int i =0; i < PHILOSOPHER;++i){
        sem_destroy(&phil_mutex[i]);
    }


}
/*
* philosopher functoin for tasks
* philosopher complete dinner after eating EAT times
***************/
void* philosopher(void *arg)
{

    int id = *((int *)arg);
    for(int i = 0; i < EAT; ++i){
        think_eat(id, 0);
        take_fork(id);
        think_eat(id, 1);
        put_fork(id);
    }
    printf(" ** PhilosopherID: %d is done\n", id);
}
/*
* sleep 0-2 sec
* mode 0 for thinking
* mode 1 for eating
*******************/
void think_eat(int id, int mode)
{
    int timer = rand()%3;
    if(mode == 0){
        printf(" == PhilosopherID: %d is thinking for %d sec\n", id, timer);
    }else{
        printf(" == PhilosopherID: %d is eating for %d sec\n", id, timer);
    }
    sleep(timer);
}

/*
* This function checks if philosopher is ready to take fork
* If both fork are available, it unlock phil_mutex for him/her to allow
* take fork
**********************************************************************/
void test(int id)
{
    // if neighbors are not eating, the philosopher can take BOTH forks to eat without deadlock
    if (state[id] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        // change state to eating
        state[id] = EATING;
        printf(" -- PhilosopherID %d takes fork %d and %d\n", id + 1, LEFT + 1, id + 1);
        // release the lock for philosopher
        sem_post(&phil_mutex[id]);
    }
}
/*
* This function waits for the philosopher to be ready to take fork by checking phil_mutex
****************************************************/
void take_fork(int id)
{
    // decrement mutex value, wait if 0
    sem_wait(&mutex);
    // if it locks other threads
    // change state to hungry
    state[id] = HUNGRY;
    printf(" ++ PhilosopherID %d is Hungry\n", id + 1);
    // eat if neighbours are not eating
    test(id);
    sem_post(&mutex);
    // wait to take fork. once neighbor put fork and test(), the lock might be released.
    sem_wait(&phil_mutex[id]);
}
/*
* This function release the fork by changing state for philosopher
* then check neighbors are ready to take fork.
* if ready, test function release lock and another thread run eating process.
******************************************************************************/
void put_fork(int id)
{
    // decrement mutex value, wait if 0
    sem_wait(&mutex);
    // if it locks other threads
    // change state to thinking
    state[id] = THINKING;
    printf(" -- PhilosopherID %d putting fork %d and %d down\n", id + 1, LEFT + 1, id + 1);
    // test left and right
    test(LEFT);
    test(RIGHT);
    // inclement mutex
    sem_post(&mutex);
}

/*
* This problem consists of 4 tasks, the Potions Master and 3 brewers.
* The brewers loop forever, first waiting for ingredients, then making the potion.
* The ingredients are bezoars, unicorn horns, and mistletoe berries.
* Solution:
* Use three helper threads (pushers) that helps to keep track of the
* available ingredients by sending signal to agent.
*
* https://www.greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
* https://learningcomputersciencemadeeasy.wordpress.com/2017/04/08/cigarette-smoker-problem/
***************************************************************/
void potionBrewers()
{
    pthread_t masters[MASTER], brewers[BREWERS], agents[TASK], pusher[TASK];

    // init mutex
    pthread_mutex_init(&pusherMutex, NULL);
    pthread_mutex_init(&brewerMutex, NULL);

    // init semaphore
    sem_init (&semAgent, 1, 0);
    sem_init (&bezoars, 1, 0);
    sem_init (&horns, 1, 0);
    sem_init (&berry, 1, 0);
    sem_init (&semBezoars, 1, 0);
    sem_init (&semHorns, 1, 0);
    sem_init (&semBerry, 1, 0);


    //// create threads for each brewers. no need to pass anything
    // create threads for master. no need to pass anything
    pthread_create(&masters[0], NULL, (void *)master, NULL);

    pthread_create(&agents[0], NULL, (void *)agentA, NULL);
    pthread_create(&agents[1], NULL, (void *)agentB, NULL);
    pthread_create(&agents[2], NULL, (void *)agentC, NULL);

    pthread_create(&brewers[0], NULL, (void *)brewerA, NULL);
    pthread_create(&brewers[1], NULL, (void *)brewerB, NULL);
    pthread_create(&brewers[2], NULL, (void *)brewerC, NULL);



    // create threads for each pushers. no need to pass anything
    pthread_create(&pusher[0], NULL, (void *)pusherA, NULL);
    pthread_create(&pusher[1], NULL, (void *)pusherB, NULL);
    pthread_create(&pusher[2], NULL, (void *)pusherC, NULL);

    for(int i = 0; i < TASK; ++i){
        pthread_join(brewers[i], NULL);
    }
    // destroy semaphore and mutex to avoid memory leak
    pthread_mutex_destroy(&pusherMutex);
    pthread_mutex_destroy(&brewerMutex);
    sem_destroy(&semAgent);
    sem_destroy(&bezoars);
    sem_destroy(&horns);
    sem_destroy(&berry);
    sem_destroy(&semBezoars);
    sem_destroy(&semHorns);
    sem_destroy(&semBerry);
}
/*
* master thread that increases agent semaphore value
* for each agent's task
*****************************************************/
void *master(void *arg)
{
    int timer;
    while(1){
        timer = rand()%3;
        sleep(timer);
        // inclement agent semaphore value for tasks
        sem_post(&semAgent);
    }
}
/*
* Agent task A is to supply bezoars AND horns
*****************************/
void *agentA(void *arg)
{
    while(1){
        // wait agent to inclement semaphore value
        sem_wait (&semAgent);
        printf(" ++ TASK A: Supply bezoars AND horns\n");
        // inclement semaphore values for ingredients
        sem_post (&bezoars);
        sem_post (&horns);
    }
}
/*
* Agent task A is to supply bezoars AND berry
*****************************/
void *agentB(void *arg)
{
    while(1){
        // wait agent to inclement semaphore value
        sem_wait (&semAgent);
        printf(" ++ TASK B: Supply bezoars AND berry\n");
        // inclement semaphore values for ingredients
        sem_post (&bezoars);
        sem_post (&berry);
    }
}
/*
* Agent task A is to supply horns AND berry
*****************************/
void *agentC(void *arg)
{
    while(1){
        // wait agent to inclement semaphore value
        sem_wait (&semAgent);
        printf(" ++ TASK C: Supply horns AND berry\n");
        // inclement semaphore values for ingredients
        sem_post (&horns);
        sem_post (&berry);
    }
}

/*
* This brewer has bezoars
******************************/
void *brewerA(void *arg)
{

    while(1){
        // wait for agent to pick ingredients for this brewer
        sem_wait (&semBezoars);
        // lock other brewer to access
        pthread_mutex_lock (&brewerMutex);
        // To avoid infinite loop.
        if(num_potion < POSION){

            printf(" -- Brewer A creating Portion\n");
            sleep(rand()%3);
            num_potion++;
            printf(" -- Brewer A created Portion\n");
            pthread_mutex_unlock (&brewerMutex);

        }else{
            // do not consume ingredients
            sem_post(&semBezoars);
            printf(" ** Created enough potions\n");
            pthread_mutex_unlock (&brewerMutex);
            // exit this thread
            pthread_exit(NULL);
        }
    }
}
/*
* This brewer has horns
******************************/
void *brewerB(void *arg)
{

    while(1){
        // wait for agent to pick ingredients for this brewer
        sem_wait (&semHorns);
        // lock other brewer to access
        pthread_mutex_lock (&brewerMutex);
        // To avoid infinite loop.
        if(num_potion < POSION){

            printf(" -- Brewer B creating Portion\n");
            sleep(rand()%3);
            num_potion++;
            printf(" -- Brewer B created Portion\n");
            pthread_mutex_unlock (&brewerMutex);

        }else{
            // do not consume ingredients
            sem_post(&semHorns);
            printf(" ** Created enough potions\n");
            pthread_mutex_unlock (&brewerMutex);
            // exit this thread
            pthread_exit(NULL);
        }
    }
}
/*
* This brewer has berry
******************************/
void *brewerC(void *arg)
{

    while(1){
        // wait for agent to pick ingredients for this brewer
        sem_wait (&semBerry);
        // lock other brewer to access
        pthread_mutex_lock (&brewerMutex);
        // To avoid infinite loop.
        if(num_potion < POSION){

            printf(" -- Brewer C creating Portion\n");
            sleep(rand()%3);
            num_potion++;
            printf(" -- Brewer C created Portion\n");
            pthread_mutex_unlock (&brewerMutex);

        }else{
            // do not consume ingredients
            sem_post(&semBerry);
            printf(" ** Created enough potions\n");
            pthread_mutex_unlock (&brewerMutex);
            // exit this thread
            pthread_exit(NULL);
        }
    }
}
/*
* pusher functoin checks possible pair of ingredients
* if it is possible to create pair, it inclement semaphore for brewers
**********************************************************************/
void *pusherA(void *arg)
{

    while(1){
        // if bezoars is ready
        sem_wait (&bezoars);
        // lock other pusher thread
        pthread_mutex_lock(&pusherMutex);
        if(ingredients[0]<9){
            ingredients[0]++;
        }

        // check pair
        if(ingredients[0]>0 && ingredients[1] > 0){
            ingredients[1]--;
            ingredients[0]--;
            // unlock brewer
            sem_post (&semBerry);
        }
        if(ingredients[0]>0 && ingredients[2]>0){
            ingredients[2]--;
            ingredients[0]--;
            sem_post (&semHorns);
        }
        // unlock pusher lock
        pthread_mutex_unlock(&pusherMutex);
    }

}
void *pusherB(void *arg)
{
    while(1){
        // if bezoars is ready
        sem_wait (&horns);
        // lock other pusher thread
        pthread_mutex_lock(&pusherMutex);
        if(ingredients[1]<9){
                ingredients[1]++;
            }
        // check pair
        if(ingredients[1]>0 && ingredients[0] > 0){
            ingredients[0]--;
            ingredients[1]--;
            // unlock brewer
            sem_post (&semBerry);
        }
        if(ingredients[1]>0 && ingredients[2]>0){
            ingredients[2]--;
            ingredients[1]--;
            sem_post (&semBezoars);
        }
        // unlock pusher lock
        pthread_mutex_unlock(&pusherMutex);
    }

}
void *pusherC(void *arg)
{
    while(1){
        // if bezoars is ready
        sem_wait (&berry);
        // lock other pusher thread
        pthread_mutex_lock(&pusherMutex);
        if(ingredients[2]<9){
                ingredients[2]++;
            }
        // check pair
        if(ingredients[2]>0 && ingredients[0] > 0){
            ingredients[0]--;
            ingredients[2]--;
            // unlock brewer
            sem_post (&semHorns);
        }
        if(ingredients[2]>0 && ingredients[1]>0){
            ingredients[1]--;
            ingredients[2]--;
            sem_post (&semBezoars);
        }
        // unlock pusher lock
        pthread_mutex_unlock(&pusherMutex);
    }

}