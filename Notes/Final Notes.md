# Week 1

## ABI, API, Load Module

- ABI specifies the load module format
- API specifies how a library can be used
- ABI is the interface between the OS and the apps
- API is the interface between different programs
- Trap Handler (Trap table initialized in boot time)
  - To execute a system call, a program must execute a special trap instruction
  - When it traps into OS, it saves registers and raises the privilege level to kernel mode, then check the trap table and jump to trap handler
  - Handle trap
  - return from trap instruction
  - restore registers
  - go back to main routine



## Why must the ISA instruction that controls where to find the trap tables in a system be privileged?

Being able to execute the instruction to tell the hardware where the trap tables are is a very powerful capability. If this instruction is not privileged, then malicious program could intall its own trap table and screw the system.

# Week 2

## Process

- Process data and process state
- How to create a process
  - What exactly do fork and exec do?
  - copy-on-write
  - how to implement a system call? (Need to mention trap handler)



## What is the purpose of a final state (also known as a zombie state) for a process?

A final state indicates that a process has finished executing all of its code. However, it has not yet been cleaned up. This allows the parent process to check its exit status and possibly perform other cleanup tasks. 



## Process Scheduling

- scheduling goals
  - maximize throughput
  - minimize wait time
  - ensure fairness
  - other priority goals
- Pros and Cons of preemptive scheduling
- Different scheduling algorithms
  - FIFO
  - RR
  - Priority Scheduling
- Metrics
  - Turnaround time: (Time completion) - (Time arrival)
  - Response time: (Time first run) - (Time arrival)



## If we use a scheduler algorithm that optimizes fairness, what other desirable property is likely to be damaged? Why?

A scheduling algorithm that optimizes fairness, like Round Robin, is likely to hurt turnaround time, the time from a process' arrival to its completion. This occurs because fair scheduling algorithms tend to give some processor to each waiting process, rather than focusing on finishing any one process.



## How does the cooperative approach to switching between running processes and running OS code work? Why is this approach not used in most operating systems?

The cooperative approach is the approach where the OS does not have the ability to terminate processes and relies on each side (OS process and user process) voluntarily yielding control to the other to allow it to run. This is not used because it is highly undersirable to allow the system to run on voluntarily yielding. The problem is that yielding is under programmer's control and they might not yield properly. 

In the cooperative approach, processes are expected to voluntarily yield every so often, which will allow the operating system to regain control without requiring a mechanism (such as a timer interrupt) to involuntarily halt the processes. This approach requires both careful thought and a certain amount of public-spiritedness on the part of all application programmers, since if they don’t figure out when they should yield, or choose not to yield for selfish reasons, the OS won’t get to run. Since we cannot count on application programmers to always do the right thing, we run the risk of highly unfair behavior. Worse, an infinite loop in a program might only be solvable by rebooting.*



## Figure 1 below shows a typical throughput vs. offered load curve for a system resource. Figure 2 below shows a typical response time vs. offered load curve for a system resource. As load increases, the response time goes to infinity, while the throughput drops less dramatically. Why does response time show a more dramatic trend than throughput?



**Figure 1**

![Screen Shot 2019-06-09 at 6.37.35 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-06-09 at 6.37.35 PM.png)



**Figure 2**

![Screen Shot 2019-06-09 at 6.38.13 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-06-09 at 6.38.13 PM.png)



Throughput drops because scheduling is not free. It takes time to dispatch a process (overhead). More dispatches mean more overhead, and thus less time is available to run processes. Response time grows to infinity because real systems have limit in queue size. When limits are exceeded, requests are typically dropped, which is an infinite response time for these requests.



## What metric might you use to determine the effectiveness of a soft real time scheduler? Why would that metric be useful for this purpose? Give an example of an application where this metric would be helpful in determining if the soft real time system was meeting its goals.

We would use timeliness as the performance metric for real time scheduler. Timeliness measures how closely does the scheduler meets the deadlines. Another metric would be predictability, which measures how much deviation there is in delivered timeliness. These metrics are useful in real time scheduler because here we care about whether or not the deadline is met or not rather than response time, turnaround time, or fairness. An example of soft real time scheduler is a video playing device. If the timeliness is good, meaning jobs are completed before deadlines, then we would see the screen is updated every frame on time. Otherwise, we would experience lags in the game.

# Week 3

## Memory Management

- Segmentation and Paging
- How does paging work exactly
  - What if I change the page table structure
- Demand paging and virtual memory
- page replacement algorithms
- Working sets and thrashing



## Elements in a memory free list could be ordered by size or could be ordered by their address. What is an advantage of ordering them by size? What is an advantage of ordering them by address?

**One advantage of ordering them by size** is it would be easier to find an element of a particular desired size, as we could use an algorithm like binary search. It would be very easy to find the largest block for a worst fit algorithm, or the smallest free block that is big enough to serve the allocation for best fit algorithm. **One advantage of ordering by address** is it would be easier to perform a coalescing algorithm to combine nearby free blocks. Coalescing free blocks reduces the chance where there is no single chunk of free memory block that satisfy an allocation request although the total amount of free blocks is sufficient. 



