//C structures

#include <string.h>
#include "../mpx_supt.h"
#include "R2_Internal_Functions_And_Structures.h"

queue *ready;
queue *blocked;
queue *suspendedReady;
queue *suspendedBlocked;

// a function to allocate memory for the queues and initialize the queues.


PCB *allocatePCB() //Returns the created PCB pointer if successful, returns NULL if an error occurs.
{
    //COLTON WILL PROGRAM THIS FUNCTION

    //allocatePCB() will use sys_alloc_mem() to allocate memory for a new PCB, possible including the stack, and perform any reasonable initialization.
    PCB* newPCB = (PCB*)sys_alloc_mem(sizeof(PCB));


    char name[20] = "newPCB";
    strcpy(newPCB->processName, name);

    newPCB->suspendedStatus = 1;
    newPCB->runningStatus = -1;
    newPCB->stackTop = (newPCB->stackTop + 1024);
    newPCB->stackBase = newPCB->stackBase;
    newPCB->priority = 0;
    
    // Setting the PCBs prev and next PCB
    newPCB->nextPCB = NULL;
    newPCB->prevPCB = NULL;

    newPCB->processClass = NULL;
   
    return newPCB;
}

int freePCB(PCB *PCB_to_free) //Return 0 is success code, reurn 1 is error code.
{
    // ANASTASE WILL PROGRAM THIS FUNCTION

    //freePCB() will use sys_free_mem() to free all memory associated with a given PCB (the stack, the PCB itself, etc.)

    (void)PCB_to_free;
    return sys_free_mem(PCB_to_free);

}

PCB *setupPCB(char *processName, unsigned char processClass, int processPriority) //Returns the created PCB pointer if successful, returns NULL if an error occurs.
{
    //COLTON WILL PROGRAM THIS FUNCTION

    //setupPcb() will call allocatePCB() to create an empty PCB, initializes the PCB information, sets the PCB state to ready, not suspended.

    PCB* tempPCB = allocatePCB();

    PCB* returnedPCB;
    
    if(findPCB(processName)->processName == processName){

        char message[] = "There is already a PCB with this name.\n";
        int messLength = strlen(message);
        sys_req(WRITE, DEFAULT_DEVICE, message, &messLength);

        returnedPCB = NULL;

    }
    else{

        strcpy(tempPCB->processName, processName);
        tempPCB->processClass = processClass;
        tempPCB->priority = processPriority;
        tempPCB->runningStatus = 0;
        tempPCB->suspendedStatus = 1;

        returnedPCB = tempPCB;

    }

    return returnedPCB;

}

PCB *findPCB(char *processName) //Returns the created PCB pointer if successful, returns NULL if PCB cannot be found.
{
    // ANASTASE WILL PROGRAM THIS FUNCTION

    //findPCB() will search all queues for a process with a given name.

    (void)processName;
    // searching in ready queue

    PCB *found_ready_pcb; // this is a pointer to another pointer (** starts). Need testing!
    found_ready_pcb = searchPCB(ready, processName);
    if (found_ready_pcb)
    {
        return found_ready_pcb;
    }

    // searching PCB in blocked queue
    PCB *found_blocked_pcb;
    found_blocked_pcb = searchPCB(blocked, processName);
    if (found_blocked_pcb)
    {
        return found_blocked_pcb;
    }

    // searching PCB in suspendedReady queue
    PCB *found_suspended_ready_pcb;
    found_suspended_ready_pcb = searchPCB(suspendedReady, processName);
    if (found_suspended_ready_pcb)
    {
        return found_suspended_ready_pcb;
    }

    // searching PCB in suspendedBlocked queue
    PCB *found_suspended_blocked_pcb;
    found_suspended_blocked_pcb = searchPCB(suspendedBlocked, processName);
    if (found_suspended_blocked_pcb)
    {
        return found_suspended_blocked_pcb;
    }

    return NULL; // for testing
}


PCB *searchPCB(queue *PCB_container, char *processName)
{
    // PCB_container has PCB*head and PCB*tail pointers
    //queue*tempQueue;

    PCB *tempPtr = PCB_container->head;

    int count = PCB_container->count; // tempQueue->count;

    int found = 0; // not found signal
    // detecting buffer overflow
    if (strlen(processName) > 20)
    {

        char error_message[30] = "Invalid process name.";
        int error_size = strlen(error_message);
        sys_req(WRITE, DEFAULT_DEVICE, error_message, &error_size);
        //return cz we have to stop if the process name is too long
    }

    int value = 0;
    while (value <= count)
    {
        if (strcmp(tempPtr->processName, processName) == 0)
        {
            found = 1; // found signal
            return tempPtr;
            break;
        }

        tempPtr = tempPtr->nextPCB; // don't know why this line is giving assignment from incompatible pointer type error.
        value++;
    }

    if (found == 0)
    {
        char result_message[30] = "The process was not found.";
        int result_size = strlen(result_message);
        sys_req(WRITE, DEFAULT_DEVICE, result_message, &result_size);
        return NULL; // Why are this return not recognized??
    }
    return tempPtr; // for testing.
}


