#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Process {
    int priority;
    int arrivalTime;
    int admissionTime;
    int exitTime;
    int totalExecutionTime;
    int type;  // 0 for silver, 1 for gold and 2 for plat
    int* instructionInfo;
    int lastExecutedLine;
    int quantumCount;
    int executionTime;
    bool isPushed;
} Process;

typedef struct Node {
    int processID;
    int priority;
    struct Node* next;
} Node;

void readInstructions(char *filename);
void readProcessInstructions(char *filename, int index);
void readProcessFiles();
void readDefinitionFile(char *filename);
Node* createNode(int id, int priority);
Node* pop(Node** head);
Node* push(Node **head,int id, int priority, int time,int executedId);
int isEmpty(Node** head);
int compareTo(Node* old, Node* new,int id);
void cleanProcesses();
void cleanActiveProcesses();
void cleanQueue(Node** head);
void addToQueue(int time,Node** head,int id,bool flag);
int findClosestArrivalTime(int time);
float* calculateTimes();
void reorder(Node** head,int time,int id);

// global variables
int const INSTRUCTION_NUMBER = 21;
int contextSwitch = 10;
int quantums[3] = {80,120,120};
int typeConversions[2] = {3,5};
int instructions[21];
Process processes[10];
int* activeProcesses;
int processNumber = 0;


int main() {
    int time = 0;
    int id = -1;
    int operationTime;
    int instructionLine;
    int instructionId;
    int oldHeadId;
    int exitedProcesses = 0;
    int currentProcessId = -1;
    bool exitInstruction = false;
    Node* head = NULL;
    bool isQuantumFinished = false;

    readInstructions("instructions.txt");
    readProcessFiles();
    char* input = "definition.txt";
    readDefinitionFile(input);

    while (1)
    {
        if (time > 0 && !isEmpty(&head)) {
            currentProcessId = head->processID;
            if ((processes[id].executionTime >= quantums[processes[id].type] && processes[id].type < 2) || exitInstruction) {
                
                // update attributes
                isQuantumFinished = true;
                processes[id].arrivalTime = time;
                processes[id].quantumCount++;
                processes[id].executionTime = 0;

                // type conversion
                if (processes[id].quantumCount >= typeConversions[processes[id].type] && processes[id].type != 2) {
                    processes[id].type++;
                    processes[id].quantumCount = 0;
                }

                if (!exitInstruction) { 
                    reorder(&head,time,id);
                    id = -1;
                } else {
                    exitedProcesses++;
                    Node *oldHead = pop(&head);
                    int poppedId = oldHead->processID;
                    processes[poppedId].arrivalTime = -1;
                    processes[poppedId].exitTime = time;
                    id = -1;
                }

                if (exitedProcesses >= processNumber) // if all processes exit, break
                    break;
                exitInstruction = false;
            }
        }
        
        addToQueue(time,&head,id,isQuantumFinished); // add processes to ready queue if their time has come

        if (!isEmpty(&head) && head->processID != currentProcessId) {  // context switch
            time += 10;
        }
        
        if (time == 0) time += 10;
        
        if (isEmpty(&head)) {  // check if cpu is idle
            int addition = findClosestArrivalTime(time);
            if (addition != __INT_MAX__) {
                time += addition;
                id = -1;
                continue;
            }
        }
        
        // set attributes and update them
        id = head->processID;  // determine the next process
        instructionLine = processes[id].lastExecutedLine;
        instructionId = processes[id].instructionInfo[instructionLine];
        operationTime = instructions[instructionId - 1];
        processes[id].executionTime += operationTime;
        processes[id].totalExecutionTime += operationTime;
        processes[id].lastExecutedLine++;

        time += operationTime;  // advance time
        isQuantumFinished = false;
        
        if (instructionId == 21)  // check if the instruction is exit
            exitInstruction = true;
    }

    float* result = calculateTimes();  // calculate turnaround and waiting times
    printf(result[1] == (int)result[1] ? "%.0f\n" : "%.1f\n", result[1]);
    printf(result[0] == (int)result[0] ? "%.0f\n" : "%.1f\n", result[0]);
    
    cleanQueue(&head);
    cleanActiveProcesses();
    cleanProcesses();
    return 0;
}