## A looping sequential page workload runs sequentially through a set of pages of some fixed size, cycling back to the first page once it is finished with the last page. Why might an LRU page replacement algorithm handle this workload poorly? What kind of practical page replacement algorithm would handle it better?

In such a workload, the least recently used page is actually the page that is going to be accessed next, since the workload goes in order circularly. This workload exhibits no temporal locality, which is what the LRU algorihtm relies on. A **most recently used** page replacement algorithm would handle this workload better since it retains pages that were used long time ago.



## When the operating system issues addresses for RAM locations for its own use (such as accessing a process control block or finding a particular buffer in the block cache), is it issuing virtual addresses or physical addresses? Why?

The OS uses the same CPU as user processes do, and its  instructions are passed through the same MMU as the instructions of those user processes. It does so because OS then would not need to worry about where the data actually is in physical memory.



## What form of fragmentation does hard disk defragmentation help with? Why does it help with this form and not the other form?

Disk defragmentation helps with external fragmentation. It helps with external fragmentation and not internal fragmentation because disk defragmentation is a reordering of drives occupied space. The reordering can combine free space and eliminate small fragments. This however has no impact on the amount of unused space in allocated segments.



## How can a user level thread package achieve preemptive scheduling?

We can ask the OS for a timer interrupt. Before dispatching a thread, we can schedule this timer that will interrut the thread if it runs too long. If a thread runs too long, the interrupt handler can yield on behalf of that thread, saving its state, moving on to the next thread in the ready queue.



## Describe an advantage of a first fit memory allocation strategy and a disadvantage of such a strategy, including in each case why the strategy has that characteristic.

First fit memory allocation is very efficient because it doesn't require a full traversal of the memory free list every time a new request is made to memory. It works practically well when memory has a lot of free blocks. Other algorithm such as best fit would need to traverse the entire list to find the best fit. However, the disadvantage of this allocation strategy is the amount of fragmentation that arises as the system grows older. There is extermal fragmentation as blocks are allocated and deallocated that resides between allocated blocks that are free but aren't fit to the requested size resulting in more and more wasted space. 

First fit tends to find suitable allocations more quickly than best or worst fit strategies, since it does not need to examine each free memory segment to determine its suitability. Once any suitable segment is found, first fit chooses it. A disadvantage is that the front of the free list tends to get fragmented quickly, since first fit will carve allocations out of the first suitable segment it finds, starting from the head of the list. In addition to obvious fragmentation problems, this characteristic will tend to cause searches to get longer as time goes on, reducing one of first fit’s advantages.



## What are two advantages of fixed memory partition memory management schemes? Why do such schemes have these advantages?

First, there is less overhead of memory management processing because each fixed size segment is either used or not used. Second, there is no external fragmentation (although there is internal fragmentation). 

# Week 4

## Threads and Critical Sections

- Differences between thread and process: v
- Race condition
  - be able to identify race conditions in a program
- Ways to enforce mutual exclusion
  - Disable interrupts
  - Atomic instructions
  - Software locking
    - atomic instructions
    - spin waiting, spin and yield
    - Asynchronous completion, sleep and wake up



## What problem is solved with sloppy counters? How do they solve this problem? What is the disadvantage of using them?

Sloppy counters solve the problem of contention and race conditions. They solve this problem by reducing mutual exclusion. Each thread is given its own counter, and it is only added to the global counter once a certain threshold is met by this small counter. This limits the amount of contention as multiple threads are unlikely to try and update the global counter at the same time while each counter can update its own counter simulteneously. The disadvantage of sloppy counters is that the global counter may be inaccuate at any single time. The global counter may not have reflected the counts from smaller counters when it is checked.



## Why is asynchronous I/O useful for systems using event-based concurrency?

Asynchronous I/O is useful for systems using event based concurrency because synchronous I/O is not multithreaded. That is, if an event flag is raised that requires blocking for I/O, the entire process waits for the event's I/O to complete. With asynchronouse I/O, the process can quickly return and resume normal execution without having to wait for the entire I/O to complete. 



## Why does code based on event-loop synchronization need to avoid blocking?

Event loops or event based concurrency systems are designe to only use one thread. In this way, there are no concurrency issues and the loop can process events as they come in. However, because there is only one thread, performing a synchronous I/O that blocks will cause this entire event loop to hang, waiting for the I/O to finish, wasting resources and missing requests that come in. Event loop therefore require asynchronous I/O such that they can perform I/O without blocking and wasting CPU cycles.

Event loop synchronization does not typically use multithreading. Instead, it waits for events to occur and calls management code to handle events as they happen. If any of the management code blocks, then, the system will not check for other events that might occur until the code unblocks. Performance will thus be poor.



## A program uses multiple threads that have a mutex protecting a critical section. What are the main factors that determine the probability that two or more threads will have a conflict on this mutex? For each factor, what is the effect of varying values (high and low) for that factor on the probability of conflict?

