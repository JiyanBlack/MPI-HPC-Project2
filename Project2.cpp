//Author:
//YanJi, 21824073, CITS5507
//JuanLu, 21691401, CITS3402

//Compile with g++ -fopenmp project1.cpp

//Results are recorded in the result.txt file. Logs are in the log.txt file.

//References:
//http://stackoverflow.com/questions/3501338/c-read-file-line-by-line
//http://stackoverflow.com/questions/4513316/split-string-in-c-every-white-space
//https://gist.github.com/codeblocks/952063

//send (data[500][4400], keys[4400]),

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>

#define length_of_array 30000000
//setting: provdie the core number of CPU
#define core_number 12
#define MASTER 0

void readData();
void readKeys();
void getBlocks(int col);
int isNeighbour(int i, int j, int col);
void printCol(int col);
void calcSignatures(int col);
long getOneSignature(int row1, int row2, int row3, int row4);
void setSignature(int col, int index, long value, int one, int two, int three, int four);
long getStartPoint(int col);
void quicksort(long x[], long first, long last);
long allocateMemory(int col);
int isInArray(int array[], int value);

//static data structures
static double data[500][4400];           //original data
static long keys[4400];                  //the keys of each row
static const double dia = 0.000001;      //the value of dia
static long filled_signature[500] = {0}; //count the already calculated signature number of each column
//MPI common variable, shared by all tasks
static long signature_number[500] = {0}; //the total signature number of each column
static long start_point[500] = {0};      //management of the start and end index of each column
static int total_col_has_blocks = 0;     //total number of columns that have blocks
static long total_block_number = 0;      //total number of blocks in all columns
static long collision_number = 0;        //total collision number
//MPI variable used by task 0
static int *signatures_one = new int[length_of_array];   //record the first element of each block
static int *signatures_two = new int[length_of_array];   //record the second element of each block
static int *signatures_three = new int[length_of_array]; //record the third element of each block
static int *signatures_four = new int[length_of_array];  //record the fourth element of each block
static int *correspond_col = new int[length_of_array];   //record the column number of each block
static long *signatures = new long[length_of_array];     //signatures of all rows in a one-dimensinal array

