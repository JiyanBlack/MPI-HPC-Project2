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
void allocateMemory(int col);
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
static int *signatures_one = new int[length_of_array];      //record the first element of each block
static int *signatures_two = new int[length_of_array];      //record the second element of each block
static int *signatures_three = new int[length_of_array];    //record the third element of each block
static int *signatures_four = new int[length_of_array];     //record the fourth element of each block
static int *correspond_col = new int[length_of_array];      //record the column number of each block
static long *signatures = new long[length_of_array];        //signatures of all rows in a one-dimensinal array
static long *sorted_signatures = new long[length_of_array]; //the sorted signatures
//result and log file
static FILE *result_txt;
static FILE *log_txt;

int main(void)
{
    //control the main work flow
    result_txt = fopen("./result.txt", "w+");
    log_txt = fopen("./log.txt", "w+");
    //read data:
    readData();
    readKeys();
    //omp section start
    omp_set_num_threads(core_number); //set thread number
//start parallel computing to calculate signatures for each column
#pragma omp parallel
    {
#pragma omp for
        for (int col = 499; col >= 0; col--)
        {
            //allocate memory for the blocks of each column
            allocateMemory(col);
        }
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
    // MPI_Barrier(MPI_COMM_WORLD);
    //MPI begin, run by all tasks
    int numtasks, taskid, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    printf("MPI_MAX_PROCESSOR_NAME = %d", MPI_MAX_PROCESSOR_NAME);
    int partner, message;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Get_processor_name(hostname, &len);
    printf("Running from task %d on %s!\n", taskid, hostname);
    if (taskid == 0)
    {
        for (int col = 0; col < 500; col++)
        {
        }
    }
    else
    {
        int col_each_task = (int)(500 / numtasks) + 1;
        for (int col = (taskid - 1) * col_each_task; col < taskid * col_each_task && col < 500; i++)
        {
            long signature_num_this_col = signature_number[col];
            long *signatures = new long[signature_num_this_col];     //signatures of all rows in a one-dimensinal array
            int *signatures_one = new int[signature_num_this_col];   //record the first element of each block
            int *signatures_two = new int[signature_num_this_col];   //record the second element of each block
            int *signatures_three = new int[signature_num_this_col]; //record the third element of each block
            int *signatures_four = new int[signature_num_this_col];  //record the fourth element of each block
            calcSignatures(col, signatures, signatures_one, signatures_two, , signatures_three, signatures_four);
        }
    }
    MPI_finalize();
    //omp section ends, all signatures are calculated into an array
    printf("%d columns have blocks, total block number is %ld\n", total_col_has_blocks, total_block_number);
    fprintf(log_txt, "%d columns have blocks, total block number is %ld\n", total_col_has_blocks, total_block_number);
    //sorting all signatures
    printf("\nQuick sorting all signatures...\n");
    fprintf(log_txt, "\nQuick sorting all signatures......\n");
    //merge sort begin
    printf("Start to sort sections of signatures...\n");
    fprintf(log_txt, "Start to sort sections of signatures...\n");
    int interval = (int)(total_block_number / core_number) + 1;
#pragma omp parallel
    {
#pragma omp for
        //parallel quick sorting all signatures
        for (int i = 0; i < core_number; i++)
        {
            quicksort(signatures, omp_get_thread_num() * interval, (omp_get_thread_num() + 1) * interval - 1);
        }
    }
}
printf("All sections are sorted, start to merge...\n");
fprintf(log_txt, "All sections are sorted, start to merge...\n");
//merging sorted child arrays of signatures
int indexes[1000] = {0};
int i = 0;
while (i < total_block_number)
{
    long min_value = signatures[indexes[0]];
    int cur_min_index = 0;
    for (int core = 1; core < core_number; core++)
    {
        if (min_value > signatures[indexes[core]])
        {
            min_value = signatures[indexes[core]];
            cur_min_index = core;
        }
    }
    if (min_value != 0)
    {
        i++;
        sorted_signatures[i] = min_value;
    }
    indexes[cur_min_index] += 1;
}
printf("Quick sorting finished! \n\nStart collision detecting...\n");
fprintf(log_txt, "Quick sorting finished! \n\nStart collision detecting...\n");
//compare sorted signatures, if they are equal then collisions are detected.
i = 0;
while (i < total_block_number)
{
    // if the adjacent signatures are the same and their corresponding columns are different, a collision is found
    if (sorted_signatures[i] == sorted_signatures[i + 1] && correspond_col[i] != correspond_col[i + 1])
    {
        int collision_cols[20] = {0};             //a temporary array for storing the collision columns
        collision_cols[0] = sorted_signatures[i]; //initialize collision columns array with the first element
        int collision_cols_index = 1;             //the number of collision columns
        int last_same_index = i + 1;              //the last index that has the same signature of index i.
        fprintf(result_txt, "Signature %ld -- block: M%d ,M%d, M%d, M%d -- collisions in columns: %d ", sorted_signatures[i], signatures_one[i], signatures_two[i], signatures_three[i], signatures_four[i], correspond_col[i]);
        while (sorted_signatures[i] == sorted_signatures[last_same_index])
        //calculate the last_same_index and print out the columns that have collisions but not stored yet
        {
            if (correspond_col[i] != correspond_col[last_same_index] && isInArray(collision_cols, correspond_col[last_same_index]) == 0)
            {
                collision_cols[collision_cols_index] = correspond_col[last_same_index];
                collision_cols_index += 1;
                fprintf(result_txt, "%d ", correspond_col[last_same_index]);
            }
            last_same_index += 1;
        }
        collision_number += 1;
        fprintf(result_txt, "\n");
        i = last_same_index;
    }
    else
    {
        i += 1;
    }
}
//collision detection finishes.
printf("%ld collisions are detected.\n", collision_number);
fprintf(log_txt, "%ld collisions are detected.\n", collision_number);
printf("\nLogs are recorded in the log.txt file.\n");
printf("Collisions are recorded in the result.txt file.\n");
fclose(result_txt);
fclose(log_txt);
}

void calcSignatures(int col, long *signatures, int *signatures_one, int *signatures_two, int *signatures_three, int *signatures_four)
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
            fprintf(log_txt, "Col %d row %d find neighbour number %d\n", col, row, index);
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
                            signatures[index] = getOneSignature(row, col_neighbours[row][x], col_neighbours[row][y], col_neighbours[row][z]);
                            signatures_one[index] = one;
                            signatures_two[index] = two;
                            signatures_three[index] = three;
                            signatures_four[index] = four;
                            index += 1;
                        }
                    }
                }
            }
        }
        printf("Col %d has signatures %d\n", col, index);
        fprintf(log_txt, "Col %d has signatures %d\n", col, index);
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

void allocateMemory(int col)
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
                            total_block_number += 1;
                        }
                    }
                }
            }
        }
        printf("Allocate memory for column %d\n", col);
    }
    signature_number[col] = index;
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

void printCol(int col)
{
    //print a single column in data
    printf("Data of column %d\n", col);
    for (int i = 0; i < 4400; i++)
    {
        printf("Row %d, value %f\n", i, data[col][i]);
    }
}
