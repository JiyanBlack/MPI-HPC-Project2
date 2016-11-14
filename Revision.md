
### Lecture 1
1. Concurrency: Concurrency is the simultaneous execution of multiple tasks. It can be logical or physical.
Logical: multiple tasks run on the same processor. Phyiscal: run on different processor.
2. Parallelism: breaking one task into several small tasks and combine the results into one.

### Lecture 2
1. A process is a program in execution.
2. The process is executed in a circle：execute the head of a que, suspend, append to the end of the que.
3. Threads are independent executed, but with a shared memory space.
4. How fast a process will run depends on how much the useful data is been cached.
5. Cache hit: data found in cache. Cache miss: data needs to be retrieved from the ram.
6. Principle of referential locality: In computer science, locality of reference, 
also known as the principle of locality, is a term for the phenomenon in which the same values,
or related storage locations, are frequently accessed, depending on the memory access pattern. 
There are two basic types of reference locality – temporal and spatial locality. 
Temporal locality refers to the reuse of specific data, and/or resources, 
within a relatively small time duration. 
Spatial locality refers to the use of data elements within relatively close storage locations. 

### Lecture 3
1. Compile OpenMP: gcc -fopenmp code.c
2. Hyperthreading: A physical core is treated as two logical cores to maximum the efficiency. 
If a thread is in idle state, the core switch to execute an other thread.
This may reduce the multiple-threads architecture programs.
3. Barrier: there is always a barrier at the end of `#pragma omp parallel {}`
4. Improvement: Reduce communications, balance load, reduce cache miss
5. #pragma omp parallel for (nowait): a default barrier at the end of for loop
6. #pragma omp parallel schedule(static/dynamic/guided)
7. #pragma omp critical: only one thread can enter the critical region
8. #pragma omp atomic: same as critical but only applies to the memory location update
9. SPMD: single program, multiple data
10. #pragma omp barrier: wait all threads at the barrier point
11. #pragma omp master: only executes by the master thread
12. #pragma omp single: code only executes by the single block
13. Data sharing: shared/ private/ firstprivate. 
14. #pragma omp for private(variable): the value is uninitialized
15. firstprivate(variable): with the corresponding original value
16. lastprivate(variable): defined as the value of its last sequential iteration

### MPI
