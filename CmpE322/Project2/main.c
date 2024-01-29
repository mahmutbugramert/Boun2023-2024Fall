#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define MAX_INPUT_SIZE 1024

// process struct
typedef struct Process{
    int id; // name of the process such as 1,2,... const
    int priority; // priority of the process const
    int arrivalTime; // arrival time of the processs const
    char type[10]; // type of the process such as PLATINUM variable
    int quantum; // time quantum of the process according to its type variable
    int quantumCounter; // quantum counter for passing to gold or platinum variable
    int currentInst; // next executed instruction of the process variable
    int leftTime; // how many times left for the process in a time quantum variable
    int burstTime; // burst time of the process
    bool isPreempted; // true if the process is preempted
} Process;

int findTheProcessIndex(int id, Process* processes, int numOfProcesses){// find the index of the process whose id is given. If it does not exist(terminated), then return -1
    for(int i = 0; i < numOfProcesses; i++){
        if(processes[i].id == id){
            return i;
        }
    }
    return -1;
}

void sortProcesses(Process* processes, int numOfProcesses){ // sort the processes for the first time according to criteria
    for(int i = 0; i < numOfProcesses; i++){
        for(int j = i + 1; j < numOfProcesses; j++){
            if(strcmp(processes[i].type, "PLATINUM") == 0){
                if(strcmp(processes[j].type, "PLATINUM") == 0){
                    if(processes[i].priority > processes[j].priority){
                        continue;
                    }
                    else if((processes[i].priority == processes[j].priority)){
                        if(processes[i].arrivalTime < processes[j].arrivalTime){
                            continue;
                        }
                        else if(processes[i].arrivalTime == processes[j].arrivalTime){
                            if(processes[i].id < processes[j].id){
                                continue;
                            }
                            else{
                                Process temp = processes[i];
                                processes[i] = processes[j];
                                processes[j] = temp;
                            }
                        }
                        else{
                            Process temp = processes[i];
                            processes[i] = processes[j];
                            processes[j] = temp;
                        }
                    }    
                    else{
                        Process temp = processes[i];
                        processes[i] = processes[j];
                        processes[j] = temp;
                    }
                }
                else{
                    continue;
                }
            }
            else{
                if(strcmp(processes[j].type, "PLATINUM") == 0){
                    Process temp = processes[i];
                    processes[i] = processes[j];
                    processes[j] = temp;
                }
                else if(processes[i].priority > processes[j].priority){
                    continue;
                }
                else if(processes[j].priority == processes[i].priority){
                    if(processes[i].arrivalTime < processes[j].arrivalTime){
                        continue;
                    }
                    else if(processes[i].arrivalTime == processes[j].arrivalTime){
                        if(processes[i].id < processes[j].id){
                            continue;
                        }
                        else{
                            Process temp = processes[i];
                            processes[i] = processes[j];
                            processes[j] = temp;
                        }
                    }
                    else{
                        Process temp = processes[i];
                        processes[i] = processes[j];
                        processes[j] = temp;
                    }
                }
                else{
                    Process temp = processes[i];
                    processes[i] = processes[j];
                    processes[j] = temp;
                }
            }
        } 
    }
}

