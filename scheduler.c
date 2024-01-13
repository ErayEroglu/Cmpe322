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
int peek(Node* head);
Node* createNode(int id, int priority);
Node* pop(Node** head);
Node* push(Node **head,int id, int priority, int time,int executedId);
int isEmpty(Node** head);
int compareTo(Node* old, Node* new,int id);
void cleanup();
void addToQueue(int time,Node** head,int id);
int findClosestArrivalTime(int time);
float* calculateTimes();
void printQueue(Node* head);
int removeNextToHead(Node** head);
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
    int currentProcessId;
    bool isTimeQuantumExceeded;
    bool exitInstruction = false;
    bool isContexSwitched = false;
    Node* head = NULL;

    readInstructions("instructions.txt");
    readProcessFiles();
    char* input = "definition.txt";
    readDefinitionFile(input);
    while (1)
    {
        if (time > 0 && !isEmpty(&head)) {
            currentProcessId = head->processID;
            if ((processes[id].executionTime >= quantums[processes[id].type] && processes[id].type < 2) || exitInstruction) {
                
                processes[id].quantumCount++;
                processes[id].executionTime = 0;
                processes[id].arrivalTime = time;

                // type conversion
                if (processes[id].quantumCount >= typeConversions[processes[id].type] && processes[id].type != 2) {
                    processes[id].type++;
                    processes[id].quantumCount = 0;
                }
                
                // isTimeQuantumExceeded = true;
                // Node *oldHead = pop(&head);
                // int poppedId = oldHead->processID;
                printf("P%d : %d \n",id + 1,processes[id].quantumCount);
                if (!exitInstruction) { 
                    reorder(&head,time,id);
                    // processes[poppedId].arrivalTime = time;
                    // processes[poppedId].isPushed = false;
                    // push(&head,poppedId,processes[poppedId].priority,time);
                    
                } else {
                    exitedProcesses++;
                    Node *oldHead = pop(&head);
                    int poppedId = oldHead->processID;
                    processes[poppedId].arrivalTime = -1;
                    processes[poppedId].exitTime = time;
                    id = -1;
                }

                if (exitedProcesses >= processNumber)
                    break;
                exitInstruction = false;
            }
        }
        
        addToQueue(time,&head,id); // add processes to ready queue if their time has come
        
        if (!isEmpty(&head) && head->processID != currentProcessId) {  // context switch
            time += 10;
            // if(head->next != NULL){
            //     int temp = removeNextToHead(&head);
            //     push(&head,temp,processes[temp].priority,time);            
            // }
            //reorder(&head,time);
        }
        
        if (time == 0) time += 10;
        
        if (isEmpty(&head)) {  // check if cpu is idle
            int addition = findClosestArrivalTime(time);
            if (addition != __INT_MAX__)
                {
                    time += addition;
                    id = -1;
                continue;}
        }
        
        id = head->processID;
        instructionLine = processes[id].lastExecutedLine;
        instructionId = processes[id].instructionInfo[instructionLine];
        operationTime = instructions[instructionId - 1];
        processes[id].executionTime += operationTime;
        processes[id].totalExecutionTime += operationTime;
        processes[id].lastExecutedLine++;

        if(!isEmpty(&head))
            printf("%d P%d instr%d %d \n",time, head->processID + 1,instructionId, processes[head->processID].type);
        
        printQueue(head);
        time += operationTime;  // advance time
        
        if (instructionId == 21)
            exitInstruction = true;
    }

    float* result = calculateTimes();
    printf(result[1] == (int)result[1] ? "%.0f\n" : "%.1f\n", result[1]);
    printf(result[0] == (int)result[0] ? "%.0f\n" : "%.1f\n", result[0]);
    return 0;
}

int peek(Node* head) {
    return head->processID;
}

void printQueue(Node* head) {
    printf("Queue: ");
    Node* current = head;
    while (current != NULL) {
        printf("P%d %d ", current->processID + 1, processes[current->processID].type);
        current = current->next;
    }
    printf("\n");
}

