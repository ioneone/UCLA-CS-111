# Week 1

man page sections



1: shell commands

2: system calles

3: C library functions

http://man7.org/linux/man-pages/man2/open.2.html

```c
open(pathname, flags, mode);
```

mode: permission

0644: 0 means octal. 644 means read/write for user, read for group and other.`

```c
creat(pathname, mode)
```



http://man7.org/linux/man-pages/man2/read.2.html

```c
read(fd, *buf, count)
// count: number of bytes you want to read
```

return value: 0 indicates end of file.

return numbe rof bytes read is returned.

EOF is state of file descriptor.

**It is not an error if this number is smaller than number of bytes requtested. Maybe read() was interrpupted by a signal.**

Need two loops? Keep writing until everything is written because write() might be interruppted?

http://man7.org/linux/man-pages/man2/dup.2.html

```
dup()
dup2()
```

It's better to use dup2() because in multithreading programs, it messes things up.



```
exit(2)
Exit(3)
```

http://man7.org/linux/man-pages/man2/exit.2.html



# Week 2



**Zombie process**

On [Unix](https://en.wikipedia.org/wiki/Unix) and [Unix-like](https://en.wikipedia.org/wiki/Unix-like) computer [operating systems](https://en.wikipedia.org/wiki/Operating_system), a **zombie process** or **defunct process** is a [process](https://en.wikipedia.org/wiki/Process_(computing)) that has completed execution (via the `exit` [system call](https://en.wikipedia.org/wiki/System_call)) but still has an entry in the [process table](https://en.wikipedia.org/wiki/Process_table): it is a process in the "[Terminated state](https://en.wikipedia.org/wiki/Process_state#Terminated)". This occurs for [child processes](https://en.wikipedia.org/wiki/Child_process), where the entry is still needed to allow the [parent process](https://en.wikipedia.org/wiki/Parent_process)to read its child's [exit status](https://en.wikipedia.org/wiki/Exit_status): once the exit status is read via the  `wait` [system call](https://en.wikipedia.org/wiki/System_call), the zombie's entry is removed from the process table and it is said to be "reaped"



**Why do you need to close unnecessary end of pipe?**



