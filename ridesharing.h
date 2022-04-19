#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <queue>
using namespace std;

// Constants predetermined
#define MAX_QUEUE_SIZE  12
#define MIN_QUEUE_SIZE  0
#define SEM_TYPE        0
#define PROTECT_SIZE    1
#define MAX_HDR_QUEUE   4
#define MICRO2MILLI     1000

#ifndef RIDESHARING_H
#define RIDESHARING_H

void* consumer(void*);
void* producer(void*);


/*
 * Arrays with producer and consumer names
 * These can be indexed with the enumerated types below
 * and are defined in io.c
 */
extern const char *producerNames[];
extern const char *producerAbbrevs[];  
extern const char *consumerNames[]; 

/*
 * Enumerated types to be used by the producers and consumers
 * These are expected in the input/output functions (io.h)
 * should be useful in your producer and consumer code.
 */

/**
 * The broker can hold up to a maximum of 12 undispatched requests 
 * in its request queue at any given time.
 * To promote the use of autonomous cars (as human drivers cost more), 
 * the broker would hold no more than 4 undispatched requests for a human driver at any given time. 
*/

/*
 * Two rider request services (producers of requests) are offered: 
 *   one for publishing (producing) a ride request for a human driver, 
 *   one for publishing (producing) a ride request for an autonomous car.
 * Each ride request service (producer) only publishes (produces) 
 * one type of requests,
 * so RequestType is synomonous with the producer type
 */
typedef enum Requests {
  HumanDriver = 0,   // ride with a human driver
  RoboDriver = 1,  // ride with an autonomous car 
  RequestTypeN = 2,   // number of ride request types
} RequestType;

/* 
 * Two dispatcher services (consumers of requests) are available using 
 * different driver search algorithms
 *   one uses a cost saving algorithm to find a nearby driver
 *   one uses a fast-matching algorithm to find a nearby driver 
 *   Requests are taken off from the broker request queue (by dispatchers) 
 *   in the order that they are published (produced).
*/
typedef enum Consumers {
  CostAlgoDispatch = 0,   // dispatch to a nearby driver based on cost
  FastAlgoDispatch = 1,  // dispatch to a nearby driver based on fast matching 
  ConsumerTypeN = 2,   // Number of consumers
} ConsumerType;

/*
ThreadVars is meant to be a object to hold important data for to be shared across threads.
Default values are set but can be overwritten if the command line has specific arguments
reqType and consType are used to label the specific threads temporarily
Broker is the main queue that holds the important data
*/
struct ThreadVars{
  int totRequests = 120;
  int costDispatcherTime = 0;
  int fastDispatcherTime = 0;
  int hdrTime = 0;
  int rdrTime = 0;

  Requests reqType;
  Consumers consType;

  sem_t *queueSize;
  sem_t *maxQueueSize;
  sem_t *criticalProtection;
  sem_t *hdrInQueue;
  sem_t *maxHDRInQueue;
  int produced[RequestTypeN] = {0, 0};
  int consumedFast[RequestTypeN];
  int consumedCost[RequestTypeN];
  int *consumedFinal[RequestTypeN];
  int brokerArr[RequestTypeN];
  queue<Requests> broker;
};

#endif
