#include <stdio.h> 
#include <string.h> 
#include <mpi.h>
#include <math.h>
#include <stdbool.h> 

bool isPowerOfTwo(int n);

int main(void) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL); 

    // Get the number of processes
    int comm_sz; 
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Array and Global Sum
    //int array[8] = {8, 19, 7, 15, 7, 13, 12, 14};
    //int array[16] = {8, 19, 7, 15, 7, 13, 12, 14, 1, 2, 3, 4, 5, 6, 7, 8};
    int array[32] = {8, 19, 7, 15, 7, 13, 12, 14, 1, 2, 3, 4, 5, 6, 7, 8, 8, 19, 7, 15, 7, 13, 12, 14, 1, 2, 3, 4, 5, 6, 7, 8};
    int globalsum = 0;
    
    // If core is not the main core
    if (my_rank != 0) {
        // Odd-numbered cores (Core #1, 3, 5, etc) send number to the core (Core # 0 and 1, 2 and 4, and etc) that is paired with
        if (my_rank % 2 != 0) {
             printf("Processor #%d: Sending %d to Processor #%d!\n", my_rank, array[my_rank], my_rank - 1);
             MPI_Send(&array[my_rank], 1, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD);
        }
        // For even numbered cores
        else if (my_rank % 2 == 0) {        
            globalsum += array[my_rank];
            
            // Core #2 receives number from Core #3, Core #4 receives number from Core #5, Core#6 receives number from Core #7
            int number;
            MPI_Recv(&number, 1, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            globalsum += number;
            printf("Processor #%d: Received %d from Processor #%d. My partial sum is %d.\n", my_rank, number, my_rank + 1, globalsum);

            // Core # 4, 8, 12, 16, 20, 24, 28, 32, etc
            if (my_rank % 4 == 0) {
                // Core #0 should receive stuff from Core #4, Core#8, Core #16, etc
                if (isPowerOfTwo(my_rank)) {
                    int i = 1;
                    int max_receives = my_rank / 4;

                    int coreNumToReceive = my_rank + pow(2, i);
                    // Core #4 receives number from Core #6
                    // Core #8 receives number from Core #10 and 12
                    // Core #16 receives number from Core #18, 20, #24
                    while (i <= max_receives && coreNumToReceive < comm_sz) {
                        int number;
                        MPI_Recv(&number, 1, MPI_INT, coreNumToReceive, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        globalsum += number;
                        printf("Processor #%d: Received %d from Processor #%d. My partial sum is %d.\n", my_rank, number, coreNumToReceive, globalsum);
                        i++;
                        coreNumToReceive  = my_rank + pow(2, i);
                    }

                    MPI_Send(&globalsum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    printf("Processor #%d: Sending %d to Processor #%d.\n", my_rank, globalsum, 0); 
                }
                // Core #12, 20, 24
                // Core #12 receives from #14. Core #12 sends number to #8. 12/4 = 3
                // Core #20 receives from #22. Core #20 sends number to #16. 20/4 = 5
                // Core #24 receives from #26 and #28. Core #24 sends number to #16. 24/4 = 6
                // Core #28 receives from #30. Core #28 sends number to #24. 28/4 = 7
                else {
                    // Even divisions have two receives, odd divsions have one receives
                    int max_receives;
                    if ((my_rank / 4) % 2 == 0) {
                        max_receives = 2;
                    }
                    else {
                        max_receives = 1;
                    }
                    
                    int i = 1;
                    while (i <= max_receives) {
                        int number;
                        MPI_Recv(&number, 1, MPI_INT, my_rank + pow(2, i), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        globalsum += number;
                        printf("Processor #%d: Received %d from Processor #%d. My partial sum is %d.\n", my_rank, number, my_rank + 2, globalsum);
                        i++;
                    }

                    MPI_Send(&globalsum, 1, MPI_INT, my_rank - 4 * max_receives, 0, MPI_COMM_WORLD);
                    printf("Processor #%d: Sending %d to Processor #%d.\n", my_rank, globalsum, my_rank - 4 * max_receives); 
                } 
            }
            // Core #2 sends number to Core #0, Core #6 sends number to Core #4, Core #10 sends number to Core #8
            else {
                printf("Processor #%d: Sending %d to Processor #%d!\n", my_rank, globalsum, my_rank - 2);
                MPI_Send(&globalsum, 1, MPI_INT, my_rank - 2, 0, MPI_COMM_WORLD);
            }
        }
    }
    // If core is the main core
    else {
        globalsum += array[my_rank];
        // Receive numbers from other cores (from Core #1, 2, 4, 8, 16, 32, 64, etc)
        int i = 0;
        int coreNum = pow(2, i);
        while (coreNum < comm_sz) {
            int number;
            MPI_Recv(&number, 1, MPI_INT, coreNum, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            globalsum += number;
            printf("Processor #%d. Received %d from Processor #%d. My partial sum is %d.\n", my_rank, number, coreNum, globalsum); 
            i++;
            coreNum = pow(2, i);
        }

        printf("GLOBAL SUM: %d.", globalsum);
    }

    // Clean up MPI environment
    MPI_Finalize();
    return 0; 
}

/* Function to check if x is power of 2*/
bool isPowerOfTwo(int n) 
{ 
   if(n==0) 
    return false; 
  
   return (ceil(log2(n)) == floor(log2(n))); 
} 