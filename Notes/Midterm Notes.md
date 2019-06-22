# Process



## What is a process?

The definition of a process is a running program. A program is just a bunch of instructions sitting on a disk.



## What does a process do?

A process can read or update the memory when it is running. This memory region the process can address is called **address space**. A process can only access its own address space. It can't access other processes' address spaces. Apart from its address space, a process can also access special registers such as **program counter**, **stack pointer**, and **frame pointer**. A program counter tells us which instruction of the program is currently being executed; a stack pointer keeps track of the call stack; a frame pointer points to the start of the stack frame and does not move for the duration of the subroutine call.



## How to provide the illusion of many CPUs? In other words, given a few physical CPUs avaialble, how can the OS provide the illusion of an infinite supply of CPUs?

The technique OS uses to create this illusion of inifite CPUs is called **virtualization** of the CPU. So what does it mean to virtualize the CPU? To virtualize the CPU, the OS runs one process, then stops it and runs another, and so forth. The OS does this so fast that it creates the illusion of having a CPU for each process. This technique is called **time sharing** of the CPU.



## How does OS switch between processes? 

This low-level implementaion of switching between processes is called the **mechanisms**. For example, the OS uses a mechanism called **context switching** to switch between processes. We will see how context switching is done later. 



## How does OS determine which processes to switch?

Okay, so the OS knows how to switch between processes, but then how does it determine which processes to switch? This high-level algorithm of choosing which processes to switch is called the **policy**. For example, the OS follows a policy called **scheduling policy** to determine which processes to switch. We will look at how scheduling works in details later.



## How is a process created from a program?

First, the OS loads its code and any static data into memory. Then the OS also allocates some memory for stack and heap. Finally, the OS does some I/O setup, specifically, it opens three default file descriptors (i.e. stdin, stdout, stderr). Now, the process starts at the entry point (i.e. `main()`)



## What are the states of a process?

A process is in either **running**, **ready**, or **blocked** state. In running state, the process is running on a processor; in ready state, a process is ready to run and waiting to be scheduled; in blocked state, a process is not ready to run until some other event takes place. A common example is waiting for an I/O request.



## How does the OS keep track of all the processes?

The OS keeps some kind of **process list** that tracks all the processes. When an I/O event completes, the OS will look at this list and make sure to wake the correct process and ready it to run again. For each process, the OS also keeps track of its **register context**. When a process is descheduled, the OS will store the contents of its registers into register context, then perform context switch.



## What is Limited Direct Execution?

Direct exucution is the idea that just run the program directly on the CPU. The OS trusts the process will not do anything harmful and will not occupy the CPU forever. This is impossible. Thus, we need to limit what a process can do. 



## How does OS make sure a process only performs restricted operations? More specifically, a process must be able to perform I/O, but without giving the process complete control over the system. How does OS do that?

A example of restricted operation is writing a file that is read only. Without the file system checking the permission, this can ruin the whole point of setting permission. The solution is to have two processor modes. In **user mode**, I/O operations are restricted. If OS detects the attempt to access I/O in user mode, it will kill the process. In **kernel model**, which the OS runs in, priviledeged operations such as I/O requests are allowed. All user processes run in user mode. So what should a user process do when it wished to perform some kind of priviledged operation? The process must perform something called **system call**. A system call is a function call that request the kernel of the OS to provide a service such as I/O access. 



## What is trap instruction?

User programs perform system calls to handle priviledged instructions. In a system call, there is a trap instruction, which switch the mode of the processor from user mode to kernel mode. To execute a system call, a program must execute **trap** instruction, which simultaneously jumps into the kernel and raises the priviledged level to kernel mode. When the priviledged instructions are done in the kernel mode, the OS calls **return-from-trap** instruction to return into the user program while simulteneously reducing the priviledge level back to user mode. Before executing a trap, the processor will push the program counter, flags, and a few other registers onto **kernel stack**; the return-from-trap will pop these values off the stack and resume execution of the user-mode program. But how does the  trap know which code to run inside the OS? The code is called **trap handlers**. The kernel creates a **trap table** at boot time, which maps from a **system-call number** to a location of corresponding trap handler. The user code is responsible for specifying system-call number when they execute trap instruction. 

![Screen Shot 2019-04-27 at 9.08.19 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-27 at 9.08.19 PM.png)





## How does the OS regain control of the CPU so that it can switch between processes?

Note: When a process is running on a CPU, that means OS is not running on the CPU.

The OS uses **timer interrupt** to regain control of the CPU. A timer device can be programmed to raise an interrupt every so many milliseconds. This is possible due to hardware feature.



# Scheduling

Note: a job is equivalent to a process

## What is turnaround time?

```
Turnaround Time = Time Completion - Time Arrival
```



## How does FIFO scheduling perform on turnaround time?

FIFO performs poorly on turnaround time when a number of relatively-short potential consumers of a resource get queued behind a heavywight resource consumer. This is known as **convoy effect**.

![Screen Shot 2019-04-14 at 7.13.07 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-14 at 7.13.07 PM.png)

(Assume Job A, B, C arrived at almost the same time except B arrived slightly later than A and C arrived slightly later than B)



## How does SJF scheduling perform on turnaround time?

SJF performs pretty well on turnaround time.

![Screen Shot 2019-04-14 at 7.18.31 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-14 at 7.18.31 PM.png)

(Assume Job A, B, and C arrived all at once)

However, if B and C arrived shortly after A, then we will end up with convoy problem just like in FIFO.

![Screen Shot 2019-04-14 at 7.20.17 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-14 at 7.20.17 PM.png)



## How does STCF (Shortest Time-to-Completion First) scheduling perform on turnaround time?

Note: a.k.a Preemptive Shortest Job First (PSJF)

STCF scheduling is a variation of SJF scheduling, where the OS can preempt job. In other words, the OS can stop the process in the middle and perform context switch to run another process. When a new job comes in, the OS preempt the current process, then pick the process that has the shortest time to completion. 

![Screen Shot 2019-04-14 at 7.25.56 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-14 at 7.25.56 PM.png)





## What is Response Time？

```
Response Time = Time First Run - Time Arrival
```



## How does SJF perform on response time?

It doesn't perform well on response time. Assume job A, B, and C arrive at the same time.

![Screen Shot 2019-04-14 at 8.53.05 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-14 at 8.53.05 PM.png)

Job C must wait for job A and job B complete before it runs for the first time.



## How does RR (Round Robin) perform on response time?

Note: RR is sometimes called **time-slicing**

The idea of RR is simple: instead of running jobs to completion, RR runs a job for a **time slice** and then switches to the next job. 

![Screen Shot 2019-04-14 at 8.58.26 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-14 at 8.58.26 PM.png)





## What is Multi-Level Feedback Queue (MLFQ)?

The goal of MLFQ is to design a scheduler that both minimizes response time for interactive jobs while also minimizing turnaround time without a prior knowledge of the job lengh.