The main factors are time spend in critical section and number of threads. Increasing the time spent in critical section increases the probability of conflict. Similarly, increasing the number of threads also increase the probability of conflict.



# Week 5

## More on lock data structures

- Semaphores
- Mutex
- Object-level locks
  - files
- Performance of locking: contention
  - convoy
  - priority inversion
    - Multiple components share an information bus
    - A high priority bus management task needed to run frequently (for brief periods, during which it locks the bus)
    - A low priority task ran occasionally (Also for brief periods, during which it locks the bus)
    - A medium priority communication task ran rarely (But for a long time when it ran, but it doesn't use the bus, so it doesn't need the lock (i.e. the bus))
    - The problem happens when the low priority task runs and acquires the lock. Consider now the high priority  task comes. It will wait for the lock, which is still okay since low priority task is short. But imagine we have middle priority task comes. The low priority task will go to the waiting queue, holding the lock. The high priority task cannot run. The middle priority task is long. The low priority task can't interrupt the middle priority task. 
- Improve performance
  - reducing locking overhead
  - reducing contention: remove CS, reduce CS, fine-grained locks, spread the requests to multiple resources



## Consider the following use of semaphores. A parent thread creates a child thread. The parent should not run until the child thread has performed a set of initialization operations. What should the semaphore's counter be initialized to? Which semaphore operations should the parent and child thread call, and when? Why does this use of a semaphore acheive the desired goal?

The semaphore should be initialized to 0. The parent should immediatly `sem_wait()` once it creates the child. This will cause the parent to sleep until the semaphore is 1. The child should perform its initialization, then call `sem_post()` to increment the value of semaphore. We can think of two cases here. The first case is when parent call `sem_wait()` after the child has completed its initialization. Then the `sem_wait()` will immediately return, and thus acheive the desired goal (the parent runs after the child's initialization). The second case is when the parent call `sem_wait()` before the child has completed its initialization. Then the `sem_wait()` will put the parent to sleep because the semaphore is still 0. Once the child completes its initialization and call `sem_post()`, the semaphore becomes 1, and thus `sem_wait()` in the parent will return and decrement the semaphore. Therefore, acheive the desired goal (the parent runs afer the child's initialization). 



## In an operating system context, what is meant by a convoy on a resource? What causes it? What is the usual effect of such a convoy?

When multiple processes need one resource, one process gets the resource and the other processes form a convoy and are all blocked waiting for the resource. Convoy effect is phenomenon associated with the First Come First Served algorithm, in which the whole Operating system slows down due to few slow processes. The usual effect of such a convoy is that CPU and I/O become idle a lot of time and thus, slow down the performance. Suppose there is one CPU intensive process in the ready queue, and several other processes with relatively less CPU time but are I/O intensive. First, the I/O intensive processes are first allocated CPU time. As they are less CPU intensive, they quickly get executed and go to I/O queues. Now the CPU intensive process is allocated CPU time. As it is CPU intensive, it takes time to complete. While the CPU intensive process is being executed, the I/O intensive processes complete their I/O and I/O is ready to process the next request. However, since CPU intensive process still hasn't finished, this leads to I/O devices being idle. When the CPU the CPU intensive process gets over, it is sent to the I/O queue so that it can access I/O devices. Meanwhile, an I/O intensive process get their required CPU time and move back to I/O queue. However, this I/O request is made to wait because the CPU intensive process is still accessing I/O devices and thus, leads to CPU sitting idle. 

A convoy in this context refers to a set of processes (or other computing entities) that all need to make exclusive use of a shared service, device, or object. If multiple entities are queued waiting for the object, and the service time to handle one entity exceeds the interarrival time for another non-queued entity to need access to the object, it will
increase the length of the queue. In effect, a convoy kills all parallelism between participating entities, since generally all but one of them is queued waiting for the resource.





## What is the purpose of a condition variable?

A condition variable is used to determine of some specific pre-defined condition has or has not occured. If the condition does occur, one more more of the blocked processes will be unblocked and permitted to run. The condition variable allows a process to wait for a specific condition without requiring the process to use a busy loop to check for the condition's occurrence.



## Like many other industries, Santa Claus has turned to automation to produce a sufficient quantity of toys to meet demand. He has built a machine that automatically produces nutcrackers, and purchased several slower machines that, on command, can grab a nutcracker and wrap is as a present. Unfortunately, the wrapping machines go haywire and attempt to wrap up the nearest elf if they try to wrap a nutcracker when none is available, so it's important not to order them into action if no finished nutcrackers are still unwrapped. Santa plans to use a semaphore to control the process, essentially using it to notify wrapping machines when a nutcracker is available. What operation on the semaphore should the nutcracker-producing machine perform? What operation on the semaphore should the wrapping machine perform? What should the semaphore be initialized to? Briefly describe how this synchronization process will work.

This problem illustrates the producer-consumer problem. Semaphore should be initialized to 0 as there is no nutcracker at the beginning. Production machine must post on each nutcracker. Wrappers must use wait operation.





