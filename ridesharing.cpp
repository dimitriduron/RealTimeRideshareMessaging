#include "io.h"

/*
PRODUCER

INPUTS: is technically void* but just needs to be converted to the shared object type
OUTPUT: none

Producer Logic/Pseudocode
1. Declare a shared object to store the passed object
2. Store producer type locally so that the thread will remember
3. Loop until all requests are made
    3.1 If human request, make sure the queue isn't full
    3.2 Decrement the amount of requests to show that a request has been made
    3.3 Decrement queue semaphore to check for the queue having space
    3.4 Decrement critical semaphore to check for exclusive access to data
    3.5 Add specific request type to the broker queue and increment the appropiate arrays
    3.6 Increment critical semaphore to give access back to the shared data
    3.7 Increment queue size to show that the queue size got smaller, so the bounds remain 0-12
    3.8 Sleep a specific amount based on command line arguments
4. Exit thread
*/
void* producer(void* input){
    struct ThreadVars *threadVar;
    threadVar = (ThreadVars *)input;
    Requests reqType = threadVar->reqType;
    threadVar->reqType = RequestTypeN;
    while(threadVar->totRequests){
        //check if the hdr amount is valid within the queue
        if(reqType == HumanDriver)
            sem_wait(threadVar->maxHDRInQueue);
            if(!threadVar->totRequests) break;
        //decrement the total amount of requests
        threadVar->totRequests--;
        //decrement to show that there is maxQueueSize-queueSize space in the queue left
        sem_wait(threadVar->maxQueueSize);

        //critical section
        sem_wait(threadVar->criticalProtection);

        threadVar->broker.push(reqType);
        if(reqType == HumanDriver){
            threadVar->produced[HumanDriver]++;
            threadVar->brokerArr[HumanDriver]++;
        }
        else if(reqType == RoboDriver){
            threadVar->produced[RoboDriver]++;
            threadVar->brokerArr[RoboDriver]++;
        }
        io_add_type(reqType, threadVar->brokerArr, threadVar->produced);

        sem_post(threadVar->criticalProtection);
        //end of critical section

        //increment to show that there is queueSize+1 items in the queue
        sem_post(threadVar->queueSize);

        if(reqType == HumanDriver)
            usleep(MICRO2MILLI*threadVar->hdrTime);
        else if(reqType == RoboDriver)
            usleep(MICRO2MILLI*threadVar->rdrTime);
    }
    return NULL;
}

/*
Consumer

INPUTS: is technically void* but just needs to be converted to the shared object type
OUTPUT: none

Consumer Logic/Pseudocode
1. Declare a shared object to store the passed object
2. Store consumer type locally so that the thread will remember
3. Loop until all requests are made and the queue is empty
    3.1 Decrement the upper bounds of the queue so we know we aren't going beyond a size of 12
    3.2 Decrement critical semaphore to check for exclusive access to data
    3.3 Grab the front value from broker and store the request type
    3.4 If request is Human, increment hdr semaphore to show that there is space for another hdr
    3.5 Increment appropiate arrays and decrement request type from broker array
    3.6 Store consumer array into shared object and output consumer data
    3.7 Increment critical semaphore to give access back to the shared data
    3.8 Increment queue size to show that there is another spot in the queue
    3.9 Sleep a specific amount based on command line arguments
4. Exit thread
*/
void* consumer(void* input){
    struct ThreadVars *threadVar;
    threadVar = (ThreadVars *)input;
    Consumers consType = threadVar->consType;
    threadVar->consType = ConsumerTypeN;
    int consumedType[RequestTypeN] = {0, 0};
    Requests requestType;
    while(threadVar->broker.size() || threadVar->totRequests){
        //decrement to show that there is queueSize-1 items in the queue
        sem_wait(threadVar->queueSize);

        //critical section
        sem_wait(threadVar->criticalProtection);

        requestType = threadVar->broker.front();
        threadVar->broker.pop();

        // increment consumed type and decrement from broker arr
        consumedType[requestType]++;
        threadVar->brokerArr[requestType]--;
        // show that there is space for another hdr in the queue
        if(requestType == HumanDriver){
            sem_post(threadVar->maxHDRInQueue);
        }
        
        io_remove_type(consType, requestType, threadVar->brokerArr, consumedType);
        threadVar->consumedFinal[consType] = consumedType;

        sem_post(threadVar->criticalProtection);
        //end of critical section

        //increment to show there is maxQueueSize-queueSize space in the queue left
        sem_post(threadVar->maxQueueSize);

        //wait time
        if(consType == CostAlgoDispatch)
            usleep(MICRO2MILLI*threadVar->costDispatcherTime);
        else if(consType == FastAlgoDispatch)
            usleep(MICRO2MILLI*threadVar->fastDispatcherTime);
    }
    return NULL;
}