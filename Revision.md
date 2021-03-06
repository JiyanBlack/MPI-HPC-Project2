### Last Lecture Video

1. 5 questions, each 10 points;
2. Three types of questions: conceptual questions, codes correction/completion/intepretation/output, 
write code to solve a question.
3. Data parallelism: SPMD, data(variables) are local to different threads. Each thread wil change 
and have its own data.

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


### OpenMP

1. Compile OpenMP: `gcc -fopenmp code.c` ; `omp_set_num_threads(4);`
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

1. The message-passing model: MPI is for communicating among processes, which have separate address spaces.
2. Data parallel: SIMD, same instruction multiple data.
3. Task parallel: MIMD, multiple instruction multiple data.
4. SPMD: single program multiple data. It is quivalent to MIMD, since multiple instructions can be stored in single program.
5. Message passing and MPI is for MIMD/SPMD parallelism.
6. Program structure:
`
int rank, size;
MPI_Init(null, null);
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Comm_size( MPI_COMM_WORLD, &size );
printf( "I am %d of %d\n", rank, size );
MPI_Finalize();
return 0;
`
7. Processes can be collected into groups. Each message is sent in a context and must be received in
the same context. A group and context together form a communicator. A process is identified by its rank
in the group associated with a communicator. MPI_COMM_WORLD is a default group that has all processes.
8. TAG is used to identify the message from sender to receivers. (MPI_ANY_TAG)
9. MPI_SEND (start_memory_address, memory_length, dataType, dest_rank in the communicator, tag, communicator)
example:
`
int rc;
rc=MPI_Send(&array,array_length,MPI_INT,1,MPI_ANY_TAG,MPI_COMM_WORLD);
if (rc != MPI_SUCCESS){...}
`
10. MPI_RECV(start, count, datatype, source, tag, comm, status)
`MPI_Status status;
rc=MPI_Recv(&local_array,array_length,MPI_INT, MPI_ANY_SOURCE, MPI_COMM_WORLD, status);
`
11. Basic API
  * MPI_Init(null,null);
  * MPI_Finalize();
  * MPI_Comm_size(MPI_COMM_WORLD,worldsize);
  * MPI_Comm_rank(MPI_COMM_WORLD, rank);
12. MPI_Bcast -- broadcast data from one process to others
13. MPI_Reduce -- combines data from other processes into one process
14. In concurrent computing, a deadlock is a state in which each member of a group of actions, is waiting for some other member to release a lock. As a result, every process is waiting for others and no progress is made.
15. Compile: mpicc -fopenmp code.c
16.When to use MPI: 
  * Portability and Performance
  * Irregular Data Structures
  * Building tools for others(libraries,crosslanguage)
  * Need to manage memory per processor
17. When not to use MPI:
  * Regular computation matches HPF
  * Solution already exists
  * Require fault tolerance
  * Distributed Computing

### Conceptual Understanding
##### Communicators and groups
 * A communicator can be a group of ordered processes, each process with a rank number.
 * Groups allow collective operations to work on a subset of processes.
 * Information can be added into communicators to be used by the processes in the communicators.
 * Communicator access operations are local, thus requiring no interprocess communication
Communicator constructors are collective and may require interprocess communication
All the routines in this section are for intracommunicators, intercommunicators will be covered separately
 * Intracommunicator: communicator within a group. Intercommunicator: communicator between a group.
 * Intracommunicator create: MPI_COMM_CREATE. This is a collective routine, meaning it must be called by all processes in the group associated with commThis routine creates a new communicator which is associated with groupMPI_COMM_NULL is returned to processes not in groupAll group arguments must be the same on all calling processesgroup must be a subset of the group associated with comm.
 * Destructors: Groups/communicator number is limited. MPI_Group/Comm_Free()..
 * Intercommunicators are associated with 2 groups of disjoint processes
Intercommunicators are associated with a remote group and a local group
The target process (destination for send, source for receive) is its rank in the remote group
A communicator is either intra or inter, never both.

##### Extended Collective Communication
 * The original MPI standard did not allow for collective communication across intercommunicators
MPI-2 introduced this capability
Useful in pipelined algorithms where data needs to be moved from one group of processes to another.
 * Rooted:
One group (root group) contains the root process while the other group (leaf group) has no root
Data moves from the root to all the processes in the leaf group (one-to-all) or vice-versa (all-to-one)
The root process uses MPI_ROOT for its root argument while all other processes in the root group pass MPI_PROC_NULL
All processes in the leaf group pass the rank of the root relative to the root group.