Node* createNode(int id, int priority) {  // helper function to create a node
    Node* temp = (Node*) malloc(sizeof(Node));
    temp->processID = id;
    temp->priority = priority;
    temp->next = NULL;
    return temp;
}

Node* pop(Node** head) {  // removes head from the queue
    Node* temp = *head;
    (*head) = (*head)->next;
    return temp;
}

void reorder(Node** head,int time,int id) {  // reorders the queue and sets head to the new queue's head
    if (isEmpty(head) || (*head)->next == NULL) {  // if queue is empty or have only one element, no need to reorder
        return;
    }
    Node* sortedList = NULL;
    Node* current = (*head);
    while (true) {  // push nodes to the new queue
        sortedList = push(&sortedList, current->processID, current->priority,time,id);
        if(current->next == NULL)
            break;
        current = current->next;
    }
    *head = sortedList;
}

Node* push(Node **head,int id, int priority, int time, int executedId) {  // creates a new node and adds it to the queue
    Node* temp = createNode(id,priority);
    if (isEmpty(head)) {  // if queue is empty, make temp the head
        (*head) = temp;
        processes[id].isPushed = true;
        return temp;
    }
    Node* start = (*head);
    if (compareTo(start,temp,executedId) < 0) {  // if new node has a higher priority, change head
        temp->next = *head;
        (*head) = temp;
    } else {
        while (start->next != NULL && compareTo(start->next,temp,executedId) > 0) {  // iterate until find an appropiate location
            start = start->next;
        }
        temp->next = start->next;
        start->next = temp;
    }
    processes[id].isPushed = true;  // mark node as pushed
    return *head;  // return the head, important when the head is changed
}

int compareTo(Node* old, Node* new, int id) {  // core function of the algorithm
    Process oldProcess = processes[old->processID];
    Process newProcess = processes[new->processID];
    
    // compares the priorities
    // positive return means the old process has higher priority
    // negative means the otherwise

    if (oldProcess.type == 2 && (id == old->processID || newProcess.type != 2))  // if old process is plat and currently is being executed
        return 1;                                                                // or the old process is plat and new is not

    if (newProcess.type == 2 && oldProcess.type != 2)  // new process is plat and old is not
        return -1;
    
    int priorityDifference = oldProcess.priority - newProcess.priority;  // if neither or both of them plat, check priorities
    if (priorityDifference != 0)
        return priorityDifference;

    int arrivalTimeDifference = newProcess.arrivalTime - oldProcess.arrivalTime;  // in the cases of equal priorities, look arrival times
    if (arrivalTimeDifference != 0)
        return arrivalTimeDifference;

    char oldIDStr[4];  // Adjust the size based on your process ID length
    char newIDStr[4];
    sprintf(oldIDStr, "%d", old->processID + 1);
    sprintf(newIDStr, "%d", new->processID + 1);

    return strcmp(newIDStr,oldIDStr); // if everything is same, compare names
} 

int isEmpty(Node** head) {  // helper function to check if the head is null
    return (*head) == NULL; 
}

void readInstructions(char *filename) {  // instruction file reader
    FILE *file = fopen(filename, "r");
    int i = 0;
    while (fscanf(file, "%*s %d", &instructions[i]) != EOF && i < INSTRUCTION_NUMBER) {
        i++;
    }
    fclose(file);
}

void readProcessInstructions(char *filename, int index) {  // process files reader
    FILE *file = fopen(filename, "r");
    processes[index].instructionInfo = (int *)malloc(15 * sizeof(int));
    char instruction[7];  // Fixed size array to store the instruction

    int i = 0;
    while (fscanf(file, "%s", instruction) != EOF) {
        if (strcmp(instruction, "exit") == 0) {
            processes[index].instructionInfo[i] = 21;  // stop reading when encountering "exit"
            break;
        }
        processes[index].instructionInfo[i] = atoi(&instruction[5]);
        i++;
    }

    fclose(file);
}