# Week 6

## Deadlock

- What is a deadlock?
  - Can you identify a deadlock from a given program?
- What are the four conditions?
  - Mutual exclusion
  - incremental allocation
  - no preemption
  - circular dependency of resources
- How to eliminate deadlocks?
- How to make synchronization easier
  - Monitor: each monitor object has a semaphore. Automatically acquired on **any** method invocation
  - Java Synchronization Methods: Each object has an associated mutex. Only acquired for specific methods.



## Consider the following proposed solution to the Dining Philogophers problem. Every of the five philosophers is assigned a number 0-4, which is known to the philosopher. The philosophers are seating at a circular table. There is one fork between each pair of philosophers, and each fork has its own semaphore, initialized to 1. `int left(p)` returns the identity of the fork to the left of philosopher `p`, while `int right(p)` returns the identity of the fork to the right of philosopher `p`. These functions are non-blocking, since they simply identify the desired fork. A philosopher calls `getforks()` to obtain both forks when he wants to eat, and calls `putforks()` to release both forks when he is finished eating, as defined below:

```C
void getforks() {
  sem_wait(forks[left(p)]);
  sem_wait(forks[right(p)]);
}

void putforks() {
  sem_post(forks[left(p)]);
  sem_post(forks[right(p)]);
}
```

## Is this a correct solution to the dining philosophers problem? Explain.

This is not a correct solution. The four conditions for deadlock are mutual exclusion, incremental allocation, no preemption, and circular dependency of resources. All four conditions exist. Semaphores provide mutual exclusion. The two `sem_wait()` calls in `getforks()` are not atomic. Philosophers continue to wait for forks; not preempted. As they are seated circularly, there is circular dependency. Consider the situation where all philosophers call `getforks()` at once, and each philosopher gets the fork to their left. Then we've achieved deadlock since all philosophers will be waiting for the right fork.



## What is the difference between synchronization using object-oriented monitors and synchronization using Java synchronized methods?

Both monitor and Java synchronized methods help developers handle locks. Each monitor object has a semaphore while each object has an associated mutex in Java synchronized methods. Java synchronized methods are more fine grained than object monitors because the latter locks object on **any** method while it only acquires the locks for specified methods in Java synchronized methods.



## Why is hold-and-wait a necessary condition for deadlock? Describe one method can be used to avoid the hold-and-wait condition to thus avoid deadlock.

Hold-and-wait is necessary for deadlock because it ensures that no contending threads will release the locks that they have already acquired, otherwise the deadlock may resolve itself. One method that can be used to avoid hold-and-wait is to force each thread to acquire all the locks before it executes (critical section). If it cannot do so, it releases everything and tries again later.



## Consider the following proposed solution to the Dining Philosophers problem. Every of the five philosophers is assigned a unique letter A-E, which is known to the philosopher. The forks are numbered 0-4. The philosophers are seating at a circular table. There is one fork between each pair of philosphers, and each fork has its own semaphore, initialized to 1. `int left(p)` returns the number of the fork to the left of philosopher `p`, while `int right(p)` returns the number of the fork to the right of philosopher `p`. These functions are non-blocking, since they simply identify the desired fork. A philosopher calls `getforks()` to obtain both forks when he wants to eat, and calls `putforks()` to release both forks when he is finished eating, as defined below:

```C
void getforks() {
  if (left(p) < right(p)) {
    sem_wait(forks[left(p)]);
    sem_wait(forks[right(p)]);
  }
  else {
    sem_wait(forks[right(p)]);
    sem_wait(forks[left(p)]);
  }
}

void putforks() {
  sem_post(forks[left(p)]);
  sem_post(forks[right(p)]);
}
```

## Is this a correct solution to the dining philosopher problem? Explain.

Yes. At least one philosopher picks up left first and one picks up right first, due to total ordering of fork acquisition. So impossible for all five philosophers to have a fork in the same hand.

Yes. In brief, the solution leverages avoidance of circular waiting to avoid deadlock. In this solution, no philosopher will ever hold a fork while waiting for a lower numbered fork. If any philosopher holds two forks, he can eat and we don’t have deadlock. The only ways possible for philosophers not to be able to obtain two forks when they want them are:

1). no philosopher can obtain any fork; not possible here since if a fork is free and a philosopher wants it, he gets it 

2) all philosophers hold one fork; not possible here since that implies that some philosopher holds fork X, but needs to obtain fork Y (Y<X) to eat, which can’t be possible since the lower numbered fork is always obtained first

In essence, we are using the resource ordering solution to avoid deadlock. To get full points, the answer must say something more than “resource ordering,” offering some explanation of why this prevents circular waiting.





## Consider the following code. Will it correctly handle parent/child thread synchronization? Why or why not?