void insertPCB(PCB *PCB_to_insert)
{
    //BENJAMIN WILL PROGRAM THIS FUNCTION

    //insertPCB() will insert a PCB into the appropriate queue.
    //Note: The ready queue is a priority queue and the blocked queue is a FIFO queue.

    if (PCB_to_insert->runningStatus == 0 && PCB_to_insert->suspendedStatus == 1)
    { // Insert into ready queue

        queue *ready = getReady();
        PCB *tempPtr = ready->head;

        if (tempPtr != NULL)
        {
            int temp = 0;
            while (temp < ready->count)
            {
                if (PCB_to_insert->priority < tempPtr->priority)
                {
                    tempPtr = tempPtr->nextPCB;
                }
                else if (PCB_to_insert->priority >= tempPtr->priority)
                {
                    PCB_to_insert->nextPCB = tempPtr;
                    PCB_to_insert->prevPCB = tempPtr->prevPCB;
                    tempPtr->prevPCB = PCB_to_insert;
                }
                else if (PCB_to_insert->priority < tempPtr->priority && tempPtr->nextPCB == NULL)
                {
                    tempPtr->nextPCB = PCB_to_insert;
                    PCB_to_insert->prevPCB = tempPtr;
                }
                temp++;
            }
            ready->count++;
        }
        else
        {
            tempPtr = PCB_to_insert;
            ready->count++;
        }
    }
    else if (PCB_to_insert->runningStatus == 0 && PCB_to_insert->suspendedStatus == 0)
    { // Insert into suspended ready queue
        queue *suspendedReady = getSuspendedReady();
        PCB *tempPtr = suspendedReady->head;

        if (tempPtr != NULL)
        {
            int temp = 0;
            while (temp < suspendedReady->count)
            {
                if (PCB_to_insert->priority < tempPtr->priority)
                {
                    tempPtr = tempPtr->nextPCB;
                }
                else if (PCB_to_insert->priority >= tempPtr->priority)
                {
                    PCB_to_insert->nextPCB = tempPtr;
                    PCB_to_insert->prevPCB = tempPtr->prevPCB;
                    tempPtr->prevPCB = PCB_to_insert;
                }
                else if (PCB_to_insert->priority < tempPtr->priority && tempPtr->nextPCB == NULL)
                {
                    tempPtr->nextPCB = PCB_to_insert;
                    PCB_to_insert->prevPCB = tempPtr;
                }
                temp++;
            }
            suspendedReady->count++;
        }
        else
        {
            tempPtr = PCB_to_insert;
            suspendedReady->count++;
        }
    }
    else if (PCB_to_insert->runningStatus == -1 && PCB_to_insert->suspendedStatus == 1)
    { // Insert into blocked queue
        queue *blocked = getBlocked();
        PCB *tempPtr = blocked->tail;

        tempPtr->nextPCB = PCB_to_insert;
        PCB_to_insert->prevPCB = tempPtr;
    }
    else if (PCB_to_insert->runningStatus == -1 && PCB_to_insert->suspendedStatus == 0)
    { // Insert into suspended blocked queue
        queue *suspendedBlocked = getSuspendedBlocked();
        PCB *tempPtr = suspendedBlocked->tail;

        tempPtr->nextPCB = PCB_to_insert;
        PCB_to_insert->prevPCB = tempPtr;
    }

}

int removePCB(PCB *PCB_to_remove) //Return 0 is success code, return 1 is error code.
{
    //BENJAMIN WILL PROGRAM THIS FUNCTION

    //removePCB() will remove a PCB from the queue in which it is currently stored.

    PCB *tempPrev = PCB_to_remove->prevPCB;
    PCB *tempNext = PCB_to_remove->nextPCB;

    tempPrev->nextPCB = tempNext;
    tempNext->prevPCB = tempPrev;

    PCB_to_remove->nextPCB = NULL;
    PCB_to_remove->prevPCB = NULL;

    int result = sys_free_mem(PCB_to_remove);

    if (result == -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

queue *getReady()
{
    return ready;
}

queue *getBlocked()
{
    return blocked;
}

queue *getSuspendedReady()
{
    return suspendedReady;
}

queue *getSuspendedBlocked()
{
    return suspendedBlocked;
}