void roundRobin(Process* processes, int numOfProcesses, int id, int time){ // round robin the same priority processes in the processes array
    for(int a = 0; a < numOfProcesses; a++){
        if(processes[a].id == id){
            for(int b = a; a < numOfProcesses; b++){
                if(b+1 < numOfProcesses && processes[b+1].priority == processes[b].priority && processes[b+1].arrivalTime < time){
                    Process temp = processes[b];
                    processes[b] = processes[b+1];
                    processes[b+1] = temp;
                }
                else if(b+1 < numOfProcesses && processes[b+1].priority == processes[b].priority && processes[b+1].arrivalTime == time){
                    if(processes[b].id < processes[b+1].id){
                        continue;
                    }
                    else{
                        Process temp = processes[b];
                        processes[b] = processes[b+1];
                        processes[b+1] = temp;
                    }
                }
                else{
                    break;
                }
            }
            break;
        }
    }
}
void sortPlatinums(Process* processes, int numOfProcesses, int time){ // sort the platinum processes in the processes array
    for(int m = numOfProcesses - 1; m > 0; m--){
        int n = m - 1;
        if(strcmp(processes[n].type, "PLATINUM") != 0 && strcmp(processes[m].type, "PLATINUM") == 0){
            for(int l = m; l > 0; l--){
                if(strcmp(processes[l-1].type, "PLATINUM") != 0){
                    Process temp = processes[l];
                    processes[l] = processes[l-1];
                    processes[l-1] = temp;
                }
                else{
                    break;
                }
            }
        }
    }
    for(int x = 0; x < numOfProcesses; x++){
        for(int y = x+1; y < numOfProcesses; y++){
            if(strcmp(processes[y].type, "PLATINUM") == 0){ 
                if(processes[x].priority > processes[y].priority){
                    continue;
                }
                else if(processes[x].priority == processes[y].priority){
                    if(processes[x].arrivalTime < processes[y].arrivalTime){
                        continue;
                    }
                    else if(processes[x].arrivalTime == processes[y].arrivalTime){
                        if(processes[x].id < processes[y].id){
                            continue;
                        }
                        else{
                            Process temp = processes[x];
                            processes[x] = processes[y];
                            processes[y] = temp;
                        }
                    }
                    else{
                        Process temp = processes[x];
                        processes[x] = processes[y];
                        processes[y] = temp;
                    }
                }
            }
            else{
                break;
            }
        }    
    }
}
int executePro(Process* processes, int index, int instructions[], int process[]);
int chooseTakenTime(Process* processes, int i, int instructions[], int p1[], int p2[], int p3[], int p4[], int p5[], int p6[], int p7[], int p8[], int p9[], int p10[]){ // determine execute time
    int takenTime;
    switch (processes[i].id){
            case 1:
                takenTime = executePro(processes, i, instructions, p1);
                break;
            case 2:
                takenTime = executePro(processes, i, instructions, p2);
                break;
            case 3:
                takenTime = executePro(processes, i, instructions, p3);
                break;
            case 4:
                takenTime = executePro(processes, i, instructions, p4);
                break;
            case 5:
                takenTime = executePro(processes, i, instructions, p5);
                break;
            case 6:
                takenTime = executePro(processes, i, instructions, p6);
                break;
            case 7:
                takenTime = executePro(processes, i, instructions, p7);
                break;
            case 8:
                takenTime = executePro(processes, i, instructions, p8);
                break;
            case 9:
                takenTime = executePro(processes, i, instructions, p9);
                break;
            case 10:
                takenTime = executePro(processes, i, instructions, p10);
                break;
            default:
                break;
            }
    return takenTime;
}

int executePro(Process* processes, int index, int instructions[], int process[]){ // choose which instruction to execute
    int timeOfInst = instructions[process[processes[index].currentInst]];
    processes[index].currentInst += 1;
    processes[index].burstTime += timeOfInst;
    return timeOfInst;
}

int deleteProcess(Process* processes, int index, int numOfProcesses){ // delete the process which is exited from the processes array
    for(int i = index + 1; i < numOfProcesses; i++){
        processes[i - 1] = processes[i];
    }
    numOfProcesses--;
    processes = (Process*) realloc(processes, sizeof(Process) * numOfProcesses);
    return numOfProcesses;
}