- Rule 1. If Priority A > B, A runs.
- Rule 2. If Priority A = B, then A and B run in RR fashion.
- Rule 3. When a new job enters the system, it is placed at the highest priority.
- Rule 4. Once a job uses up its time allotment at a given level, (regardless of how many times it has given up CPU), its priority is reduced.
- Rule 5. After some time period S, move all the jobs in the system to the topmost queue.









# Memory



## What is space sharing?

Space sharing is the idea that multiple processes sharing the memory.

![Screen Shot 2019-04-15 at 4.54.25 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-15 at 4.54.25 PM.png)





## What is Address Space?

An address space is the running program's view of memory in the system. Every process performs instructions with its own virtual address, and the OS makes sure the instructions are actually performed on physical address. We call this the OS is **vritualizing memory** because all the processes are running on top of a single physical memory, but each process thinks it has its own memory. The address space consists of three parts—code, stack, and heap. 

![Screen Shot 2019-04-17 at 8.05.17 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-17 at 8.05.17 AM.png)

 The code of the program have to live in memory somewhere, and thus they are in the address space. The stack keeps track of where it is in the function call chain as well as to allocate local variables. The heap is used for dynamically-allocated memory.



## What is external fragmentation?

External fragmentation is the idea that the free space gets chopped into little pieces of different sizes and thus is fragmented; subsequent requests may fail because there is no single contiguous space that can satisfy the request, even though the total amount of free space exceeds the size of the request.

![Screen Shot 2019-04-17 at 8.26.41 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-17 at 8.26.41 AM.png)



## What is internal fragmentation?

Internal fragmentation is the idea that an allocator hands out chunks of memory bigger than requested, and thus there is a waste occurs inside the allocated unit.



## What is a free list of the memory?

A free list is a data structure containing references to all of the free chunks of space in the managed region of memory.

![Screen Shot 2019-04-17 at 8.46.16 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-17 at 8.46.16 AM.png)



## Why does `free(void *ptr)` only require the pointer to allocated memory, and not require the size of the allocated memory to free?

An allocator store a little bit of extra information in a header block just before the handed-out chunk of memory.

 ![Screen Shot 2019-04-17 at 9.36.39 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-17 at 9.36.39 AM.png)



## What is magic number?

A magic number is a value for sanity check. We want to make sure we are looking at the right block of memory.



## What is best fit for managing free space?

Best fit strategy searches through the free list and find chunks of free memory that are as big or bigger than the requested size. Then, return the one that is the smallest in that group of candidates.



## What is worst fit for managing free space?

The worst fit strategy is the opposite of best fit strategy: find the largest chunk and return the requested amount. The worst fit strategy is better than best fit strategy in terms of external fragmentation.



## What's the advantage of first fit over best fit/worst fit for managing free space?

The first fit strategy simply finds the first block that is big enough and returns the requested amount to the user. The benefit of this strategy is that we don't need to do exhausitve search of all the free spaces, and thus it's fast.



## What is next fit for managing free space?

Instead of doing first fit search at the beginning of the free list, next fit algorithm keeps an extra pointer that holds the location within the list where one was looking last. The idea is to spead the searches throughout the list more uniformly, thus avoiding many small chunks of free spaces at the beginning of the list.



## What's the idea of Segragated Lists for managing free space?

If a particular application has one (or a few) popular-sized request that it makes, keep a separate list just to manage objects of that size; all other requests are forwarded to a more general memory allocator. The advantage of having separated lists for particular size is that external fragmentation is very less likely to happen.



## What's the idea of Buddy Allocation for managing free space?

When a request for memory is made, the search for free space recursively divides free space by two until a block that is big enough to accommodate the request is found (and a further split into two would result in a space that is too small). The disadvantage is that it can cause internal fragmentation. The benefit of buddy allocation is that it makes the merging of free spaces very eazy. When a block is freed, the allocator checks if its "buddy" is freed. If so, it coalesces those two blocks. And this will be performed recursively.

![Screen Shot 2019-04-17 at 11.13.31 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-17 at 11.13.31 AM.png)



## How can we relocate process with virtual addresses to physical addresses in a way that is transparent to the process? And how do we maintain control over which memory locations an application can access, and thus ensure that application memory accesses are properly restricted?

We have two hardware registers within each CPU: **base** register and **bounds **register. In this setup, each program is written and compiled as if it is loaded at address zero. However, when a program starts running, the OS decides where in physical memory it should be loaded and sets the base register to that value. The physical address is computed as `physical address = virutal address + base`. Bounds register is used to check if the address is valid. Before fetching the data, the processor makes sure the memory reference is within the bounds. If a process requests a virtual address that is greater than the bounds, or one that is negative, the CPU will raise an exception, and the process will be terminated.



## What is Memory Management Unit (MMU)?

MMU is the part of CPU that helps with address translation (i.e. base and bounds registers)



## What is Segmentation?

The idea of segmentation is to reduce the amount of allocated, but unused memory space we have between the stack and the heap.

![Screen Shot 2019-04-20 at 4.14.22 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-20 at 4.14.22 PM.png)

A segment is just a contiguous portion of the address space of a particular length. In the figure above, we have three segmenets: code, stack, and heap. The idea of the segmentaion is this: instead of having base and bounds registers for the entire process, we will have base and bounds registers for each segment of the process. The benefit of this is that we can place each segment independently in physical memory, and only used memory is allocted space in physical memory.

![Screen Shot 2019-04-20 at 4.20.26 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-20 at 4.20.26 PM.png)





## During address translation, how does the hardware know which segment we are refering to?

Note: Assume there are 3 segments in the address space

The hardware looks at the most significant two bits to tell which segment we are refering to. 00 would refers to code segment, 01 refers to heap segment, and 11 referes to stack segment. After the hardware knows whihc segment it is, it simply use the offset to compute the physical address and to check the bounds. `physical address = base + offset` and `check if offset < bounds`



## What is protection bit for?

The protection bit is for code sharing. Multiple process might use the same code, and we don't want to duplicate the same exact code in phyiscal memory. Protection bit tells us if a certain segment is readable, writable, and executable.







## What is Paging?

The problem of segmentaion is that the segment itself can have internal fragmentation. Instead of splitting up a process's address space into some number of variable-sized logical segments (e.g. code, heap, stack), we divide it into fixed-sized units, each of which we call a **page**. Both virutal memory and physical memory are divided into pages. Each slot of page in the physical adress is called a **page frame**.

![Screen Shot 2019-04-21 at 1.55.30 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-21 at 1.55.30 PM.png)

(Address Space)

![Screen Shot 2019-04-21 at 1.56.02 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-21 at 1.56.02 PM.png)

(Phyiscal Address)



In order to keep track of where each virtual page of the address space is placed in phyiscal memory, the OS keeps a **page table** for each process for address translations. 



## How does address translation work in paging?

