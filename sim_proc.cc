#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <queue>
#include "sim_proc.h"
using namespace std;
/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/


int main (int argc, char* argv[])
{
    /*argc=5;
    argv[0] = "sim";
    argv[1] = "512";
    argv[2] = "64";
    argv[3] = "7";
    argv[4] = "C:\\Users\\glent\\CLionProjects\\ECE-463-Project3\\val_trace_perl1";*/
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint64_t pc; // Variable holds the pc read from input file

    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    /*printf("rob_size:%lu "
           "iq_size:%lu "
           "width:%lu "
           "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);*/
    // Open trace_file in read mode
    issues issueQueue[params.iq_size];
    stages fetchstage[params.width];
    stages decodestage[params.width];
    stages renamestage[params.width];
    stages regreadstage[params.width];
    stages dispatchstage[params.width];
    stages issuestage[params.width];
    stages executestage[params.width];
    stages writebackstage[params.width];
    stages retirestage[params.width];
    RMT RMTtable[67];
    ROB ROB[params.rob_size];
    counterInfo randomInfo;


    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    for(int i=0; i<params.rob_size; i++){
        ROB[i].ready=true;
        ROB[i].dst=NULL;
        ROB[i].PC=0;
        ROB[i].readytime=0;
        ROB[i].assinged=0;
        ROB[i].readyPC=0;

    }
    for(int i=0;i<params.iq_size;i++){
        issueQueue[i].readytime=-1;
        issueQueue[i].PC=0;

    }
    for(int i=0; i<67; i++){
        RMTtable[i].tag=0;
        RMTtable[i].valid=false;

    }

    stagenullify(fetchstage, params.width);
    stagenullify(decodestage, params.width);
    stagenullify(renamestage, params.width);
    stagenullify(regreadstage, params.width);
    stagenullify(dispatchstage, params.width);
    stagenullify(issuestage, params.width);
    stagenullify(executestage, params.width);
    stagenullify(writebackstage, params.width);
    stagenullify(retirestage, params.width);
    randomInfo.counter=-1;
    int counter=0;
    int filedone=0;
    while(retirestage[0].counter<fetchstage[0].counter || retirestage[0].counter==-1) {
        //printf("%d %d\n",retirestage[0].counter,fetchstage[0].counter);
        if(writebackstage[0].dest!=0 /*&& retirestage[0].counter<9999*/) {
            retire(counter, fetchstage, decodestage, renamestage, regreadstage, dispatchstage,
                   issuestage, executestage, writebackstage, retirestage, params.width, ROB, params.rob_size, randomInfo, issueQueue, params.iq_size);
            counter++;
        }
        if(executestage[0].PC!=0) writeback(executestage, writebackstage, params.rob_size, ROB, params.width, issueQueue, params.iq_size);
        if(issuestage[0].PC!=0) execute(executestage, issuestage, RMTtable, ROB, params.width, retirestage);
        if(dispatchstage[0].PC!=0) issue(issuestage, dispatchstage, params.width, issueQueue, params.iq_size, RMTtable, ROB, writebackstage);
        if(regreadstage[0].PC!=0) dispatch(regreadstage, dispatchstage, params.width);
        if(renamestage[0].PC!=0) regread(regreadstage, renamestage, params.width);
        if(decodestage[0].PC!=0) rename(renamestage, decodestage, RMTtable, ROB, params.rob_size, params.width);
        if(fetchstage[0].PC!=0) decode(fetchstage, decodestage, params.width, RMTtable);
        fetch(fetchstage, params.width, FP, RMTtable);

    }

    printf("# === Simulator Command =========\n"
           "# ./sim %d %d %d %s\n"
           "# === Processor Configuration ===\n"
           "# ROB_SIZE = %d\n"
           "# IQ_SIZE  = %d\n"
           "# WIDTH    = %d\n",params.rob_size,params.iq_size,params.width,trace_file,params.rob_size,params.iq_size,params.width);
    printf("# === Simulation Results ========\n"
           "# Dynamic Instruction Count    = %d\n"
           "# Cycles                       = %d\n"
           "# Instructions Per Cycle (IPC) = %.2f\n",fetchstage[0].counter+1,retirestage[0].retirefinaltime,float(retirestage[params.width-1].counter)/float(retirestage[params.width-1].retirefinaltime));




    /*while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF) {
        //printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correct

    }*/
    return 0;
}
