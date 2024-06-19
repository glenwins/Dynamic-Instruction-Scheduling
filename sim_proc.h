#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

class counterInfo{
public:
    int counter;
};

///Class used for each spot in the issue queue
class issues{
public:
    int readytime;
    int PC;
};
///Class used for stage in the pipeline
class stages{
public:
    int op;
    int dest;
    int src1;
    int src2;
    unsigned long PC;
    int prevRename;
    int clock;
    int RMTloc1;
    int RMTloc2;
    int issuetime;
    int excount;
    int prevexecutetime[500];
    int robfullpos;
    int previssuetime;
    int prevrenametime;
    int robposition;
    int counter;
    int prevregreadtime;
    int issueQueueLoc;
    int prevdispatchtime;
    int prevRetireStage;
    int numRetireStages[16];
    int retireTracker;
    int retirefinaltime;
    int endfile;

};

///Class used for RMT Table
class RMT{
public:
    bool valid;
    int tag;
};
///Class used for each spot in ROB Table
class ROB{
public:
    int dst;
    bool ready;
    int PC;
    int readytime;
    int readyPC;
    int assinged;
};
///function to set each value in each stage to 0;
void stagenullify(stages temp[], int width){
    for(int i=0; i<width; i++){
        temp[i].dest=NULL;
        temp[i].src1=NULL;
        temp[i].src2=NULL;
        temp[i].RMTloc1=0;
        temp[i].RMTloc2=0;
        temp[i].op=NULL;
        temp[i].prevRename=0;
        temp[i].prevrenametime=0;
        temp[i].robfullpos=0;
        temp[i].robposition=0;
        temp[i].PC=NULL;
        temp[i].clock=-1;
        temp[i].issuetime=0;
        temp[i].excount=0;
        temp[i].previssuetime=0;
        temp[i].prevregreadtime=0;
        temp[i].counter=-1;
        temp[i].issueQueueLoc=0;
        temp[i].prevdispatchtime=0;
        temp[i].prevRetireStage=0;
        temp[i].retireTracker=0;
        temp[i].retirefinaltime=0;
        temp[i].endfile=0;
        for(int j=0; j<500; j++){
            temp[i].prevexecutetime[j]=0;
        }
        for(int j=0; j<16; j++){
            temp[i].numRetireStages[j]=0;
        }
    }
}
/// Each pineline phase gets dest, src1, src2, op_type, and pc and carries it through the pipeline.
///fetch phase for the pipeline
void fetch(stages fetch[], int width, FILE *FP, RMT table[]){
    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint64_t pc; // Variable holds the pc read from input file
    for(int i=0; i<width; i++){

        if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2)==EOF) {
            for(int j=0; j<width; j++)fetch[j].endfile=1;
            break; }
        //printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2);
            fetch[i].dest=dest;
            fetch[i].src1=src1;
            fetch[i].src2=src2;
            fetch[i].op=op_type;
            fetch[i].PC=pc;
        for(int j=0; j<width; j++)fetch[j].counter++;
        //printf("%d\n",fetch[0].counter);

    }



}
///decode phase for the pipeline
void decode(stages fetch[], stages decode[], int width, RMT table[]){
    for(int i=0; i<width; i++){
        //if(decode[i].dest==NULL){
            decode[i].dest=fetch[i].dest;
            decode[i].src1=fetch[i].src1;
            decode[i].src2=fetch[i].src2;
            decode[i].op=fetch[i].op ;
            decode[i].PC=fetch[i].PC;
            fetch[i].dest=NULL;
            fetch[i].PC=0;

        //}
    }
}
///rename phase for the pipeline
void rename(stages rename[], stages decode[], RMT table[],ROB Reorder[], int robSize, int width){
    int donealready=0;
    for(int i=0; i<width; i++){

        rename[i].dest = decode[i].dest;
        rename[i].src1 = decode[i].src1;
        rename[i].src2 = decode[i].src2;
        rename[i].op = decode[i].op;
        rename[i].PC = decode[i].PC;
        ///Here, I implemented the RMT location of the sources because to save it in case another version of the register got put in the RMT table later.
        rename[i].RMTloc1 = decode[i].RMTloc1;
        decode[i].PC = 0;
        if (decode[i].src2 != -1)rename[i].RMTloc2 = decode[i].RMTloc2;
        decode[i].dest = NULL;
        Reorder[rename[i].robfullpos].dst = rename[i].dest;
        Reorder[rename[i].robfullpos].ready = 0;
        Reorder[rename[i].robfullpos].PC = rename[i].PC;
        ///The robfullpos cycles through the reorder buffer and fills the oldest spot
        rename[i].robposition=rename[i].robfullpos;
        if(rename[i].src1!=-1)rename[i].RMTloc1=table[rename[i].src1].tag;
        if(rename[i].src2!=-1)rename[i].RMTloc2=table[rename[i].src2].tag;
        if (rename[i].dest != -1) {
            table[rename[i].dest].tag = rename[i].robfullpos;
            table[rename[i].dest].valid = 1;
        }
        for(int k=0;k<width;k++) {
            ///Here I set each rename robfullpos to be the same so each width wide stage had the right value
            rename[k].robfullpos++;
            if (rename[k].robfullpos >= robSize)rename[k].robfullpos = 0;
        }


    }
}
///regread phase for the pipeline
void regread(stages regread[], stages rename[], int width){

    for(int i=0; i<width; i++){
        regread[i].dest=rename[i].dest;
        regread[i].src1=rename[i].src1;
        regread[i].src2=rename[i].src2;
        regread[i].op=rename[i].op ;
        regread[i].PC=rename[i].PC;
        regread[i].RMTloc1=rename[i].RMTloc1;
        if(rename[i].src2!=-1)regread[i].RMTloc2=rename[i].RMTloc2;
        regread[i].robposition=rename[i].robposition;
        rename[i].dest=NULL;
        rename[i].PC = 0;
    }

}
///dispatch phase for the pipeline
void dispatch(stages regread[], stages dispatch[], int width){
    for(int i=0; i<width; i++){
        dispatch[i].dest=regread[i].dest;
        dispatch[i].src1=regread[i].src1;
        dispatch[i].src2=regread[i].src2;
        dispatch[i].op=regread[i].op ;
        dispatch[i].PC=regread[i].PC;
        dispatch[i].RMTloc1=regread[i].RMTloc1;
        if(regread[i].src2!=-1)dispatch[i].RMTloc2=regread[i].RMTloc2;
        dispatch[i].robposition=regread[i].robposition;
        regread[i].dest=NULL;
        regread[i].PC=0;
    }
}
///issue phase for the pipeline, I was struggling to implement the IQ correctly here but i implemented it later
void issue(stages issue[], stages dispatch[], int width, issues IQ[], int iqsize, RMT table[], ROB reorder[], stages writeback[] ){
    for(int i=0; i<width; i++) {
        //printf("TEST\n");
        issue[i].dest = dispatch[i].dest;
        issue[i].src1 = dispatch[i].src1;
        issue[i].src2 = dispatch[i].src2;
        issue[i].op = dispatch[i].op;
        issue[i].PC = dispatch[i].PC;
        issue[i].RMTloc1=dispatch[i].RMTloc1;
        if(dispatch[i].src2!=-1)issue[i].RMTloc2=dispatch[i].RMTloc2;
        issue[i].robposition=dispatch[i].robposition;
        dispatch[i].dest=NULL;
        dispatch[i].PC=0;
    }

}
///execute phase for the pipeline
void execute(stages execute[], stages issue[], RMT table[], ROB Reorder[], int width, stages retire[]){
    for(int i=0; i<width; i++){
        if(issue[i].src2 != -1) {
            execute[i].issuetime++;
            execute[i].dest = issue[i].dest;
            execute[i].src1 = issue[i].src1;
            execute[i].src2 = issue[i].src2;
            execute[i].op = issue[i].op;
            execute[i].PC = issue[i].PC;
            execute[i].RMTloc1=issue[i].RMTloc1;
            execute[i].issueQueueLoc=issue[i].issueQueueLoc;
            if(issue[i].src2!=-1)execute[i].RMTloc2=issue[i].RMTloc2;
            execute[i].robposition=issue[i].robposition;
            issue[i].dest = NULL;
            issue[i].PC=0;
        }
        else{
            execute[i].dest = issue[i].dest;
            execute[i].src1 = issue[i].src1;
            execute[i].src2 = issue[i].src2;
            execute[i].op = issue[i].op;
            execute[i].PC = issue[i].PC;
            execute[i].RMTloc1=issue[i].RMTloc1;
            if(issue[i].src2!=-1)execute[i].RMTloc2=issue[i].RMTloc2;
            execute[i].robposition=issue[i].robposition;
            execute[i].issueQueueLoc=issue[i].issueQueueLoc;
            issue[i].dest = NULL;
            issue[i].PC=0;

        }
    }


}
///writeback phase for the pipeline
void writeback(stages execute[], stages writeback[], int robSize, ROB Reorder[], int width, issues IQ[], int iqsize){
    for(int i=0; i<width; i++) {
        writeback[i].dest = execute[i].dest;
        writeback[i].src1 = execute[i].src1;
        writeback[i].src2 = execute[i].src2;
        writeback[i].op = execute[i].op;
        writeback[i].PC = execute[i].PC;
        writeback[i].RMTloc1=execute[i].RMTloc1;
        if(execute[i].src2!=-1)writeback[i].RMTloc2=execute[i].RMTloc2;
        writeback[i].robposition=execute[i].robposition;
        writeback[i].issueQueueLoc=execute[i].issueQueueLoc;
        execute[i].dest=NULL;
        execute[i].PC=0;
    }
    for(int i=0; i<width; i++){
        ///Here is where the reorder buffer sets values to ready
        if (Reorder[writeback[i].robposition].PC == writeback[i].PC ) {
            Reorder[writeback[i].robposition].ready = 1;
            Reorder[writeback[i].robposition].readyPC=writeback[i].dest;
        }
    }

}
///retire phase for the pipeline, I set a lot of the values for the cycles here
void retire(int counter, stages fetch[], stages decode[], stages rename[], stages regread[], stages dispatch[],
            stages issue[], stages executive[], stages writeback[], stages retire[], int width, ROB reorder[],
            int robsize, counterInfo randomInfo, issues IQ[], int iqsize) {
    for(int i=0; i<width; i++) {
        //printf("%d %d\n",retire[0].counter,fetch[0].counter);

        for (int k = 0; k < width; k++)retire[k].counter++;
        if (retire[0].counter <= fetch[0].counter){
        retire[i].dest = writeback[i].dest;
        retire[i].src1 = writeback[i].src1;
        retire[i].src2 = writeback[i].src2;
        retire[i].op = writeback[i].op;
        retire[i].PC = writeback[i].PC;
        retire[i].RMTloc1 = writeback[i].RMTloc1;
        if (writeback[i].src2 != -1)retire[i].RMTloc2 = writeback[i].RMTloc2;
        retire[i].robposition = writeback[i].robposition;
        retire[i].issueQueueLoc = writeback[i].issueQueueLoc;
        writeback[i].dest = NULL;
        writeback[i].PC = 0;
        ///Once the correct values for the first width are found, fetch-decode are set for the rest
        if (i == 0) {
            /// each clock is incremented once from the last
            fetch[i].clock++;
            decode[i].clock = fetch[i].clock + 1;
            rename[i].clock = decode[i].clock + 1;
            ///the rename clock is incremented here if the regread clock is incremented below due to a full ROB
            for (int k = 0; k < width; k++) {
                if (retire[k].prevRename != 0 && i == 0) {
                    rename[i].clock = retire[i].prevregreadtime;
                    decode[i].clock = retire[i].prevrenametime;
                    retire[k].prevRename = 0;
                }
            }
            regread[i].clock = rename[i].clock + 1;
            if (retire[i].prevdispatchtime != 0)regread[i].clock = retire[i].prevdispatchtime;
            int roundROB = 0;
            int temp = 0;
            int clockminusrob = 0;
            ///This for loop checks to see if the rob is ready to accept the next instruction, if not, the regread clock goes up until its ready.
            for (int k = 0; k < width; k++) {
                temp = k;

                if ((retire[i].robposition + k) >= robsize)temp = k - robsize;
                clockminusrob = regread[i].clock - (reorder[retire[i].robposition + temp].readytime + 1);
                while (clockminusrob <= 0 && i == 0) {
                    regread[i].clock++;
                    retire[i].prevRename++;
                    clockminusrob++;

                }
            }

            dispatch[i].clock = regread[i].clock + 1;
            issue[i].clock = dispatch[i].clock + 1;

            ///if the previous issue time for an earth trace is higher, the current is increased
            for (int k = 0; k < width; k++) {
                while (retire[k].previssuetime >= issue[i].clock) {

                    issue[i].clock++;
                    dispatch[i].clock++;
                }

                if (retire[i].prevregreadtime != 0)rename[i].clock = retire[i].prevregreadtime;
                if (retire[i].prevrenametime != 0)decode[i].clock = retire[i].prevrenametime;
                while (retire[k].prevrenametime >= rename[i].clock) {
                    rename[i].clock++;
                    decode[i].clock++;
                    fetch[i].clock++;
                }
                while (fetch[i].clock + 1 < decode[i].clock)fetch[i].clock++;
            }
            /// there is where the ready values are checked for the IQ, if not ready, the issue clock is increased
            int lowest = IQ[0].readytime;
            int lowestmarker = 0;
            int noPC = 0;
            int actualLowest = 0;
            int lowestwidth[width];
            int found = 0;
            for (int j = 0; j < iqsize; j++) {
                if (IQ[j].PC == 0) {
                    lowestmarker = j;
                    noPC = 1;
                }
                if (noPC == 0) {
                    if (lowest >= IQ[j].readytime) {
                        lowest = IQ[j].readytime;
                        lowestmarker = j;
                    }
                }
            }


            lowest = IQ[0].readytime;
            actualLowest = lowestmarker;
            int storeLowest[width];
            int largestSpot = 0;
            int largestNum = -1;
            for (int k = 0; k < width; k++)storeLowest[k] = 9999;
            for (int j = 0; j < iqsize; j++) {
                largestNum = -1;
                for (int k = 0; k < width; k++) {
                    if (storeLowest[k] > largestNum) {
                        largestNum = storeLowest[k];
                        largestSpot = k;

                    }
                }
                for (int k = 0; k < width; k++) {
                    if (storeLowest[largestSpot] > IQ[j].readytime) {
                        storeLowest[largestSpot] = IQ[j].readytime;
                        break;
                    }
                }

            }
            int largest = -1;
            for (int k = 0; k < width; k++) {
                if (storeLowest[k] > largest)largest = storeLowest[k];
            }

            if (largest > issue[i].clock && largest != 9999) {
                issue[i].clock = largest;
                //printf("TEST\n");
            }


        } else {
            ///the following values of the clocks for widths>0 match the the first of the width
            fetch[i].clock = fetch[0].clock;
            decode[i].clock = decode[0].clock;
            rename[i].clock = rename[0].clock;
            regread[i].clock = regread[0].clock;
            dispatch[i].clock = dispatch[0].clock;
            issue[i].clock = issue[0].clock;
        }
        ///The following 2 if statements check the ready time for each of the sources and use it to determine execution time
        executive[i].clock = issue[i].clock + 1;
        if (reorder[retire[i].RMTloc1].readytime > executive[i].clock &&
            (reorder[retire[i].RMTloc1].readyPC == retire[i].src1) && retire[i].src1 != -1)
            executive[i].clock = reorder[retire[i].RMTloc1].readytime;

        if ((reorder[retire[i].RMTloc1].readytime <= reorder[retire[i].RMTloc2].readytime ||
             (reorder[retire[i].RMTloc2].readytime > executive[i].clock &&
              reorder[retire[i].RMTloc1].readyPC != retire[i].src1)) &&
            reorder[retire[i].RMTloc2].readyPC == retire[i].src2 && retire[i].src2 != -1)
            executive[i].clock = reorder[retire[i].RMTloc2].readytime;
        //
        retire[i].issuetime = executive[i].clock - issue[i].clock;
        ///This while loop checks to see if there are more than width instructions in a certain execution phase, if so it incremnents and checks again
        while (issue[i].clock >= executive[i].clock)executive[i].clock++;
        int temp1 = 0;
        int numCounted = 0;
        while (temp1 == 0) {
            numCounted = 0;
            for (int k = 0; k < width; k++) {
                for (int j = 0; j < 500; j++) {
                    if (retire[k].prevexecutetime[j] == executive[i].clock) {
                        numCounted++;
                    }
                }
            }
            if (numCounted >= width) {

                executive[i].clock++;
            } else {
                temp1 = 1;
            }
        }
        ///This increments the writeback clock based on the execution type
        if (retire[i].op == 0)writeback[i].clock = executive[i].clock + 1;
        if (retire[i].op == 1)writeback[i].clock = executive[i].clock + 2;
        if (retire[i].op == 2)writeback[i].clock = executive[i].clock + 5;
        retire[i].clock = writeback[i].clock + 1;
        int lowest = IQ[0].readytime;
        int lowestmarker = 0;
        int noPC = 0;
        ///This increments the IQ, it finds the lowest value of readytime and replaces it
        reorder[retire[i].robposition].readytime = writeback[i].clock;
        for (int j = 0; j < iqsize; j++) {
            if (IQ[j].PC == 0) {
                lowestmarker = j;
                noPC = 1;
            }

            if (noPC == 0) {
                if (lowest >= IQ[j].readytime) {
                    lowest = IQ[j].readytime;
                    lowestmarker = j;
                }
            }
        }
        if (retire[i].counter > 0) {
            for (int j = 0; j < iqsize; j++) {
            }
            //printf("\n");
        }
        IQ[lowestmarker].PC = retire[i].PC;
        IQ[lowestmarker].readytime = executive[i].clock;

        int retirelength=1;
        if(i!=0) {
            //printf("%d %d\n",(retire[i].clock),retire[i].prevRetireStage);
            if((retire[i].clock+retirelength)<retire[i].prevRetireStage){
                retirelength=retire[i].prevRetireStage-retire[i].clock;

            }

        }
        ///This is where I calculated the amount of cycles for each retire phase.  If their were more than width of a specific value, it incremented
        for(int j=0;j<width;j++) {
            while((retire[i].clock + retirelength) < retire[j].prevRetireStage)retirelength++;
        }
            for(int j=0;j<width;j++){
                retire[j].prevRetireStage=retire[i].clock+retirelength;
            }

            int countRetires=0;
            for(int j=0; j<width; j++){
                if(retire[i].numRetireStages[j]==retire[i].prevRetireStage && retire[i].prevRetireStage!=0){
                    countRetires++;
                }
                if(countRetires>=width){
                    retirelength++;
                    retire[i].prevRetireStage++;
                    break;
                }

            }
        for(int j=0; j<width; j++){
            retire[j].numRetireStages[retire[j].retireTracker]=retire[i].clock + retirelength;
            retire[j].retireTracker++;
            if(retire[j].retireTracker>=width)retire[j].retireTracker=0;

        }
        //if(retire[i].counter>9900)
            printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d} \n",
                   retire[i].counter, retire[i].op, retire[i].src1, retire[i].src2, retire[i].dest, fetch[i].clock,
                   decode[i].clock - fetch[i].clock, decode[i].clock, rename[i].clock - decode[i].clock,
                   rename[i].clock, regread[i].clock - rename[i].clock, regread[i].clock,
                   dispatch[i].clock - regread[i].clock, dispatch[i].clock, issue[i].clock - dispatch[i].clock,
                   issue[i].clock, executive[i].clock - issue[i].clock, executive[i].clock,
                   writeback[i].clock - executive[i].clock, writeback[i].clock, retire[i].clock - writeback[i].clock,
                   retire[i].clock, retirelength);
        retire[i].prevexecutetime[retire[i].excount] = executive[i].clock;
        retire[i].previssuetime = issue[i].clock;
        retire[i].prevrenametime = rename[i].clock;
        retire[i].prevregreadtime = regread[i].clock;
        retire[i].prevdispatchtime = dispatch[i].clock;
        retire[i].excount++;
        if (retire[i].excount >= 500)retire[i].excount = 0;
        for(int j=0;j<width;j++)retire[j].retirefinaltime=retire[i].clock+retirelength;

    }
    }

}


// Put additional data structures here as per your requirement

#endif