void readProcessFiles() {  // reads all process files in order
    for (int i = 0; i < 10; i++) {
        char filename[10];
        sprintf(filename, "P%d.txt", i + 1);
        readProcessInstructions(filename, i);
    }
}


void readDefinitionFile(char *filename) {  // reads the input file
    FILE *file = fopen(filename, "r");
    char processId[3];
    int priority, arrivalTime;
    char type[7];
    char line[100];
    activeProcesses = (int *)malloc(10 * sizeof(int));

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s %d %d %s", processId, &priority, &arrivalTime, type) != 4) {
            fprintf(stderr, "Error reading line: %s\n", line);
            exit(EXIT_FAILURE);
        }
        // set attributes accordingly
        int index = atoi(&processId[1]) - 1;  // Convert processId array index
        activeProcesses[processNumber] = index;
        processNumber++;
        processes[index].priority = priority;
        processes[index].arrivalTime = arrivalTime;
        processes[index].lastExecutedLine = 0;
        processes[index].quantumCount = 0;
        processes[index].isPushed = false;
        processes[index].admissionTime = arrivalTime;
        
        if (strcmp(type, "SILVER") == 0) {
            processes[index].type = 0;
        } else if (strcmp(type, "GOLD") == 0) {
            processes[index].type = 1;
        } else if (strcmp(type, "PLATINUM") == 0) {
            processes[index].type = 2;
        }
    }

    if (processNumber < 10)
        activeProcesses = realloc(activeProcesses, processNumber * sizeof(int));
    fclose(file);
}

// memory cleaning functions
void cleanProcesses() {
    for (int i = 0; i < 10; i++) {
        free(processes[i].instructionInfo);
    }
}

void cleanActiveProcesses() {
    for (int i = 0; i < processNumber; i++)
    {
        free(activeProcesses);
        activeProcesses = NULL;
    }
}

void cleanQueue(Node** head) {
    Node* current = *head;
    Node* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    *head = NULL; // Set the head to NULL after freeing all nodes
}

void addToQueue(int time,Node** head, int id, bool flag) {  // calls push function iteratively
    for (int i = 0; i < processNumber; i++)
    {
        int index = activeProcesses[i];
        if (time >= processes[index].arrivalTime  && processes[index].arrivalTime >= 0 && !processes[index].isPushed) {
            push(head,index,processes[index].priority,time,id);
        }
    }
    if (id != -1 && id != (*head)->processID && !flag) {  // if the head is changed, that means a cpu burst
        processes[id].arrivalTime = time;  // set current time to new arrival time
        processes[id].quantumCount++;  // increase time quantum count
        processes[id].executionTime = 0; // reset execution time
        if (processes[id].quantumCount >= typeConversions[processes[id].type] && processes[id].type != 2) {  // type conversion
            processes[id].type++;
            processes[id].quantumCount = 0;
        }
        reorder(head,time,id);
    }
}

int findClosestArrivalTime(int time) {  // if cpu is idle, find the first comer process
    int result = __INT_MAX__;
    int current = 0;
    for (int i = 0; i < processNumber; i++)
    {
        current = processes[activeProcesses[i]].admissionTime;
        if (time < current && current < result)
            result = current;
    }
    return result - time;
}

float* calculateTimes() {  // calculate turnaround and waiting times
    float* result = (float *) malloc(sizeof(float) * 2);
    float turnaround = 0.0;
    float temp = 0.0;
    float waiting = 0.0;
    int index;
    for (int i = 0; i < processNumber; i++)
    {
        index = activeProcesses[i];
        temp = processes[index].exitTime - processes[index].admissionTime;
        turnaround += temp;
        waiting += temp - processes[index].totalExecutionTime;
    }
    result[0] = turnaround / processNumber;
    result[1] = waiting / processNumber;
    return result;
}