Similar to address translation in segmentation, the hardware looks at the most significant bits to determine which virtual page we are refering to (this is called **virtual page number (VPN)**). Then it looks at the page table to find the correspoding page frame in the physical address (this is called **physical frame number (PFN)**). Finally, it fetches the data by replacing virutal page number with physical frame number (the offset bits remain the same).

![Screen Shot 2019-04-21 at 2.15.48 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-21 at 2.15.48 PM.png)

![Screen Shot 2019-04-21 at 2.16.49 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-21 at 2.16.49 PM.png)





## What is linear page table?

The OS indexes the array by the virtual page number, and look up the page-table entry at that index in order to find the desired physical frame number.



## What is page table entry?

In the page table, along with the physical frame number, it also stores some useful information. For example, a page table entry includes a **valid bit**, **protection bit**, **present bit**, **dirty bit**, and **reference bit**.



## What does valid bit in the page table entry tell us?

A valid bit tells us whether the particular translation is valid; for example, all the unused space in-between the stack and heap will be marked invalid.



## What does protection bit in the page table entry tell us?

Just like in segmentaion, a protection bit tells us whether the page could be read from, written to, or executed from.



## What does present bit in the page table entry tell us?

A present bit tells us whether this page is in physical memory or on disk (i.e. it has been swapped out). The act of accessing a page that is not in physical memory is commonly refered to as a **page fault**. When a page fault occures, the OS runs page-fault handler and swap the required page back into memory. 



## What does dirty bit in the page table entry tell us?

A dirty bit tells us whether the page has been modified since it was brought into memory. The idea is that when we move a page from disk to memory, what we are doing is actually copying from disk to memory. Before swapping out dirty pages, we need to write the page back to the disk.



## How do we not block the processes when we swap out dirty pages for writing to disk?

Do eager writing. Background thread constatnly writing not active dirty pages to disk.





## What does reference bit in the page table entry tell us?

A reference bit tells us whether the page has been accessed, which is useful to determine whether this page is popular and thus should be kept in memory. This is useful during page replacement. (clock algororithm)



## Page table is stored in memory. It is expensive to translate virtual address if we search the page table for each virtual address translation. How do we speed up address translation?

We will use something called **translation-lookaside buffer (TLB)**. A TLB is part of the chip's memory management unit (MMU), and is simply a cache of popular virtual-to-physical address translations (i.e. page table entry). Upon each virutal memory reference, the hardware first checks the TLB to see if the desired translation is held therein. If it does, we have a **TLB hit** and the translation is performed. Otherwise, we have **TLB miss**, and we have some more work to do. We have to access the page table to find the translation, and update the TLB. Once the TLB is updated, the hardware retries the instruction, and will result in TLB hit, and the translation is performed. TLB improves the efficiency due to locality.



## What is temporal locality and spatial locality?

Temporal locality is the idea that an instruction or data item that has been recently accessed will likely be re-accessed soon in the future (e.g. loop variables or instructions in a loop). Spacial locality is the idea that if a program accesses memory at addres `x`, it will likely soon access memory near `x`. 



## How to manage TLB contents on a context switch? In other words, how do we make sure one process does not use page table entries from other processes?

One way to solve this is to flush the TLB upon context switch. However, this is pretty costly if we context switch frequently as we would encounter TLB miss a lot. A better solution is to have a bit called **address space identifier (ASID)**. This is kind of like **process identifier (PID)**. Each process will have different ASID. Each page table entry in the TLB will contain corresponding ASID. 

![Screen Shot 2019-04-21 at 4.18.41 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-21 at 4.18.41 PM.png)





## What is Swap Space?

Swap space is a reserved space on the disk for moving pages back and forth. If the memory is full when we want to swap in a page, old page will be replaced. The way we choose the page to be replaced follows **page-replacement policy**.





## What happens when a program fetches some data from memory?

First it checks TLB. If TLB hit occures, the hardware can immediatly translate the virtual address to physical address and fetch the data. If TLB miss occures, the processor goes to the memory and checks the page table. If the target page's present bit is set, then we store the page table entry to TLB, and retries the instruction. It will cause TLB hits next time. Otherwise, the target page is not in memory. The page-fault handler will be run. It goes to the dist and swap in the desired page to memory, and retries the instruction. This will cause TLB miss, but the corresponding page entry's present bit is set. Thus, it will be loaded to TLB, then when it retries the instruction again, it will cause TLB hit.



## What are some good page replacement policies?

Least-Frequently-Used (LFU) policy and Least-Recently-Used (LRU) policy are good. They work very well because of the princple of locaility: programs tend to access certain code sequences and data structures quite frequently; we should thus try to use history to figure out which pages are important, and keep those pages in memory when it comes to eviction time. But implementing these policies are very expensive as there are so many pages we need to scan through.



## How do we approximate LRU policy?