```C
int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void thr_exit() {
  Pthread_mutex_lock(&m);
  Pthread_cond_signal(&c);
  Pthread_mutex_unlock(&m);
}

void thr_join() {
  Pthread_mutex_lock(&m);
  Pthread_cond_wait(&c, &m);
  Pthread_mutex_unclock(&m);
}

void *child (void *arg) {
  printf("child\n");
  thr_exit();
  return NULL;
}

int main(int argc, char *argv[]) {
  printf("parent: begin\n");
  Pthread_t p;
  Pthread_create(&p, NULL, child, NULL);
  thr_join();
  printf("parent: end\n");
  return 0;
}
```

No, this code will not correctly handle parent child thread synchroniztion as there is a possibility for deadlock when the child thread reaches the `thr_exit()` function. If the parent has already reached `thr_join()` then it will wait for the child to signal to conditional variable `c`. However, if the child calls `thr_exit()` before the parent calls `thr_join()` then there is a problem. When the parent calls `Pthread_cond_wait()` function, there is no thread that will wake it up, and thus the parent will sleep forever, which leads to deadlock (we can fix it by using the variable `done`. Set it to 1 immediately after `Pthread_cond_signal()`. We put the parent into sleep only if `done` is 0).



## Assume you have 2 CPU cores (C1 and C2) and 4 processes (P1, P2, P3, and P4) to run. The processes all take around the same amount of time to execute, but must acquire locks on certain resources to run. P1 requires locks on F1 and F2. P2 requires locks on F2 and F3. P3 requires a lock on F3. P4 requires locks on F1 and F2. How could you use scheduling <u>alone</u> to ensure that no processes in this set deadlock, while making maximum use of the cores? User of a technique other than scheduling to solve the problem will get no credit.

This basic idea is to run two processes on each core. The question is which two processes we should choose. To achieve maximum performance, we should place P1 and P4 to C1, and P2 and P3 to C2. Since P1 and P2 has only one lock overlap, there is still contention but it's better than running P1 and P4 in parallel (in which case only one process is actually doing something meaningful). Then since P4 and P3 have no locks in common, they can run completely in parallel.  



## The following multithreaded C code contains a synchronization bug. Where is it? What is the effect of this bug on execution?

```C
sem_t balance_lock_semaphore;
int balance = 100;

sem_init(&balance_lock_semaphore, 0, 0);

char add_balance(amount) {
  sem_wait(&balance_lock_semaphore);
  balance += amount;
  sem_post(&balance_lock_semaphore);
}

void subtract_balance(amount) {
  balance -= amount;
}

/* This code is run by thread 1 */
add_balance(deposit);

/* This code is run by thread 2 */
if (balance >= withdrawl) {
  sem_wait(&balance_lock_semaphore);
  subtract_balance(withdrawal);
  sem_post(&balance_lock_semaphore);
}
```



Initialization of semaphore is wrong. It should be 1 instead of 0. (`if(balance >= withdraw)` is also a potential issue if there is another thread that decreass the balance. )



## The following C code is intended to use semaphores to control reading and writing from a circular buffer by two differnt threads. It has a serious synchronization bug. Find the bug and describe what further synchronization operations are requried to fix it. 

```C
/* This code is run by thread 1 */
char pipe_read_char() {
  wait(&pipe_semaphore);
  c = buffer[read_ptr++];
  if (read_ptr >= BUFSIZE)
    read_ptr -= BUFSIZE;
  return c;
}

/* This code is run by thread 2 */
void pipe_write_string(char *buf, int count) {
  while (count-- > 0) {
    buffer[write_ptr++] = *buf++;
    if (write_ptr >= BUFSIZE)
      write_ptr -= BUFSIZE;
    post(&pipe_semaphore);
  }
}
```



Write can override the information if write is much faster than read.







# Week 7-8

## Device I/O and FS

- I/O
  - Device derivers
  - Communication mechanisms
  - Direct Memory Access (DMA): A method that allows an I/O device to send or receive data directly to or from the main memory, bypassing the CPU to speed up memory operations.
  - Redirect on Write
    - Once written, blocks and inodes are immutable
    - add new info to the log, and update the index
    - The old inodes and data remain in the log
    - We must eventually recycle old log entries (garbage collection)
- File System
  - File system architecture
    - Linked Extents (e.g. DOS FAT file system)
    - File index blocks (e.g. Unix System V file system)
  - File and directory
  - Control structures
    - FAT directory entries
    - Inode
      - In V file system, first 10 block pointers point to first 10 blocks of file
      - 11th points to an indirect block
      - 12th points to a double indirect block
      - 13th points to triple indirect block
  - Guarantee crash consistency



## What two mechanisms of a modern memory management system lead to the need for scatter/gather I/O? Why do they do so?

**Virtual Memory** and **Paging** lead to the need for scatter/gather I/O.  We want to support **DMA**, which requires the entire transfer to be contiguous in physical memory. However, because of virtual memory and paging, buffers in paged virtual memory may be spread all over physical memory (i.e. not contiguous). Thus, when we read from devices, we must **scatter** the data into multiple pages. Similarly, when we write to devices, we must gather data from multiple pages (and make it contiguous), then write it to device. 

![Screen Shot 2019-06-07 at 12.37.00 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-06-07 at 12.37.00 PM.png)



