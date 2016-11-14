int main(array)
{
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
main([ 1, 800, 153, 553, 34 148, 754, 839, 276, 962 679, 993, 896, 897, 172 846, 246, 741, 749, 898 745, 119, 486, 406, 8 407, 571, 551, 920, 940 214, 266, 691, 740, 885 310, 585, 781, 585, 658 105, 78, 884, 390, 274 599, 943, 583, 837, 753 422, 808, 651, 266, 918 659, 23, 73, 211, 751 638, 223, 462, 921, 758 46, 945, 130, 35, 814 435, 388, 457, 563, 310 851, 966, 559, 929, 599 984, 483, 708, 733, 69 77, 326, 945, 534, 198 825, 198, 570, 886, 672 774, 204, 262, 776, 884 ]);