int main(void)
{
    int numtasks, taskid, len, rc;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int partner, message;
    long individual_index;
    MPI_Status status;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Get_processor_name(hostname, &len);
    //read data:
    readData();
    readKeys();
    //split col index into different nodes
    int col_each_task = (int)(500 / (numtasks - 1)) + 1;
    if (taskid != MASTER)
    {
        //allocate memory for the blocks of each column and sending to MASTER
        for (int col = (taskid - 1) * col_each_task; col < taskid * col_each_task && col < 500; col++)
        {
            individual_index = allocateMemory(col);
            rc = MPI_Send(&individual_index, 1, MPI_LONG, 0, col, MPI_COMM_WORLD);
            if (rc != MPI_SUCCESS)
                printf("%d: Send index failure\n", taskid);
        }
        //receiving signature_number and start_point array
        rc = MPI_Recv(&signature_number, 500, MPI_LONG, 0, 501, MPI_COMM_WORLD, &status);
        if (rc != MPI_SUCCESS)
            printf("%d: Receive signature_number failure\n", taskid);
        rc = MPI_Recv(&start_point, 500, MPI_LONG, 0, 502, MPI_COMM_WORLD, &status);
        if (rc != MPI_SUCCESS)
            printf("%d: Receive start_point failure\n", taskid);
    }
    else
    {
        // master receiving indexes
        printf("Calculating indexes and allocating memory for each node...\n");
        for (int col = 0; col < 500; col++)
        {
            rc = MPI_Recv(&individual_index, 1, MPI_LONG, MPI_ANY_SOURCE, col, MPI_COMM_WORLD, &status);
            if (rc != MPI_SUCCESS)
                printf("%d: Receive index failure\n", taskid);
            signature_number[col] = individual_index;
        }
        for (int i = 0; i < 500; i++)
        {
            long result = 0;
            for (int j = 0; j < i; j++)
            {
                result += signature_number[j];
            }
            start_point[i] = result;
        }
        //sending signature_number and start_point array to all other nodes
        for (int node = 1; node < numtasks; node++)
        {
            rc = MPI_Send(&signature_number, 500, MPI_LONG, node,
                          501, MPI_COMM_WORLD);
            if (rc != MPI_SUCCESS)
                printf("%d: Send signature_number failure\n", taskid);
            rc = MPI_Send(&start_point, 500, MPI_LONG, node,
                          502, MPI_COMM_WORLD);
            if (rc != MPI_SUCCESS)
                printf("%d: Send start_point failure\n", taskid);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    //calculate signatures and sync to the master cluster
    if (taskid == MASTER)
    {
        printf("...done!\n\n");
        printf("Master is receiving signatures from other nodes...\n");
        // receiving signature arrays from other nodes
        for (int task = 1; task < numtasks; task++)
        {
            int end_col, start_col;
            long length_of_task = 0;
            start_col = (task - 1) * col_each_task;
            if (task * col_each_task < 500)
                end_col = task * col_each_task;
            else
                end_col = 500;
            long start_index_of_task = start_point[start_col];
            for (int col = start_col; col < end_col; col++)
            {
                length_of_task += signature_number[col];
            }
            total_block_number += length_of_task;
            rc = MPI_Recv(&signatures[start_index_of_task], length_of_task, MPI_LONG, MPI_ANY_SOURCE, (task + 1) * 1000 + 0, MPI_COMM_WORLD, &status);
            if (rc != MPI_SUCCESS)
                printf("%d: Receive signatures failure\n", taskid);
            rc = MPI_Recv(&signatures_one[start_index_of_task], length_of_task, MPI_INT, MPI_ANY_SOURCE, (task + 1) * 1000 + 1, MPI_COMM_WORLD, &status);
            if (rc != MPI_SUCCESS)
                printf("%d: Receive signatures_one failure\n", taskid);
            rc = MPI_Recv(&signatures_two[start_index_of_task], length_of_task, MPI_INT, MPI_ANY_SOURCE, (task + 1) * 1000 + 2, MPI_COMM_WORLD, &status);
            if (rc != MPI_SUCCESS)
                printf("%d: Receive signatures_two failure\n", taskid);
            rc = MPI_Recv(&signatures_three[start_index_of_task], length_of_task, MPI_INT, MPI_ANY_SOURCE, (task + 1) * 1000 + 3, MPI_COMM_WORLD, &status);
            if (rc != MPI_SUCCESS)
                printf("%d: Receive signatures_three failure\n", taskid);
            rc = MPI_Recv(&signatures_four[start_index_of_task], length_of_task, MPI_INT, MPI_ANY_SOURCE, (task + 1) * 1000 + 4, MPI_COMM_WORLD, &status);
            if (rc != MPI_SUCCESS)
                printf("%d: Receive signatures_four failure\n", taskid);
            rc = MPI_Recv(&correspond_col[start_index_of_task], length_of_task, MPI_INT, MPI_ANY_SOURCE, (task + 1) * 1000 + 5, MPI_COMM_WORLD, &status);
            if (rc != MPI_SUCCESS)
                printf("%d: Receive correspond_col failure\n", taskid);
        }
        //sorting signatures
        printf("Total signature number is %ld. \n\nStart to sorting and compare...\n", total_block_number);
        quicksort(signatures, 0, total_block_number);
        printf("...done!\n\n");
        printf("Start to calculate collisions...\n");
        //compare sorted signatures, if they are equal then collisions are detected.
        long i = 0;
        while (i < total_block_number)
        {
            // if the adjacent signatures are the same and their corresponding columns are different, a collision is found
            if (signatures[i] == signatures[i + 1] && correspond_col[i] != correspond_col[i + 1])
            {
                int collision_cols[20] = {0};      //a temporary array for storing the collision columns
                collision_cols[0] = signatures[i]; //initialize collision columns array with the first element
                int collision_cols_index = 1;      //the number of collision columns
                int last_same_index = i + 1;       //the last index that has the same signature of index i.
                printf("Signature %ld -- block: M%d ,M%d, M%d, M%d -- collisions in columns: %d ", signatures[i], signatures_one[i], signatures_two[i], signatures_three[i], signatures_four[i], correspond_col[i]);
                while (signatures[i] == signatures[last_same_index])
                //calculate the last_same_index and print out the columns that have collisions but not stored yet
                {
                    if (correspond_col[i] != correspond_col[last_same_index] && isInArray(collision_cols, correspond_col[last_same_index]) == 0)
                    {
                        collision_cols[collision_cols_index] = correspond_col[last_same_index];
                        collision_cols_index += 1;
                        printf("%d ", correspond_col[last_same_index]);
                    }
                    last_same_index += 1;
                }
                collision_number += 1;
                printf("\n");
                i = last_same_index;
            }
            else
            {
                i += 1;
            }
        }
        //collision detection finishes.
        printf("%ld collisions are detected.\n", collision_number);
        printf("Program ends successfully.\n");
    }
    else
    {
        int end_col, start_col;
        long length_of_task = 0;
        start_col = (taskid - 1) * col_each_task;
        if (taskid * col_each_task < 500)
            end_col = taskid * col_each_task;
        else
            end_col = 500;
        long start_index_of_task = start_point[start_col];
#pragma omp parallel
        {
#pragma omp for
            for (int col = start_col; col < end_col; col++)
            {
                length_of_task += signature_number[col];
                //sending the arrays to the master node
                calcSignatures(col);
            }
        }
        rc = MPI_Send(&signatures[start_index_of_task], length_of_task, MPI_LONG, 0, (taskid + 1) * 1000 + 0, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
            printf("%d: Send signatures failure\n", taskid);
        rc = MPI_Send(&signatures_one[start_index_of_task], length_of_task, MPI_INT, 0, (taskid + 1) * 1000 + 1, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
            printf("%d: Send signatures_one failure\n", taskid);
        rc = MPI_Send(&signatures_two[start_index_of_task], length_of_task, MPI_INT, 0, (taskid + 1) * 1000 + 2, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
            printf("%d: Send signatures_two failure\n", taskid);
        rc = MPI_Send(&signatures_three[start_index_of_task], length_of_task, MPI_INT, 0, (taskid + 1) * 1000 + 3, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
            printf("%d: Send signatures_three failure\n", taskid);
        rc = MPI_Send(&signatures_four[start_index_of_task], length_of_task, MPI_INT, 0, (taskid + 1) * 1000 + 4, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
            printf("%d: Send signatures_four failure\n", taskid);
        rc = MPI_Send(&correspond_col[start_index_of_task], length_of_task, MPI_INT, 0, (taskid + 1) * 1000 + 5, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
            printf("%d: Send correspond_col failure\n", taskid);
    }
    MPI_Finalize();
    return 0;
}

void calcSignatures(int col)
{
    //generate neighbours for each row in each col
    int col_neighbours[4400][200];
    int exist_neighbours = 0;
    for (int row = 0; row < 4400; row++)
    {
        col_neighbours[row][0] = -2;
        int index = 0;
        for (int x = row + 1; x < 4400; x++)
        {
            if (isNeighbour(row, x, col) == 1)
            {
                col_neighbours[row][index] = x;
                index += 1;
            }
        }
        if (index < 3)
        {
            col_neighbours[row][0] = -2;
        }
        else
        {
            col_neighbours[row][index] = -2;
            col_neighbours[row][index + 1] = -2;
            col_neighbours[row][index + 2] = -2;
            exist_neighbours = 1;
        }
    }
    int index = 0;
    if (exist_neighbours == 1)
    {
        for (int row = 0; row < 4400; row++)
        {
            if (col_neighbours[row][0] >= 0)
            {
                for (int x = 0; col_neighbours[row][x] > 0; x++)
                {
                    for (int y = x + 1; col_neighbours[row][y] > 0; y++)
                    {
                        for (int z = y + 1; col_neighbours[row][z] > 0; z++)
                        {
                            long one_signature = getOneSignature(row, col_neighbours[row][x], col_neighbours[row][y], col_neighbours[row][z]);
                            setSignature(col, -1, one_signature, row, col_neighbours[row][x], col_neighbours[row][y], col_neighbours[row][z]);
                            index += 1;
                        }
                    }
                }
            }
        }
        // printf("Col %d found signatures %d\n", col, index);
    }
}

int isInArray(int array[], int value)
{
    for (int i = 0; i < 20; i++)
    {
        if (array == 0)
            return 0;
        if (array[i] == value)
            return 1;
    }
    return 0;
}

long allocateMemory(int col)
{
    //generate neighbours for each row in each col
    int col_neighbours[4400][200];
    int exist_neighbours = 0;
    for (int row = 0; row < 4400; row++)
    {
        col_neighbours[row][0] = -2;
        int index = 0;
        for (int x = row + 1; x < 4400; x++)
        {
            if (isNeighbour(row, x, col) == 1)
            {
                col_neighbours[row][index] = x;
                index += 1;
            }
        }
        if (index < 3)
        {
            col_neighbours[row][0] = -2;
        }
        else
        {
            col_neighbours[row][index] = -2;
            col_neighbours[row][index + 1] = -2;
            col_neighbours[row][index + 2] = -2;
            exist_neighbours = 1;
        }
    }
    long index = 0;
    if (exist_neighbours == 1)
    {
        total_col_has_blocks += 1;
        for (int row = 0; row < 4400; row++)
        {
            if (col_neighbours[row][0] >= 0)
            {
                for (int x = 0; col_neighbours[row][x] > 0; x++)
                {
                    for (int y = x + 1; col_neighbours[row][y] > 0; y++)
                    {
                        for (int z = y + 1; col_neighbours[row][z] > 0; z++)
                        {
                            index += 1;
                        }
                    }
                }
            }
        }
    }
    return index;
}

void quicksort(long x[], long first, long last)
{
    // quick sorting the whole signature array with the four elements row index array and the correspond column value
    long pivot, j, temp, i, tempone, temptwo, tempthree, tempfour, tempcol;
    if (first < last)
    {
        pivot = first;
        i = first;
        j = last;
        while (i < j)
        {
            while (x[i] <= x[pivot] && i < last)
                i++;
            while (x[j] > x[pivot])
                j--;
            if (i < j)
            {
                temp = x[i];
                tempone = signatures_one[i];
                temptwo = signatures_two[i];
                tempthree = signatures_three[i];
                tempfour = signatures_four[i];
                tempcol = correspond_col[i];
                x[i] = x[j];
                signatures_one[i] = signatures_one[j];
                signatures_two[i] = signatures_two[j];
                signatures_three[i] = signatures_three[j];
                signatures_four[i] = signatures_four[j];
                correspond_col[i] = correspond_col[j];
                x[j] = temp;
                signatures_one[j] = tempone;
                correspond_col[i];
                signatures_two[j] = temptwo;
                signatures_three[j] = tempthree;
                signatures_four[j] = tempfour;
                correspond_col[j] = tempcol;
            }
        }
        temp = x[pivot];
        tempone = signatures_one[pivot];
        temptwo = signatures_two[pivot];
        tempthree = signatures_three[pivot];
        tempfour = signatures_four[pivot];
        tempcol = correspond_col[pivot];
        x[pivot] = x[j];
        signatures_one[pivot] = signatures_one[j];
        signatures_two[pivot] = signatures_two[j];
        signatures_three[pivot] = signatures_three[j];
        signatures_four[pivot] = signatures_four[j];
        correspond_col[pivot] = correspond_col[j];
        x[j] = temp;
        signatures_one[j] = tempone;
        signatures_two[j] = temptwo;
        signatures_three[j] = tempthree;
        signatures_four[j] = tempfour;
        correspond_col[j] = tempcol;
        quicksort(x, first, j - 1);
        quicksort(x, j + 1, last);
    }
}

long getOneSignature(int row1, int row2, int row3, int row4)
{
    return keys[row1] + keys[row2] + keys[row3] + keys[row4];
}

void setSignature(int col, int index, long value, int one, int two, int three, int four)
{
    //set the at column - col, and the corresponding row infomation of the blocks four element
    long start = getStartPoint(col);
    if (index == -1) //means add a new value
    {
        signatures[start + filled_signature[col]] = value;
        signatures_one[start + filled_signature[col]] = one;
        signatures_two[start + filled_signature[col]] = two;
        signatures_three[start + filled_signature[col]] = three;
        signatures_four[start + filled_signature[col]] = four;
        correspond_col[start + filled_signature[col]] = col;
        filled_signature[col] = filled_signature[col] + 1;
    }
    else
    { //change an exist value
        signatures[start + index] = value;
        signatures_one[start + index] = one;
        signatures_two[start + index] = two;
        signatures_three[start + index] = three;
        signatures_four[start + index] = four;
        correspond_col[start + index] = col;
    }
    // printf("For col %d get start point %ld,setting to %ld,filled signatures is %ld\n", col, start, value, filled_signature[col]);
}

long getStartPoint(int col)
{
    //get the start index of a col
    return start_point[col];
}

int isNeighbour(int i, int j, int col)
{
    // check if two elements are within dia/2
    if (data[col][i] >= data[col][j])
    {
        if ((data[col][i] - data[col][j]) < dia / 2)
            return 1;
    }
    else
    {
        if ((data[col][j] - data[col][i]) < dia / 2)
            return 1;
    }
    return 0;
}

void readData()
{
    //read data from data.txt
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_index = 0;
    int column_index;

    fp = fopen("./data.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *pch;
        column_index = 0;
        pch = strtok(line, " ,");
        while (pch != NULL)
        {
            sscanf(pch, "%lf", &data[column_index][line_index]);
            column_index += 1;
            pch = strtok(NULL, " ,");
        }
        line_index += 1;
    }

    fclose(fp);
    if (line)
        free(line);
}

void readKeys()
{
    //read keys from keys.txt
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int index = 0;

    fp = fopen("./keys.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *pch;
        pch = strtok(line, " ,");
        while (pch != NULL)
        {
            sscanf(pch, "%ld", &keys[index]);
            index += 1;
            pch = strtok(NULL, " ,");
        }
    }
    fclose(fp);
    if (line)
        free(line);
}