Node* createNode(int id, int priority) {
    Node* temp = (Node*) malloc(sizeof(Node));
    temp->processID = id;
    temp->priority = priority;
    temp->next = NULL;
    return temp;
}

Node* pop(Node** head) {
    Node* temp = *head;
    (*head) = (*head)->next;
    return temp;
}

int removeNextToHead(Node** head) {
    Node* temp = (*head)->next;
    (*head)->next = temp->next;
    int id = temp->processID;
    free(temp);
    return id;
}

void reorder(Node** head,int time,int id) {
    if (isEmpty(head) || (*head)->next == NULL) {
        // List is empty or has only one element, no need to reorder
        return;
    }
    //printQueue(*head);
    Node* sortedList = NULL;
    Node* current = (*head);

    while (true) {
        sortedList = push(&sortedList, current->processID, current->priority,time,id);
        //printf("reordering ... P%d\n",current->processID + 1);
        if(current->next == NULL)
            break;
        current = current->next;
    }
    *head = sortedList;
}

Node* push(Node **head,int id, int priority, int time, int executedId) {
    Node* temp = createNode(id,priority);
    if (isEmpty(head)) {
        (*head) = temp;
        processes[id].isPushed = true;
        return temp;
    }
    Node* start = (*head);
    if (compareTo(start,temp,executedId) < 0) {
        int oldId  = (*head)->processID;
        processes[oldId].arrivalTime = time;
        processes[oldId].quantumCount++;
        processes[oldId].executionTime = 0;

        if (processes[oldId].quantumCount >= typeConversions[processes[oldId].type] && processes[oldId].type != 2) {
            processes[oldId].type++;
            processes[oldId].quantumCount = 0;
        }

        temp->next = *head;
        (*head) = temp;
    } else {
        while (start->next != NULL && compareTo(start->next,temp,executedId) > 0) {
            start = start->next;
        }
        temp->next = start->next;
        start->next = temp;
    }
    processes[id].isPushed = true;
    return *head;
}

int compareTo(Node* old, Node* new, int id) {
    Process oldProcess = processes[old->processID];
    Process newProcess = processes[new->processID];
    
    if (oldProcess.type == 2 && (id == old->processID || newProcess.type != 2))
        return 1;

    if (newProcess.type == 2 && oldProcess.type != 2)
        return -1;

    int priorityDifference = oldProcess.priority - newProcess.priority;
    if (priorityDifference != 0)
        return priorityDifference;

    int arrivalTimeDifference = newProcess.arrivalTime - oldProcess.arrivalTime;
    if (arrivalTimeDifference != 0)
        return arrivalTimeDifference;

    char oldIDStr[4];  // Adjust the size based on your process ID length
    char newIDStr[4];
    sprintf(oldIDStr, "%d", old->processID + 1);
    sprintf(newIDStr, "%d", new->processID + 1);

    return strcmp(newIDStr,oldIDStr);
} 

int isEmpty(Node** head) {
    return (*head) == NULL; 
}

void readInstructions(char *filename) {
    FILE *file = fopen(filename, "r");
    int i = 0;
    while (fscanf(file, "%*s %d", &instructions[i]) != EOF && i < INSTRUCTION_NUMBER) {
        i++;
    }
    fclose(file);
}

void readProcessInstructions(char *filename, int index) {
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

void readProcessFiles() {
    for (int i = 0; i < 10; i++) {
        char filename[10];
        sprintf(filename, "P%d.txt", i + 1);
        readProcessInstructions(filename, i);
    }
}


void readDefinitionFile(char *filename) {
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

void addToQueue(int time,Node** head, int id) {
    for (int i = 0; i < processNumber; i++)
    {
        int index = activeProcesses[i];
        if (time >= processes[index].arrivalTime  && processes[index].arrivalTime >= 0 && !processes[index].isPushed) {
            push(head,index,processes[index].priority,time,id);
        }
    }
}

int findClosestArrivalTime(int time) {
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

float* calculateTimes() {
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