int main(){
    //HARDCODE PROCESS FILES
    int p1[12] = {1, 19, 15, 18, 3, 2, 20, 15, 18, 3, 2, 0};
    int p2[9] = {18, 2, 5, 6, 5, 6, 5, 6, 0};
    int p3[10] = {8, 7, 12, 11, 13, 16, 19, 8, 7, 0};
    int p4[6] = {9, 2, 19, 9, 2, 0};
    int p5[11] = {9, 2, 19, 9, 2, 2, 19, 9, 2, 19, 0};
    int p6[15] = {10, 9, 20, 11, 4, 5, 7, 10, 9, 20, 11, 4, 5, 7, 0};
    int p7[6] = {8, 1, 10, 11, 2, 0};
    int p8[5] = {14, 4, 3, 1, 0};
    int p9[6] = {19, 12, 9, 1, 7, 0};
    int p10[10] = {20, 3, 19, 5, 2, 11, 8, 13, 14, 0};
    FILE* file;

    file = fopen("definition.txt", "r");

    if(file == NULL){
        printf("No file named definition.txt!\n");
    }

    char line[MAX_INPUT_SIZE];
    int numOfProcesses = 0; // number of processes (variable)
    int numOfProcessesCal = 0; // number of processes for later calculations (constant)
    Process* processes = (Process*) malloc(sizeof(Process) * 10); // process array

    int arrivalTimes[11] = {0,0,0,0,0,0,0,0,0,0,0}; // store arrival times of processes

    while(fgets(line, MAX_INPUT_SIZE, file)){
        char* tokens[4];
        char* token = strtok(line, " ");
        int counter = 0;
        while(token != NULL){
            tokens[counter] = token;
            token = strtok(NULL, " ");
            counter ++;
        }
        Process newProcess;
        //store id as int
        char id[3];
        strcpy(id, tokens[0]);
        char* tempId = &id[1];
        strcpy(id, tempId);
        newProcess.id = atoi(id);
        //priority and arrivaltime
        newProcess.priority =  atoi(tokens[1]);         
        newProcess.arrivalTime = atoi(tokens[2]);
        //type
        strcpy(newProcess.type, tokens[3]);
        for(int i = 0; i < strlen(tokens[3]); i++){
            if(tokens[3][i] ==  '\n'){
                tokens[3][i] = '\0';
            }
        }
        if(strcmp(tokens[3], "PLATINUM") == 0){
            newProcess.type[8] = '\0';
        }
        else if(strcmp(tokens[3], "GOLD") == 0){
            newProcess.type[4] = '\0';
        }
        else if(strcmp(tokens[3], "SILVER") == 0){
            newProcess.type[6] = '\0';
        }
        //other data fields
        newProcess.quantumCounter = 0;
        newProcess.currentInst = 0;
        newProcess.burstTime = 0;
        newProcess.isPreempted = false;
        if(strcmp(newProcess.type, "PLATINUM") == 0 || strcmp(newProcess.type, "GOLD") == 0){
            newProcess.quantum = 120;
            newProcess.leftTime = 120;
        }
        else{
            newProcess.quantum = 80;
            newProcess.leftTime = 80;
        }

        arrivalTimes[newProcess.id] = newProcess.arrivalTime;
        processes[numOfProcesses] = newProcess;
        numOfProcesses++;
    }
    numOfProcessesCal = numOfProcesses;
    processes = (Process*) realloc(processes, sizeof(Process) * numOfProcesses); // realloc the process array as the number of processes
    
    //HARDCODE INSTRUCTIONS
    int instructions[21] = {10, 90, 80, 70, 60, 50, 40, 30, 20, 30, 40, 50, 60, 70, 80, 90, 80, 70, 60, 50, 40};

    int time = 0; // time is zero at first
    int previousID = -1;// id of previous process initialized as -1 at first

    int timeOfCompletions[11] = {0,0,0,0,0,0,0,0,0,0,0}; // copmletion times of processes
    int bursTimes[11] = {0,0,0,0,0,0,0,0,0,0,0};// burst times of processes

    sortProcesses(processes, numOfProcesses); // sort the processes before the while loop

    while(true){
        int prevIndex = findTheProcessIndex(previousID, processes, numOfProcesses);
        
        int i = 0;
        if(previousID != -1 && prevIndex != -1 && strcmp(processes[prevIndex].type, "PLATINUM") == 0 && processes[prevIndex].burstTime > 0){ // if platinum process is in the CPU, make sure it continues its burst.
            i = prevIndex;
        }
        else{
            for(i; i < numOfProcesses; i++){ // check the process which will be executed among the arrived ones
                if(processes[i].arrivalTime > time){
                    continue;
                }
                else{
                    break;
                }
            }
        }   

        if(i >= numOfProcesses){ // if CPU is idle, increase the time accordingly
            int newTime = processes[0].arrivalTime;
            for(int k = 0; k < numOfProcesses; k++){
                if(processes[k].arrivalTime < newTime){
                    newTime = processes[k].arrivalTime;
                }
            }
            if(newTime - time <= 10){
                time += 10;
            }
            else{
                time = newTime;
            }
            continue;
        }

        int currentProcessId = processes[i].id;
        if(time == 0){
            time += 10; // context switch 
        }
        if(previousID != processes[i].id && time != 10){ // there is process change here, add context switch time, and do other necessary things
            time += 10; // context switch
            int prevIndex = findTheProcessIndex(previousID, processes, numOfProcesses); // find the index of previous process

            if(prevIndex != -1 && !processes[prevIndex].isPreempted){ // if previous process preemted because of higher priority process enter here
                processes[prevIndex].quantumCounter++;
                processes[prevIndex].leftTime = processes[prevIndex].quantum; // set the left time of the previous process again to time quantum
                if(strcmp(processes[prevIndex].type, "SILVER") == 0 && processes[prevIndex].quantumCounter == 3){
                    strcpy(processes[prevIndex].type, "GOLD");
                    processes[prevIndex].quantum = 120;
                    processes[prevIndex].leftTime = 120;
                    processes[prevIndex].quantumCounter = 0;
                }
                else if(strcmp(processes[prevIndex].type, "GOLD") == 0 && processes[prevIndex].quantumCounter == 5){
                    strcpy(processes[prevIndex].type, "PLATINUM");
                    processes[prevIndex].quantumCounter = 0;
                }
                if(strcmp(processes[prevIndex].type, "PLATINUM") == 0){ // if previous process upgraded to the platinum, sort the platinums
                    sortPlatinums(processes, numOfProcesses, time);
                }
                else{// else do the round robin
                    roundRobin(processes, numOfProcesses, previousID, time);
                }
            }   
            else{
                processes[prevIndex].isPreempted = false;
                
            } 
            i = 0;
            for(i; i < numOfProcesses; i++){ // check the process which will be executed among the arrived ones
                if(processes[i].arrivalTime > time){
                    continue;
                }
                else{
                    break;
                }
            }
            if(processes[i].id == previousID){// if process is the same cancel cs
                time -= 10; // cancel the context switch
            }
        }
        else if(previousID == processes[i].id){ // if same process in the CPU execute here
            currentProcessId = processes[i].id;
            int k = 0;
            if(processes[i].leftTime <= 0){ // if quantum ends do refresh leftTime, increase quantum counter, and control upgrades
                processes[i].leftTime = processes[i].quantum;
                processes[i].quantumCounter++;
                if(strcmp(processes[i].type, "SILVER") == 0 && processes[i].quantumCounter == 3){
                    strcpy(processes[i].type, "GOLD");
                    processes[i].quantum = 120;
                    processes[i].leftTime = 120;
                    processes[i].quantumCounter = 0;
                }
                else if(strcmp(processes[i].type, "GOLD") == 0 && processes[i].quantumCounter == 5){
                    strcpy(processes[i].type, "PLATINUM");
                    processes[i].quantumCounter = 0;
                }
                if(strcmp(processes[i].type, "PLATINUM") == 0){// if previous process upgraded to the platinum, sort the platinums
                    sortPlatinums(processes, numOfProcesses, time);
                }
                else{// else do the round robin
                    roundRobin(processes, numOfProcesses, currentProcessId, time);
                }
   
                for(k; k < numOfProcesses; k++){ // check the process which will be executed among the arrived ones
                    if(processes[k].arrivalTime > time){
                        continue;
                    }
                    else{
                        break;
                    }
                }
                if(processes[k].id != currentProcessId && processes[k].arrivalTime <= time){   
                    int index = findTheProcessIndex(currentProcessId, processes, numOfProcesses);
                    previousID = currentProcessId;
                    processes[index].isPreempted = true;
                    // processes[index].leftTime = processes[index].quantum;
                    continue;
                }
            }
        }
        int index = findTheProcessIndex(currentProcessId, processes, numOfProcesses);
        previousID = currentProcessId;
        int takenTime = chooseTakenTime(processes, index, instructions, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);

        time += takenTime;// increase time as instruction time
        processes[index].leftTime -= takenTime;//decrease lefttime as instruction time
        if(takenTime == 10){ // delete the process if exit is executed
            timeOfCompletions[processes[index].id] = time;
            bursTimes[processes[index].id] = processes[index].burstTime;
            numOfProcesses = deleteProcess(processes, index, numOfProcesses);
        }

        if(numOfProcesses == 0){
            break;
        }
        continue; 
    }
    //FINAL CALCULATIONS
    float turnAroundTime = 0;
    float waitingTime = 0;
    float AvgTurnAroundTime = 0;
    float AvgWaitingTime = 0;
    for(int i = 0; i < 11; i ++){
        turnAroundTime += timeOfCompletions[i] - arrivalTimes[i];
        waitingTime -= bursTimes[i];
    }
    waitingTime += turnAroundTime;
    AvgTurnAroundTime = turnAroundTime/numOfProcessesCal;
    AvgWaitingTime = waitingTime/numOfProcessesCal;
    if((float)(int)AvgTurnAroundTime == AvgTurnAroundTime && (float)(int)AvgWaitingTime == AvgWaitingTime){
        printf("%d\n%d\n", (int)AvgWaitingTime, (int)AvgTurnAroundTime);
    }
    else if((float)(int)AvgTurnAroundTime == AvgTurnAroundTime && (float)(int)AvgWaitingTime != AvgWaitingTime){
        printf("%.1f\n%d\n", AvgWaitingTime, (int)AvgTurnAroundTime);
    }
    else if((float)(int)AvgTurnAroundTime != AvgTurnAroundTime && (float)(int)AvgWaitingTime == AvgWaitingTime){
        printf("%d\n%.1f\n", (int)AvgWaitingTime, AvgTurnAroundTime);
    }
    else{
        printf("%.1f\n%.1f\n", AvgWaitingTime, AvgTurnAroundTime);
    }
}