## For a journaling file system that only puts metadata in the journal, the data blocks must be written to the storage device before the journal is written to that device. The process requesting the write is informed of its success once the journal is written to the device. Why is this order of operation important?

**Assume the data blocks are written after the the journal is written to that device**. If the system fails after the journal is written to the device but before the data blocks are written, then there will be inconsistency in the journal because it will be pointing to invlid data blocks (the data blocks are not written yet). Thus, the data blocks must be written before the journal is written. **Assume the process is informed of its success of the write after the data blocks are written but before the journal is written to the device**. If the system fails after the data blocks are written but before the journal is written to the device, then there is no metadata that points to the location of appropriate data blocks (the journal is not written yet). Thus, the pricess must be informed of its success of the write after the journal is written.



## Does a URL more closely resemble a hard link or a soft (symbolic) link? Why?

A URL more closely resembles a symbolic link because it simply specifies the location of a file with no guarantee that it is still there. A hard link contains a pointer to the inode corresponding to a file, and it is accounted for in the inode's reference count. A symbolic link only contains the pathname of the file, and thus, there is no guarantee the file will be at the specified path until we try to open it (the filename could've changed or the file could've been removed). Also, a symblic link does not affect the reference count.



**Hard link Example**

![Screen Shot 2019-06-07 at 1.36.48 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-06-07 at 1.36.48 PM.png)



**Symbolic link example**

![Screen Shot 2019-06-07 at 1.37.29 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-06-07 at 1.37.29 PM.png)



## What is the difference between a hard link and a symbolic link in a Unix-style file system? What implications does this have for how metadata about the file referenced by the link is stored?

A hard link contains a pointer to the inode corresponding to a file, and it is accounted for in the inode's reference count. A symbolic link only contains the pathname of the file, and thus, there is no guarantee the file will be at the specified path until we try to open it (the filename could've changed or the file could've been removed). Also, a symblic link does not affect the reference count. Links are just names for the files. All other metadata is stored in the file inode.



## Why is the DOS FAT file system unable to efficiently store sparse files? Why can the Unix System V file system store such files much more efficiently?

The DOS FAT file system is unable to efficiently store sparse files because of the way the file allocation table is set up. Since sparse files are mostly empty, we would like to only store data in the file that is not empty. However, since in DOS FAT file system, each FAT entry corresponds to cluster, and contains the number of the next cluster, we still need to allocate a cluster for those empty space in the file. Otherwise, it doesn't know where to find the next cluster. On the other hand, Unix System V file system can store sparse files much more efficiently because it uses pointers to find the data blocks. Each inode contains some block pointers. If we want to allocate an empty space, we can simply set the block pointer to 0 (invalid pointer), and thus we need not to waste space for those empty space in the sparse file.

**DOS FAT File System** 

![Screen Shot 2019-06-08 at 2.31.37 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-06-08 at 2.31.37 PM.png)



**Unix System V File System**

![Screen Shot 2019-06-08 at 2.32.57 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-06-08 at 2.32.57 PM.png)



## Describe an optimization related to making writes to a file system perform better. When does this optimization help? What complexities does this optimization add to the operating system and to expected file system behavior?

Write buffering is an optimization related to making writes to a file system perform better. This optimization especially helps when several small changes are being made to a file. Buffering these writes can help amortize the write per-operation overhead. The complexities added to the operating system is the need to keep and maintain buffers for multiple writes. The operating system should also have at least two buffers so that while one buffer is full and being transfered to disk, the process can continue write buffering with another buffer. The complexities added to the file system behavior is that it must take care of data loss due to power failure (e.g. keep a journal). 



## Why is redirect on write a good strategy to use in file systems that are to be run on flash devices?

Flash derives have a requirement where first of all if a block is written to, it must be erased and in order to erase and rewrite a block, one must erase that entire block. Redirect on write is a good strategy because it prevents the need to erase on every write but instead writes in long contiguous buffered writes to the next available free space, eliminating the need to perform two I/Os (erase first, then write). Furthermore, if we don't use redirect, we might even have to erase data that is already being used, which results in another I/O where the data needs to be copied.

Flash memory can only be written once without requiring a large and performance-expensive erase cycle. Thus, a file system that does not perform writes in place, but instead writes new data in a new location and updates pointers, will fit better into the flash paradigm. Since there are no overwrites of existing data, the flash’s write-once disadvantage can be largely ignored. Since flash memory has equal performance for sequential and random access, redirect-on-write’s resulting scattering of data throughout the memory will not impose performance penalties.



## Unix-style operating systems such as Linux keep two types of in-memory kernel-level data structures to keep track of activities involving open files: open file instance descriptors and in-memory inodes. What is the purpose of each?

In-memory inodes are inodes that are read into memory. Reading into memory gives the user fast access to file information without needing to access the disk (slower). Open file instance descriptors (OFIDs) are used to keep track of specific instances of opens of a file by processes, and the characteristics of those opens. 

