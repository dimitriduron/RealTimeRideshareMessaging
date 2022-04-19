#include "io.h"

/*
Program Logic
1. Declare the shared object that will share data across threads
2. Pass important data from the command lines into the shared object
3. Declare and initalize the semaphores with the correct resource amounts
4. Store the semaphores into the shared object along with more data
5. Run 2 producer threads, one for Human requests and another for Robot requests
6. Run 2 consumer threads, one with the Fast algorithm and another for the Cost algorithm
7. Loop until all threads are complete
8. Output summary and exit
*/
int main(int argc, char** argv){
    //*******HELPFUL VARIABLES AND OBJECTS*******//
    ThreadVars *threadVar;
    threadVar = new ThreadVars;
    int tempArg;
    
    //*******COMMAND LINE ARGS*******//
    while( (tempArg = getopt(argc, argv, "n:c:f:h:a:")) != -1){
        switch (tempArg){
            case 'n':
                threadVar->totRequests = atoi(optarg);
                break;
            case 'c':
                threadVar->costDispatcherTime = atoi(optarg);
                break;
            case 'f':
                threadVar->fastDispatcherTime = atoi(optarg);
                break;
            case 'h':
                threadVar->hdrTime = atoi(optarg);
                break;
            case 'a':
                threadVar->rdrTime = atoi(optarg);
                break;
            default:
                cout << "bad output" << endl;
                exit(1);
        }
    }

    //*******MAIN CODE SECTION*******//
    // all semaphores declared, it doesn't matter the name as they will only be accessed once
    sem_t s1;
    sem_t s2;
    sem_t s3;
    sem_t s4;
    sem_t s5;

    // the value is assigned and the semaphores are initalized
    sem_init(&s1, SEM_TYPE, MAX_QUEUE_SIZE);
    sem_init(&s2, SEM_TYPE, MIN_QUEUE_SIZE);
    sem_init(&s3, SEM_TYPE, PROTECT_SIZE);
    sem_init(&s4, SEM_TYPE, MAX_HDR_QUEUE);
    sem_init(&s5, SEM_TYPE, MIN_QUEUE_SIZE);

    //important data, including most of the semaphores are passed into the shared data
    threadVar->maxQueueSize = &s1;
    threadVar->queueSize = &s2;
    threadVar->criticalProtection = &s3;
    threadVar->maxHDRInQueue = &s4;
    threadVar->hdrInQueue = &s5;
    threadVar->brokerArr[HumanDriver] = 0;
    threadVar->brokerArr[RoboDriver] = 0;
    threadVar->reqType = HumanDriver;
    threadVar->consType = FastAlgoDispatch;

    //threads are declared and initalized
    pthread_t humanProducer, robotProducer, costConsumer, fastConsumer;
    pthread_create(&humanProducer, NULL, &producer, threadVar);
    pthread_create(&costConsumer, NULL, &consumer, threadVar);

    // wait for the request type to be set within the thread
    while(threadVar->reqType == HumanDriver);
    threadVar->reqType = RoboDriver;
    pthread_create(&robotProducer, NULL, &producer, threadVar);

    // wait for the algo type to be set within the thread
    while(threadVar->consType == FastAlgoDispatch);
    threadVar->consType = CostAlgoDispatch;
    pthread_create(&fastConsumer, NULL, &consumer, threadVar);
    
    //a loop to make sure that the main thread doesn't end while the threads are running
    while(threadVar->broker.size() || threadVar->totRequests);

    //last critical section
    sem_wait(threadVar->criticalProtection);
    io_production_report(threadVar->produced, threadVar->consumedFinal);
    sem_post(threadVar->criticalProtection);

    return 0;
}