A naive solution is to have a timestamp for each page entry. But it is very expensive to search through all the pages and find which entry is least recently used. We will approximate LRU instead. Each page entry will have something called **use bit**, which indicates whether the page is used or not. We will also have a **clock hand** pointing to one of the page to start with (it doesn't really matter which page). When a replacement must occur, the OS checks if the currently-pointed page P has a used bit of 1 or 0. If 1, this implies that page P was recently used and thus is not a good candidate for replacement. Thus, the use bit for P set to 0, and the clock hand is incremented to the next page. We repeat this process until we find a process with used bit 0. Then we replace with that process as this implies that the page hasn't been used since the clock hand visits the page last time. This is called **clock algorithm**.  



## What is thrashing?

Trashing refers to the idea that memory is oversubscribed, and the system constantly be paging. To resolve this isssue, the OS temporaility move some processes to the disk, thus creates enough space for paging.











# Thread



## What's the difference between context switching processes and threads?

While both of them need to store the register state of old program, former one stores it in **process control block (PCB)**, but the latter one stores it in **thread control block (TCB)**. Another important difference is that when context switching between threads, the address space remains the same (i.e. there is no need to switch which page table we are using). Note that in multi-threading process, there are multiple stacks in the address space.

![Screen Shot 2019-04-22 at 8.24.55 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-22 at 8.24.55 AM.png)



## What is parallelism?

Parallelism is the idea of speeding up the execution of a task by running the task with multiple processors simultaneously (e.g. adding two large arrays together)



## What are race condition, critical section, and mutual exclusion?

A **race condition** is the behavior that the result of a system is not deterministic, and depends on the sequence of execution (e.g. incrementing shared counter with multiple threads). **Critical section** is a piece of code that accesses a shared variables. To resolve this issue, we will do something called **mutual exclution**. The idea of mutual exclusion is that when one thread is executing within the critical section, the others will be prevented from doing so.



## What does excuting atomically mean?

It means to execut a series of instructions as a unit, without being interrpted in the middle.



## One simple method for mutual exclusion is to disable interrupts. What's the downsides of doing this?

Scheduled processes will not be executed until the atomic instruction is complete. Devices may be idle, and the system will be less responsive.







## What is synchronization primitives?

Synchronization primitives are simple mechanisms provided by the hardware or OS to the users for the purposes of supporting thread or process synchronization.



## How does test-and-set instruction help building spin locks?

Test-and-set instruction (a.k.a atomic exchange) is a hardware instruction that returns the old value pointed to by `ptr`, and simulteneously updates said value to `new`.

```C
int TestAndSet(int *old_ptr, int new) {
  int old = *old_ptr;
  *old_ptr = new;
  return old;
}
```

We can implement spin locks as the follow.

```C
void lock(lock_t *lock) {
  while (TestAndSet(&lock->flag, 1) == 1);
}
```





## How does compare-and-swap instruction help building spin locks?

Compare-and-swap instruction tests whether the value at the address specified by `ptr` is equal to `expected`; if so, update the memory location pointed to by `ptr` with the new value. If not, do nothing.

```C
int CompareAndSwap(int *ptr, int expected, int new) {
  int actual = *ptr;
  if (actual == expected)
    *ptr = new;
  return actual;
}
```

We can implement spin locks as the follow.

```C
void lock(lock_t *lock) {
  while (CompareAndSwap(&lock->flag, 0, 1) == 1);
}
```





## How does load-linked and store-conditional instructions help building spin locks?

Here's how load-linked and store-conditional instructions work.

```C
int LoadLinked(int *ptr) {
  return *ptr;
}

int StoreConditional(int *ptr, int value) {
  if (no update to *ptr since LoadLinked to this address) {
    *ptr = value;
    return 1; // succeess
  } else {
    return 0; // failure
  }
}
```

We can implement spin locks as the follow.

```C
void lock(lock_t *lock) {
  while (1) {
    while (LoadLinked(&lock->flag) == 1);
    if (StoreConditional(&lock->flag, 1) == 1)
      return
  }
}
```

When two different threads both want to call `StoreConditional`, only one of them will succeed.





## How does ticket lock work?

Ticket locks ensure progress for all threads. It uses fetch-and-add instruction. When a thread wishes to acquire a lock, it first does an atomic fetch-and-add on the ticket value; that value is now considered this thread's "turn". The global shared `lock->turn` is then used to determine which thread's turn it is; when `(lock->turn == myTurn)` for a given thread, it is that thread's turn to enter critical section. When unlocking, simply increment `lock->turn`.

```C
int FetchAndAdd(int *ptr) {
  int old = *ptr;
  *ptr = old + 1;
  return old;
}
```





## How can we develop a lock that does't needlessly waste time spinning on the CPU?

One solution is to use `yield()`. Basically a thread checks if the lock is held. If it is, it just gives up the CPU and let other processes to use it. While this is better than simply spinning on the CPU, it is still costly if you have, say, 99 threads doing it because there is one thread holding the lock (imagine you are running 100 threads in Round Robin fashion). Worst case senario, the one thread holding the lock may never get scheduled while other threads keep getting scheduled and yielding. This problem is known as starvation. The solution is to use `park()`, which puts the process into sleep. We also use a queue to keep track of which threads are waiting to acuqire the lock.

```C
void lock(lock_t *m) {
  while (TestAndSet(&m->guard, 1) == 1); // although there is still a spinning,
                                         // the time of spinning is guranteed to be
                                         // reasonably shorter as there is only a few
                                         // lines of code inside lock instead of
                                         // user defined critical section.
  if (m->flag == 0) { // no thread is in critical section, no need to sleep
    m->flag = 1;      // I'm in the critical section, other threads should not enter
    m->guard = 0;
  } else {
    queue_add(m->q, gettid());
    setpark(); // about to park: if unpark() is called in different thread, 
               // subsequent park() returns immediately instead of sleeping.
    m->guard = 0;
    park();
  }
}

void unlock(lock_m *m) {
  while (TestAndSet(&m->guard, 1) == 1);
  
  if (queue_empty(m->q)) 
    m->flag = 0;
  else
    unpark(queue->remove(m->q));
  
  m->guard = 0;
  
```



## When is it more suitable to use thread over process?

If you want to have parallel activities in a single program.





## What is Conditional Variables?

There are many cases where a thread wishes to check whether a condition is ture before continuing its execution. We call such varialbles conditional variables.



```C
cond_t empty, fill;
mutex_t mutex;

void *producer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    Pthread_mutex_lock(&mutex);
    while (count == MAX)
      Pthread_cond_wait(&empty, &mutex);
    put(i);
    Pthread_cond_signal(&fill);
    Pthread_mutex_unlock(&mutex);
  }
}

void *consumer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    Pthread_mutex_lock(&mutex);
    while (count == 0) 
      Pthread_cond_wait(&fill, &mutex);
    int tmp = get();
    Pthread_cond_signal(&empty);
    Pthread_mutex_unlock(&mutex);
    printf("%d\n", tmp);
  }
}
```





## How does Approximate Counter perform better than traditioanl multithreaded counter?

The approximate counter works by representing a single logical counter via numerous *local* physical counters, one per CPU core, as well as a single *global* counter. Specifically, on a machine with four CPUs, there are four local counters and one global one. In addition to these counters, there are also locks: one for each local counter1, and one for the global counter. o locks: one for each local counter1, and one for the global counter. The basic idea of approximate counting is as follows. When a thread running on a given core wishes to increment the counter, it increments its local counter; access to this local counter is synchronized via the corresponding local lock. Because each CPU has its own local counter, threads across CPUs can update local counters without contention, and thus updates to the counter are scalable. However, to keep the global counter up to date (in case a thread wishes to read its value), the local values are periodically transferred to the global counter, by acquiring the global lock and incrementing it by the local counter’s value; the local counter is then reset to zero.



## What is Semaphores？

A semaphore is an object with an integer value that we can manipulate with two routines; `sem_wait()` and `sem_post()`.

```C
int sem_wait(sem_t *s) {
  // decrement the value of semaphore s by one
  // wait if value of semaphore s is negative
}

int sem_post(sem_t *s) {
  // increment the value of semaphore s by one
  // if there are one or more threads waiting, wake one
}
```





## What is Binary Semaphores?

Binary semaphore is an implementation of lock with semaphores.

```C
sem_t m;
sem_init(&m, 0, 1);

sem_wait(&m); // m = 0 now. No other thread will be able to enter critical section
/*
critical section here
*/
sem_post(&m); // m = 1 now. Other threads can enter.
```



## How to use semaphores for ordering primitives like conditional variables (Notification)?

```C
sem_t s;

void *child(void *arg) {
  printf("child\n");
  sem_post(&s); // if parent runs first, this will set s = 0. Now parent will stop waiting.
  return NULL;
}

int main(int argc, char *argv[]) {
  sem_init(&s, 0, 0);
  pthread_t c;
  Pthread_create(&c, NULL, child, NULL);
  sem_wait(&s); // if parent runs first, s = -1. Parent will wait. 
                // Otherwise child runs first and set s = 1. This sets it back to s = 0
                // and thus, immediately returns.
  return 0;
}
```



# What to expect from midterm

- scheduling algorithm (which one has best response time)
- paging algorithm (what heppens when cache miss)
- LRU / LFU
- clock algorithm
- what's the differene between pipe and socket?
- how `execv()` works (all the address space will be replaced)
- what happens after `fork()` ? (What's same in both parent and child: code and data are the same, stack and pid is different, copy on write)
- What's the difference between segmentation and paging? (What's the advantage and disadvantage of each)
- Why do we need virtual memory?
- Make sure you cover everything in slides
- Interprocess communication (read the link on website, it will be on midterm!!!)
- What is API and ABI? (Mentioned twice)
- What is the difference between internal fragmentation and external fragmentation? If you segmentation, do you have internal fragmentation or external fragmentation? If you use paging, do you have internal fragmentation or external fragmentaiton? 
- Working-set
- How does TLB work? Why do we need it?
- Given certain situation, should you use pipe or socket or signal?
- Why do we need shared memory in interprocess communication?
- What's the difference between spin clock, semaphor, and conditional variables?
- Around 10 short answer questions
- One question with many subsections
- Understand "process" question on sample
- What's the benefit of paging and virutal memory in terms of shared library? => one copy of it in the physical memory works
- Make sure you understand copy on write. What's the drawback of copy on write?
- How is mutex different from spin locks and semaphore?
- What's the definition of turnaround time, response time, and working-set?



# User-Mode Thread Implementation



## What's the difference between threads implemented in a user mode library and kernel implemented threads? What's the problem with user-mode thread implementation?

One problem is that when a system call block one of the thread (e.g. `read()`, `write()`: the caller can't do anything until the system call returns), it will block the entire process, and thus other threads are also blocked as from OS's point of view, they are just one process. Another issue is that the OS is not aware that a process is comprised of multiple threads, and thus cannot execute in parallel on the available cores. 





# Inter-Process Communication



## What is Inter-Process Communication (IPC)?

IPC refers to a mechanism where different processes interact with each other. The type of interactions can be divided into two broad categories: coordination of operations (e.g. synchronization) and exchange of data (e.g. pipe).



## What is pipe?  

Pipes are temporary files with a few special features, that recognize the difference between a file and an inter-process data stream. For example, if the reader exhausts all of the data in the pipe, but there is still an open write file descriptor, the reader does not get an EOF. Rather the reader is blocked until more data becomes available, or the write side is closed. Each program accepts a byte-stream input, and produces a byte-stream output. Byte streams are inherently unstructured (e.g. it may or may not be comma separated depending on the implementation), and it's the responsibility of the agent to interpret the input byte-stream correctly. The available buffering capacity of the pipe may be limited. If the writer gets too far ahead of the reader, the OS may block the writer until the reader catches up. This is called **flow control**.



## How is named pipe different from ordinary pipe?

While a normal pipe interconnects processes start by a single user, a named pipe can interconnect unrelated processes by opening the pipe by name. There are some problems with named pipes. Readers and writers have no way of authenticating one another's identities. If there are multiple writers, the readers have no way to tell which bytes come from which writer. The bytes can be mixed up, which is bad. Moreover, all readers and writers must be running on the same system, otherwise they can't share the named pipe (it's just a special file). 



## What is mailbox? How is it different from named pipes?

Mailboxes are inter-process communication mechanisms that is sort of an improvement of named pipes. Instead of reading and writing byte-stream data, each write is stored and delivered as a distinct **message**, and thus bytes from different writer is not mixed up. Each write is also accompanied by authenticated identification information about its sender, and thus the readers know where the message is coming from.



## What is Shared Memory?

The motivation behind shareing memory is to improve the performance. Since there is no need for system call to read/write the data, it is very fast. However, it makes the code complicated as the user needs to take care of the synchronization.



## What's the point of having out-of-band signals? Why can't we send signal through the network?

The idea of out-of-band signals is to have another communication channel specifically for signals apart from the normal requests. Imagine there are megabytes of queued requests, and we want to send a message to abort those queued requests. Since the network connection is FIFO, this is impossible with only one communication channel. The motivation behind having another reserved channel for signals is that we occasionally would like to send an important message to go directly to the front of the line.



# Working Sets



## What is Working Sets?

The motivation behind working sets is to implement LRU and RR together. Implementing LRU and RR naively doesn't work well as separate processes usually don't access the same page. We will be having the most recently used pages in memory belong to the process that will not be run for a long time. We will be losing the least recently used pages in memory belong to the process that is just about to run again. To resolve this issue, we will need to implement per-process LRU, which works well with RR. We call the number of optimal pages for a process the working set size of the process. Here, optimal means if we increase the number of page frames allocated to the process, it makes very little difference in the performance. If we reduce the number of page frames allocated to the process, the performance suffers noticeably. 

![Screen Shot 2019-04-27 at 10.27.33 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-27 at 10.27.33 AM.png)

Note the working set size will change over time. Some processes might need more pages and some migiht have more than enough pages. Processes that need more pages will steal page allocations from other processes that already have enough page allocations. This is called **page stealing**. Processes that reference more pages more often, will accumulate larger working sets. Processes that reference fewer pages less often will find their working sets reduced due to page stealing.





# Garbage Collection and Defragmentaion



## What is Garbage Collection?

Once in a while, the system initiates garbabge collection to remove all the resources that are not reachable from memory. For each resource, it keeps tracks of how many references are pointing. 



## Defragmentation

Defragmentation is the idea of removing the fragmentaion in physical disk. File reads and writes can progress orders of magnitude more quickly if logically consecutive blocks are stored contiguously on disk (so they can be read in a single operation with no head movement). But eventually, after many generations of files being created and deleted, the free space, and many files become discontiguous, and the file systems become slower and slower.

![FragmentationDefragmentation](/Users/one/Desktop/CS 111/Notes/FragmentationDefragmentation.gif)





# Real Time Scheduling



## How is real time scheduling different from traditional scheduling algorithms? 

In traditional scheduling, we cared about things like turnaround time and response time. But in real time scheduling, we care about timeliness. We want jobs to be done on certain time. Typically, in real time systems, we already know how long things will take. Starvation is acceptable (e.g. it's okay to update the monitor once a while, but it's not okay to delay the timing for acceleration or adjusting things in space shuttle.) Hard real-time system refers to a system that have strong requirements on when specified tasks are run. Failure to meet the deadline may result in system failure. Soft real-time system refers to a system that missing a deadline only degrades the performance or recoverable failures. 



# Stack Frames and Linkage Conventions



## What's the similarity between procedure calls and interruts/traps? What's the difference between them?

They are similar in a sense that we want to transfer the control to a handler. Before doing so, we need to store the current states of registers, and restore it when it is done. They are different in that procedure calls are invoked by the software whereas traps are invoked by the hardware.



## What's the mechanism for typical interrupt or trap?

There is a number associated with every possible interrupt or execution exeption (trap). There is also a table, initialized by the OS, that associates a program counter and process status word with each possible interrupt or execution exception. When an event happens that would trigger the trap, the CPU uses the interrupt number to index into the appropriate table entry. Then it loads a new PC and PSW from the entry to enter 1st level trap handler. In this handler, all of the general registers are pushed onto the stack. It also gathers necessary info and select 2nd level handler, which actually deals with the problem. After the 2nd level handler is done, it returns to the 1st level trap handler, which restores all of the saved registers. Finally, it executes return from the trap instruction to reload the PC and PSW saved at the time of trap. The execution resumes as if the trap never happened.

![Screen Shot 2019-04-27 at 2.28.56 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-04-27 at 2.28.56 PM.png)



# Object Modules, Linkage Editing, Libraries



## What's the difference among static, shared, and dynamically loaded libraries?

Static libraries are directly incorporated into the load module. Since many libraries are used across different program on the system, it will create many identical copies of the library. Moreover, librarys change over time with enhancement, and we have to rebuild the load module to incorporate the update. Shared libraries solve this issue. Shared libraries are loaded on run-time when the program starts. There may be some libraries that are very large but are seldom used, which slows down the program's start-up time. We can fix this issue with dynamically loaded libraries. The library is loaded as needed. 



# Software Interface Standards

## 

## What is API?

API is a document that includes a list of methods and their signatures (parameters / return types). It explains how to use the service they provide. The benefit of API is that developers need not worry about the implementaion. 



## What is ABI?

The motivation behind ABI is that when we download a program, we just want to run it. However, executable programs are compiled for a particular instructionset architecture and operating system. The structure of the executable is system dependent (i.e. `1010` might mean different things in different system. Maybe it means `int` in one ISA and means something totally different in another). An ABI is the binding of an API to an ISA. If I have a C program and want to distribute it to Windows users and Mac OS users. I have to compile one with Windows ISA and another with Mac OS ISA.



# 2018 Winter Midterm



## In a virtual memory system, why is it beneficial to have a dirty bit associated with a page?

If we don't have a dirty bit associated with a page, then we don't know whether the data in the page has been modified or not since it arrives the memory. When the page needs to be swapped out to the disk, we need to always write the page back to the disk since we don't know whether it has been modified. However, if we have a dirty bit, we check if the page has been modified. If it hasn't, we can simply swap the page out of memory, without writing it back to the disk, and thus less I/O is involved, so the swap is done faster.



## What is the relationship between a system's Application Binary Interface and its system call interface?

ABI is the binding of an API to an ISA. System call interface is a subset of ABI.



## Why can't shared libraries include global data?

Multiple processes accessing the shared global data at the same time would be a problem (i.e. synchronization problem).



## Describe how a trap instruction is used to implement a system call in a typical operating sytem

A system call will first save all the current registers onto kernel stack including program counter so that it knows where to come back later on. Then it executes a trap instruction, which jumps into the kernel and raises the privileged level to kernel mode. Next, the kernel uses the system call number to index into the trap table and finds which trap handler to run. After the trap is handled, the OS executes return-from-trap instruction, which restores registers from kernel stack and move to user mode. Finally, it will resume the user program.





## What is the relationship between the concept of working sets and page stealing algorithms?

Working sets of a process refer to the pages of the process available in the memory. Generally for any process, there is a optimal number of pages, where page fault rate is small enough and increasing the number of pages do not decrease page fault rate that much. However, the working sets of a process change over time. Page stealing algorithm is an algorithm to keep the working sets optimal over time. The idea is that processes that need more pages than current working sets size will steal the pages from other processes whose working sets are too large. 



## Name a performance metric that is likely to be maximizable using non-preemptive scheduling. Why is this form of scheduling useful to maximize this metric?

Throughput is the rate at which something is processesd. In non-preemprive scheduling, a process can't be interrupted in the middle, and thus likely to maximize the rate of processing something over time.



## Why does a worst fit algorithm for managing a free list handle external fragmentation better than a best fit algorithm?

Worst fit algorithm fits allocation requests into the largest free chunk of memory available, assuming no perfect fit is available. The remainder of that chunk will be as large as possible, meaning it will be well suited to match later requests. A best fit algorithm will choose the free chunk closest and larger in size to the requested allocation, which implies that the leftover free memory returned to the free list is likely to be a small chunk, poorly suited to matching future requests. The definition of external fragmentation is scattering small, useless chunks of free memory throughout the free list, so best fit is more likely to cause external fragmentation than worst fit.





## Both shared memory IPC and the processes' data areas after a Linux `fork()` operation would require the page tables of two processes to point to the same physical page frames. What would be different about the two cases (other than being caused by IPC vs. forking)?

Major difference is fork results in copy-on-write, while shared memory IPC doesn't.



## What is the purpose of a condition variable?

A condition variable is used to determine if some specific pre-defined condition has or has not occured. If the condition is satisfied, one or more of the blocked processes will be unblocked and permitted to run. The condition variable allows a process to wait for a specific condition without requiring the process to use a busy loop to check for the condition's occurrence. 





## In a system using demand paging, what operations are required when a TLB miss occurs? What are the possible outcomes of those operations?

TLB is a cache of page table entries. When a process asks for a memory access, the processor first checks if the address is even valid. If not, the process will be terminated because it tries to access invalid memory location. Otherwise, the address is valid. Then the processor checks if the translation exists in the TLB. If it does, it just translates the virutal address to physical address to access the data. Otherwise, it's TLB miss. The processor will then go to the page table in the memory, and see if the corresponding page exists in the memory. If it does, just bring that corresponding page table entry to TLB. Otherwise, the page is not in the memory. The processor then go to the disk and load the page onto the memory.



# 2017 Fall Midterm

## In a system using virutal memory techniques, what is the relationship between a page and a page frame?

In a system using virtual memory techniques, the virtual address space of a program is divided into small pieces called pages. Physical memory is also divided into small pieces of region called page frame. Pages of a process will be placed into these page frame slots in physical memory.



## Why are operating system ABIs of importance for convenient application software distribution?

The format of the executables depend on the Instruction Set Architecture and the operating system. An ABI is the binding of an API to an Instruction Set Architecture. It tells how a particular set of machines expect for the format of the executables. With ABI, application developers need not worry about the hardware differences. When producing the executables, they just need to create different format of executables for different ABI.





## Why is information hiding a good property in an operating system interface?

Primary benefit is to abvoid bugs arising from improper dependencies among modules. Another benefit is that it allows changes in module implementation without chaning the specification.



## When an operating system performs a context switch between process, what information must the OS save?

- Save general registers
- Save PC: A register that stores the address of the next instruction
- Save Stack Pointer: A register that stores the address of the last program request (e.g. function call)
- Save PS (program status): A register that stores various flags (e.g. zero flag)





## What is the purpose of a trap table?

The kernel creates a **trap table** at boot time, which maps from a **system-call number** to a location of corresponding trap handler. The OS tells the hardware the address of system call handlers. The user code is responsible for specifying system-call number when they execute trap instruction.



## What is race condition?

A **race condition** is the behavior that the result of a system is not deterministic, and depends on the sequence of execution (e.g. incrementing shared counter with multiple threads)



## Why is blocking a problem for user-mode threads? Why isn't it a problem for kernel-mode threads?

User mode threads block other threads of the same process. Kernel-mode threads do not block other threads of the same process, as other threads can be scheduled to run on the same or another core.



## Why does Shortest Time-To-Completion First scheduling require preemption?

New processes might have shorter time to completion, and we need to switch to newly added shorter ones. To switch, we need to interrupt the one we are processing.



## When a Unix-system follows a fork with an exec, what resources of the forked process are replaced? 

It will replace pretty much everything including code segment, data segment, and stack segment. General registers and PS/PC.





## What form of fragmentation do we still suffer if we use a paging memory management system? For a segmented paging system, how much fragmentation per segment do we see?

Internal fragmentation. Since the fragmentation only occurs in the last page, on average, we will see half the page of internal fragmentation.

# 2018 Summer



## In a multiprogramming system, why is a simple clock page replacement algorithm applied to the full set of pages likely to lead to poor performance?

Implementing LRU and RR naively doesn't work well as separate processes usually don't access the same page. We will be having the most recently used pages in memory belong to the process that will not be run for a long time. We will be losing the least recently used pages in memory belong to the process that is just about to run again. 



## What fundamental requirement for a modern general purpose operating system is met by providing a trap instruction in hardware? How does provision of this instruction enable the OS to meet that requirement?

switch from user mode to kernel mode, with handling context switching



## Why does the Shortest Job First scheduling policy typically result in better turnaround time than the First Come, First Served policy?

Assume jobs arrive at the same time. With FIFO, jobs that require only a little amount of time can be scheduled after a job that require a long time. Since turnaround time is defined as `(time to completion) - (time arrival)`, short jobs result in long turnaround time.



## Describe a way to allow for otherwise incompatible interface changes without sacrificing backward compatibility. Why might such changes be valuable?

- Version Control



## Describe two benefits of virtualizing memory addresses at the page level, including why virutal memory provides that benefit.

- Virtualizing memory addresses allow the pages not to be contiguous in phyiscal memory, and thus, better memory mangement because we can arrange them such that there is less fragmentaiton.
- Virtualizing memory addresses also allows the easier relocation of pages. When pages are swapped out and swapped in again, they may not be in the same physical address. However, we don't need to change the address of each individual line of the virtual memory. All we need to do is to update base register to the beginning of the phyiscal address of the page. 
- Virtualing memory address allows only some pages of the process phyiscally in the memory. 



## When load increases, throughput drops below maximum ideal values, but response time quickly grows to infinity. Why do these two metrics exhibit different behavior under the same condition of increasing load?

When load increases, the CPU just keeps swapping pages in and out as the memory is oversubscribed by too many processes. This is called thrashing. To resolve this, the CPU will drop some process to make more spaces. And thus there will be many more context switching, and page swapping, and thus throughput drops below maximum ideal values. The response time grows to infinity because when the CPU drops some processes, those processes will not be executed. Even worse, they may never be scheduled again. Thus, it quickly grows to infinity.



## Describe three techniques we can use to ensure proper behavior of critical sections, briefly indicating why each of them  can achieve that effect. 

- Disable interrupt
- Synchronization primitives
- Locks



## What is the difference between swapping and paging? What is each of these two technique used for?







## What kind of fragmentation will a paged virtual memory system experience? For each segment that a process requires, how much of that kind of fragmentation will a paged virtual memory system exhibit, on average?

1/2 the page





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

# 2018 Fall Midterm

## Why is proper choice of a page replacement algorithm critical to the success of an operating system that uses virtual memory techniques? What is the likely difficulty if a poor choice of this algoritm is made by the OS designer?

We don't have enough RAM. A good page replacement algorithm has less page fault. If you make sure all the pages you have are pages you will use in the near future, then you don't have many page fault. Otherwise, you have many pages faults and it's slow. 



## Spin locks can cause performance problems if not used carefully. Why? In some cases, a thread using a spin lock can actually harm its own performance. Why?

Spin locks are just wasting CPU time, and thus cause performance problems. It harms its own performance because any other threads that don't have the lock can't make any progress. 



## Assume you are running on a virtual memory system that uses both segmentation and demand paging. When a process issues a request to access the memory word at address X, one possible outcome in terms of how the address is translated and the content of the address is made available is: the address is valid, the page is in a RAM page frame, and the MMU caches the page table entry for X, resulting in fast access to the word. Describe three other possible outcomes of the attempt to translate this address and the actions the system performs in those cases.

Address is invalid. Address is valid, we don't have the cache, the page is in RAM. Address is valid, we don't have it in cache, the page is not in RAM, we fetch the page from disk.



## When a Linux process executes a `fork()` call, a second process is created that's nearly identical. In what way is the new process different Why is that difference useful?

`pid` is different. Instruction pointers are the same. They have different address space although they share the same code segment. Data segment is also shared but it's copy on write. The stack is same but it's a copy.



## If your OS schedular's goal is to minimize avarage turnaround time, what kind of scheduling algorithm are you likely to run? Why?

STCF



## Assume you start with an operating system performing paged memory allocation with a page size of 4K. What will the effects of switching to a page size of 1K be on external and internal fragmentation? Describe one other non-fragmentation effect of this change and why it occurs.

There is external fragmentaion in segmentation. There is no external fragmentation in paging. Internal fragmentaion will decrease because average external fragmentation per process is half the size of page. The size of the page table increases. Number of pages in MMU is fixed. Let's say MMU can only have 100 entries. Originally it could store 400K, but now it can only contain 100K. More cache miss. Each time we bring a page from the disk, it's faster because it only has 1K instead of 4K. 



## An operating system can provide flow control on an IPC mechanism like sockets, but cannot provide flow control on an IPC mechanism like shared memory. Why?

The OS transfers the bytes for you. It knows reader is faster or writer is faster. The OS doesn't know what's happening in the shared memory becasue it all depends on user code. The OS has no control on which process should write first or read first. OS provides API for sockets. 



## Why are application binary interfaces of particular importance for successful software distribution?

ABI tells you which part of the executable represent code, data, etc. API specify how an application binary must interact with a particular OS running on a particular ISA. We need to craete different executables for different compliant system and we need ABI for that.



## Which memory management technique allows us to solve the problem of relocating memory partitions? 

Note: reloacation means translation from virtual to physical

Virtualization of memory address. The problem of relocating memory partitions refer to the idea that each process operates on its own address space. We need a memory relocation technique that maps from virtual address to physical address.



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













# Concurrency Bugs



## What's Atomicity-Violation Bug?

The idea of atomicity violation bug is that a region of code is assumed to be executed atomically.

```C
// Thread 1
if (thd->proc_info) {
  fputs(thd->proc_info, ...);
}

// Thread 2
thd->proc_info = NULL;
```

To fix this issue, we can simply wrap the critical section with lock.

```C
pthread_mutex_t proc_info_lock = PTHREAD_MUTEX_INITIALIZER;

// Thread 1
pthread_mutex_lock(&proc_info_lock);
if (thd->proc_info) {
  fputs(thd->proc_info, ...);
}
pthread_mutex_unlock(&proc_info_lock);

// Thread 2
pthread_mutex_lock(&proc_info_lock);
thd->proc_info = NULL;
pthread_mutex_unlock(&proc_info_lock);
```





## What's Order-Violation Bug?

The idea of order-violation bug is that there is an assumption is the order of execution.

```C
// Thread 1
void init() {
  mThread = PR_CreateThread(mMain, ...);
}

// Thread 2
void mMain() {
  mState = mThread->State;
}
```

To fix this issue, we can introduce conditional variables, and thus enforce the order of execution.

```C
pthread_mutex_t mtLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  mtCond = PTHREAD_COND_INITIALIZER;
int mtInit             = 0;

// Thread 1
void init() {
  mThread = PR_CreateThread(mMain, ...);
  
  pthread_mutex_lock(&mtLock);
  mtInit = 1;
  pthread_cond_signal(&mtCond);
  pthread_mutex_unlock(&mtLock);
}

// Thread 2
void mMain() {
  
  pthread_mutex_lock(&mtLock); // TODO: Why is this necessary
  while (mtInit == 0)
    pthread_cond_wait(&mtCond, &mtLock);
  pthread_mutex_unlock(&mtLock);
  
  mState = mThread->State;
}
```



## What are the four conditions needed for Deadlock to happen?

- Mutual Exclusion
- Hold-and-Wait
- No preemption
- Circular wait



## How to prevent circular wait?

Use total ordering or partial ordering to ensure the order of the locks accquired is consistent.



## How to prevent Hold-and-wait?

Accquire all the locks at once

```C
pthread_mutex_lock(prevention);   // begin lock acquisition
pthread_mutex_lock(L1);
pthread_mutex_lock(L2);
pthread_mutex_unlock(prevention); // end
```



## How to prevent No preemption?

Use `pthread_mutex_trylock()`

```C
top:
	pthread_mutex_lock(L1);
	if (pthread_mutex_trylock(L2) != 0) {
    pthread_mutex_unlock(L1);
    goto top;
  }
```



## How to avoid the need for mutual exclusion?

It is not always possible, but we can try atomic instruction such as `compare-and-swap` and see if there is a way around it.





## How to avoid deadlock without changing the code?

We can schedule the threads in such a way that deadlock can't possibly happen.

|      | T1   | T2   | T3   | T4   |
| ---- | ---- | ---- | ---- | ---- |
| L1   | yes  | yes  | no   | no   |
| L2   | yes  | yes  | yes  | jno  |

![Screen Shot 2019-05-05 at 11.00.36 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-05-05 at 11.00.36 AM.png)







# Deadlock Avoidance



# Health Monitoring





# Event-based Concurrency



## Why is event-based concurrency more powerful than traditional concurrency techniques?

Because there is no threads, there is no traditional concurrency bugs. There is only one process that handles events one at a time, and thus no locks are needed. The problem of event-based concurrency system is that there could be system call requests. When an event requires the process to make, for example, I/O requests, this may takes a while to process. While making the system call like `read()`, the process can no longer make progress (i.e. cannot handle other event until current event is completely handled, which may take a long time due to I/O requests). To resolve this issue, we can use **asnychronous I/O**. 





# Persistence

## What are the differences among memory bus, I/O bus, peripheral bus?



![Screen Shot 2019-05-12 at 9.55.20 AM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-05-12 at 9.55.20 AM.png)



## How can the OS check device status without frequent polling, and thus lower the CPU ovrhead required to manage the device?

We can use interrupt when the I/O is done. The OS can do something else while waiting for the task.

![Screen Shot 2019-05-12 at 1.17.54 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-05-12 at 1.17.54 PM.png)

Note taht using interrupts is not always the best solution. If the device performs its tasks very quickly, it's actually faster to just spin waiting. If the speed of the device is not known, or sometimes fast and sometimes slow, it may be best to use a hybrid that polls for a little while and then, if the device is not yet finished, uses interrupts (**two-phased approach**).



## What is Programmed I/O?

TODO



## What is Direct Memory Access? 

The idea is that with Programmed I/O, the CPU spends too much time moving data to and from devices by hand. DMA is essentially a very specific device within a system that can orchestrate transfers between devices and main memory without much CPU intervention.

![Screen Shot 2019-05-12 at 1.56.43 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-05-12 at 1.56.43 PM.png)

![Screen Shot 2019-05-12 at 1.57.24 PM](/Users/one/Desktop/CS 111/Notes/Screen Shot 2019-05-12 at 1.57.24 PM.png)





## How does the hardware communicate with devices?

It uses either **I/O instructions** or **memory mapped I/O** 



## What's the difference between write back and write through?

In write back, the drive acknowldges the write has completed when it has put the data in its memory. In write through, it acknowledges the write has completed after the write has actually been written to disk.



## How to compute the I/O time? What is it consisted of?

```
(I/O time) = (Seek Time) + (Rotational Time) + (Transfer Time)
```



## How can we implement Shortest Seek Time First (SSTF) scheduling but avoid starvation?

Disk starvation occurs when there is a steady stream of requests to the inner track, where the head is currently positioned. Requests to any other tracks would then be ignored. One way to solve this is to use **SCAN algorithm** (a.k.a. Elevator). With SCAN, the head sweeps the tracks inners to outers (or outers to inners) back and forth. But we can't combine this with SSTF. We can use Shortest Positioning Time First (SPTF).



## What is Redundant Arrays of Inexpensive Disks (RAIDs)?







## What is DMA?



## What is Memomy Mapping?





## What is defragmentation?

They recognize on-disk data to place files contiguously and make free space for one or a few contiguous regions, moving data around an then rewriting inodes and such to reflect the changes.



## What is crash-consistency problem?

Crash-consistency problem referes to the general problem that writing to the disk is interrupted due to power outage, wihch leaves the file system into a funny state. When a file is updated, three things needs to be updated in the file system— bitmap, inode, and data block. For example, if the power is cut off after only updating bitmap, then it will result in a space leak.



