The in-memory inode serves as a kernel data structure to keep track of the fact that the file is open, to provide access to its inode metadata, and to provide information about which of its blocks are in memory and which are still on the disk. It is shared by all processes that have the file open, and thus is used to record things relevant to all processes using the open file. The open file instance descriptors are used to keep track of specific instances of opens of a file performed by processes and the particular characteristics of that open, such as whether it is open just for read or for read/write, whether read-ahead is requested, etc. While multiple processes can share such an instance, they only do so when sharing things like the mode of the open and the current pointer into the file are intended.



## Operating systems typically have two interfaces related to device drivers: the Device Driver Interface (DDI) and the Device/Kernel Interface (DKI). What is the purpose of each of these interfaces? Describe one kind of functionality you would be likely to find in the DDI and one functionality you would be likely to find in the DKI.

A device driver encapsulates knowledge of how to use a device. It maps standard operations to device-specific operations. Device Driver Interface is a top-end device driver entry-points (such as `open`, `close`, `read`, and `write`). On the other hand, Device/Kernel Interface specifies bottom-end services OS provides to drivers. It specifies things drivers can ask the kernel to do (such as memory allocation, data transfer and buffering). This is analogous to an ABI for device driver writers because it is OS specific.





## What factors determine the largest possible size for a file in the FAT file system? What factors determine the largest possible size for a file in the Unix System V file system?

The width of FAT (File Allocation Table) determines the max file size. Assume each cluster is 512 bytes. Then the max file size is 2^(width of FAT) * 512 bytes since each FAT entry contains the number of next cluster. Thus the width of FAT determines how many clusters it can represent. There are 10 direct blocks, 1 indirect block, 1 double indirect block, 1 triple indirect block in the Unix System V file system. The number of bytes in a block and number of bytes of a pointer determine the max file size. Assuming 4k bytes per block and 4 bytes per pointer. Then 10 direct blocks can represent 10 * 4K bytes = 40 bytes. Indirect block can represent 1K * 4K = 4M bytes. Double indirect block can represent 1K * 4M = 4G bytes. Triple indirect can represent 1K * 4G = 4T bytes.



## What would be the potential benefits of having a file system perform read-ahead automatically based on observed behavior? What would be the potential costs of doing so?

The idea of read-ahead is to request blocks from the disk before any process asked for them. The benefit is that when we read-ahead, the data will be stored in the cache. Once the process requests that data, it does not need to go all the way to the file system to fetch the data. But instead the process can access the data from the cache, which is much less cost than accessing data in disk. However, the potential cost of doing so arises when it read-ahead the wrong data, which results in a waste due to extra access time reading unwanted blocks. It may also waste buffer space on unneeded blocks.



## Why, for reliability purposes, is it beneficial to write out the data associated with a file update to persistent storage before writing out the metadata associated with the same update?

Crash consistency



## One approach to keeping track of the storage space used by a particular file on a device would be linked variable extents, in which the file descriptor would point to a section of the device where some variable amount of space had been allocated to the file. The last two words of that space would be a pointer to the next extent used to store more of the file's data and a length of that extent. What advantages would this approach have over the Unix-style block pointers stored in an inode? What disadvantages?

This approach could support arbitrarily large files. Also, it could do so with relatively few linked segments, assuming memory had not yet been fragmented, since each of the variable length extents could be quite large. One disadvantage is that traversing the file will require some extra I/Os. How many would depend on how many extents were in the file, which would depend on how the file was created (many small writes might lead to many small extents) and fragmentation issues. A second disadvantage is that it would be impossible to support sparse files with this particular approach.

# Week 9

## Security

- Authentitcation and authorization
- Access Control: Access control is the idea that the OS can control which processes access which resources, and thus enforcing security policies. To get access, issue request to OS.
- Different ways of implementing access control in the OS
  - Access Control Lists (ACLs): For every resource, we maintain a single list managed by the OS, in which it specifies who can access the resource. (e.g. each file contains information of read, write, execute permissions for owner, group, and other)
  - Capabilities: A data structure (a collection of bits) that tells us what objects a subject can access. Essentially, a set of tickets. Possession of the capability for an object implies that access is allowed. (e.g. file descriptor)
- Encryption and decryption
  - different cryptographic methods
  - How to use a PK crypto system
- Password salting: a salt is random data that is used as an additional input to a one-way function that "hashes" data, a password or passphrase. 

## In what way is a file descriptor like a capability?

When a process issues a request to OS for a file, the resource manager puts the file descriptor in the process' file descriptor table and returns its index. The process can access the file through the file descriptor. This is similar to capability because capability is like a ticket. Possesion of the capability for an object implies the access is allowed.



## Describe how a certificate allows us to securely obtain a public key for some other party. What information, in addition to the certificate itself, must we have to be sure of the certificate's validity? Why?

A public key certificate allows us to securely obtain a public key by encrypting it with another trusted party's private key. Since that other trusted party is the only one wth its private key, we can decrypt the certifiate using the trusted party's public key, and if the digital signature provided with the certificate is valid, we can be sure that the public key we received is the one we were looking for. This implies that we need, in advance, the public key of the trusted party. Often these come with installed software like web browsers.



