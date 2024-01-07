#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Process {
    int priority;
    int arrivalTime;
    int type;  // 0 for silver, 1 for gold and 2 for plat
    int* instructionInfo;
} Process;

typedef struct Node {
    int processID;
    int priority;
    struct Node* next;
} Node;

void readInstructions(char *filename);
void printIntArray(int arr[], int size);
void readProcessInstructions(char *filename, int index);
void readProcessFiles();
void readDefinitionFile(char *filename);
int peek();
Node* createNode(int id, int priority);
Node* pop(Node** head);
Node* push(Node **head,int id, int priority);
int isEmpty(Node** head);
int compareTo(Node* old, Node* new);
void cleanup();
void addToQueue(int time);
int findClosestArrivalTime(int time);

// global variables
Node* head = NULL;
int const INSTRUCTION_NUMBER = 21;
int instructions[21];
Process processes[10];
int* activeProcesses;
int processNumber = 0;

int main() {
    int time = 0;
    readInstructions("instructions.txt");
    readProcessFiles();
    readDefinitionFile("definition.txt");
    
    while (1)
    {
        addToQueue(time);
        printf("%d",head->processID);
        int nextArrival = findClosestArrivalTime(time);
    }
    
    return 0;
}

int peek() {
    return head->processID;
}

Node* createNode(int id, int priority) {
    Node* temp = (Node*) malloc(sizeof(Node));
    temp->processID = id;
    temp->priority = priority;
    temp->next =NULL;
}

Node* pop(Node** head) {
    Node* temp = *head;
    (*head) = (*head)->next;
    free(temp);
    return temp;
}

Node* push(Node **head,int id, int priority) {
    Node* temp = createNode(id,priority);
    if (isEmpty(head)) {
        (*head) = temp;
        return temp;
    }

    Node* start = (*head);
    if (compareTo(start,temp) < 0) {
        temp->next = *head;
        (*head) = temp;
    } else {
        while (start->next != NULL && compareTo(start->next,temp) > 0) {
            start = start->next;
        }
        temp->next = start->next;
        start->next = temp;
    }
    return *head;
}

int compareTo(Node* old, Node* new) {
    Process oldProcess = processes[old->processID];
    Process newProcess = processes[new->processID];

    if (newProcess.type == 2 && oldProcess.type != 2)
        return -1;
    else if ((newProcess.type != 2 && oldProcess.type != 2) || (newProcess.type == 2 && oldProcess.type == 2)) {
        int priorityDifference = newProcess.priority - oldProcess.priority;
        if (priorityDifference != 0)
            return priorityDifference;

        int arrivalTimeDifference = newProcess.arrivalTime - oldProcess.arrivalTime;
        if (arrivalTimeDifference != 0)
            return arrivalTimeDifference;

        return new->processID - old->processID;
    } else
        return 1;
} 

int isEmpty(Node** head) {
    return (*head) == NULL; 
}

void printIntArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
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
    }
}

void addToQueue(int time) {
    for (int i = 0; i < processNumber; i++)
    {
        int index = activeProcesses[i];
        if (time >= processes[index].arrivalTime)
            push(&head,index,processes[index].priority);
    }
}

int findClosestArrivalTime(int time) {
    int result = __INT_MAX__;
    for (int i = 0; i < processNumber; i++)
    {
        int current = processes[activeProcesses[i]].arrivalTime;
        if (time < current && current < time)
            result = current;
    }
    return result;
}