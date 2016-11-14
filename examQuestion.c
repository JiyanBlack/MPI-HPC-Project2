#include <stdio.h>
#include <omp.h>

int main()
{
	//samle array
	static int array[100];
	array[0]=10;
	array[1]=170;
	array[2]=150;
	array[3]=-6;
	array[4]=5;
    //array stores the local max/min values
    int local_max[4];
    int local_min[4];
    //initialize global max/min
    int global_max = array[0];
    int global_min = array[0];
    int initialized = 0;
    //set four threads
    omp_set_num_threads(4);
#pragma omp parallel
    {
#pragma omp for private(initialized)
        for (int i = 0; i < 100; i++)
        {
            int thread = omp_get_thread_num();
            if (initialized == 0)
            //set the initial value as the first element
            {
                local_max[thread] = array[i];
                local_min[thread] = array[i];
                initialized = 1;
            }
            else
            //compare local max/min
            {
                if (array[i] > local_max[thread])
                {
                    local_max[thread] = array[i];
                }
                if (array[i] < local_min[thread])
                {
                    local_min[thread] = array[i];
                }
            }
        }
    }
    //compare the local max/min
    for (int i = 0; i < 4; i++)
    {
        if (global_max < local_max[i])
        {
            global_max = local_max[i];
        }
        if (global_min > local_min[i])
        {
            global_min = local_min[i];
        }
    }
    //print the global max/min
    printf("The maximum value: %d\n", global_max);
    printf("The minimum value: %d\n", global_min);
    return 0;
}