## If you use capability to provide access control in a distributed environment, what extra challenges do you face that you do not face when using them in a single machine environment?

Capability is a data structure that tells us what objects a subject can access. It's like a ticket. In single machine environment, we did not have to worry about forgery of the capability because the OS prevents that. However, in distributed environement such as network environment, we need to make sure no other clients can copy the capability when the server sends the capability to a client. For example, the capability must be encrypted before sending to the client.



## A typical secure session over the Internet uses both symmetric and asymmetric cryptography. What is each used for, and why is that form of cryptography used for that purpose?

Asymmetric cryptography is used to establish a establish a session key using private and public keys. Then symmetric cryprography is used for communication using this session key. They are used in such a way because we want to use a symmetric key to establish a secure connection. But we need the server and the client to agreen on the same session key to do so. It is dangerous to simply send the key over the network. Thus, we need to use asymmetric  cryptography as a means of sending session key. Imagine Alice wants to share a session key with Bob. Alice would first encrpt the session key with Bob's public key. Then encrypt it again with Alice's private key. Alice sends this key to Bob. When Bob receives it, Bob will decrypt the key with Alice's public key. Check the digital signature to verify it is indeed from Alice. Finally, Bob decrypt the key again with Bob's private key, and thus obtain the session key. 

Asymmetric cryptography is used for initial authentication of one or both parties, and for secure session key establishment. Asymmetric crypto works well for this since distribution of public keys allows authentication of an otherwise unknown partner, which, with Diffie-Hellman key exchange, allows secure session key establishment. Symmetric cryptography is used for bulk transport of data once a session key is established. It is not as well suited for bootstrapping authentication and key establishment, but is computationally cheaper than asymmetric crypto, and thus is a better choice once a session key has been established.



## Why aren't symmetric cryptography ideal for prividing authentication? 

Because key distribution can be a problem. 

# Week 10

## Distributes Systems

- Design goals
- What workloads are suitable for distributed processing?
  - Map Reduce: A method of dividing large problems into small pieces. 
    - Map: Perform the function on each piece on a separate node
    - Reduce: Combine the results to obtain output
- Reliability and security
  - Consensus: the idea of consenus is to achieve simultaneious, unanumous agreement even in the presense of node & network failures
  - Security



## What is the purpose of a callback in AFSv2?

In distributed system, we want to minimize the interactions between the server and the client because it is costly. Thus, client can keep a read cache on its side whenever it reads something from a server. Next time the client accesses it, the  client needs not to reach out to the server. A callback is simply a promise from the server to the client that the server will inform the client when a fie that the client is caching has been modified.



## What is meant by horizontal scalability in a distributed system? Why is it good?

Horizontal scalability is being able to expand a distributed system by simply adding more machines to it. This is good becase machines (servers) can be cheap and thus make scaling the distributed system cheap as well. These machines can also fail or break individually and be replaced wihout much impact to the system.



## What is an advantage of using a stateless protocol in a distributed system? 

Stateless protocols like Https allows servers and distributed systems to resume its operations after a crash or restart without the same state of a previous execution. If doesn't require any type of session tracking or saving and restoration mechanism. Thanks to this nature of stateless, stateless protocol is relialble and scalable since client supplies necessary context with each request and successor server needs no memory of past events.

A stateless protocol has at least two advantages. First, it has better reliability properties. Since the remote partner in a stateless exchange does not try to maintain information about the state of the exchange, failure of the local partner causes no problems. The local partner needs to expend more effort, perhaps, for each communication, but he also
need not try to maintain any state about the remote partner, so failure of that machine will be easier to recover from. Second, for servers handling large numbers of clients, stateless protocols have important scaling advantages. Since the server need not remember information about its hundreds or thousands of clients, it can potentially handle more clients. Since each client is likely to work with only a few servers at once, on the other hand, there is no scaling issue offloaded onto them. A possible third advantage is that stateless protocols allow more ready horizontal scaling of systems. Each request can be sent to a different machine, so the load balancing problem is easier. Other advantages might be acceptable, depending on details.*



## What is the reason for having a client stub in an RPC system? What functionality does it have?

Remote procedure call system is a system to make the process of executing code on remote machine as simple and straightforward as calling a local function. A client stub is a piece of code that contains each of the functions specified in the interface. Internally, each of these functions in the client stub do all the work needed to perform the remote procedure calls. To the client, the code just appears as a function call. Specifically, it takes arguments from clients and serializes the message and send it to the server. It also deserialize message coming back from the server.  



## What kind of workloads will benefit most from distributed systems based on the principle of horizontal scalability? Why?

Embarrassingly parallel jobs will benefit most from distributed systems. Embarassingly parallel jobs refer to problems where it's really easy to parallelize them (e.g. the data sets are easily divisible and exactly the same things are done on each piece). We can just map them out among the nodes and let each go independently. Finally, we can reduce the results from different nodes into one.





## 



