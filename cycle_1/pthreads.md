# Study of POSIX Threads - Part 1

- [Aim](#Aim)
- [Theory](#Theory)
- [Program](#Program)
- [Execution Steps](#Execution)
- [Output](#Output)
- [Result](#Result)

## Aim

Study the basic posix thread functions – pthread_create, pthread_join, pthread_self, pthread_detach, pthread_exit

## Theory

### 1.pthread_create()

Header File: pthread.h\
Syntax: `int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg);`
Description: The pthread_create() function starts a new thread in the calling process.The new thread starts execution by invoking start_routine(); arg is passed as the sole argument of start_routine()

### 2.pthread_join()

Header File: pthread.h\
Syntax: ` int pthread_join(pthread_t thread, void **retval);`\
Description: The pthread_join() function waits for the thread specified by thread to terminate. If that thread has already terminated, then pthread_join() returns immediately.The thread specified by thread must be joinable.

### 3.pthread_self()

Header File: pthread.h\

Syntax: ` pthread_t pthread_self(void);`\
Description: The pthread_self() function returns the ID of the calling thread.This is the same value that is returned in thread in the pthread_create() call that created this thread.

### 4.pthread_detach()

Header File: pthread.h\
Syntax: ` int pthread_detach(pthread_t thread);`\
Description: The pthread_detach() function shall indicate to the implementation that storage for the thread thread can be reclaimed when that thread terminates. If thread has not terminated, pthread_detach() shall not cause it to terminate.

### 5.pthread_exit()

Header File: pthread.h\
Syntax: ` noreturn void pthread_exit(void *retval);`\
Description: The pthread_exit() function terminates the calling thread and returns a value via retval that (if the thread is joinable) is available to another thread in the same process that calls pthread_join(3)

## Program

![](https://i.imgur.com/1R9ORdJ.png)\
![Link to program](https://github.com/vhawk19/NetworkProgrammingLab/blob/main/cycle_1/pthreads.c)

## Execution

1. Start
2. pthread calls:- pthread_create,pthread_join,pthread_self,pthread_detach,pthread_exit are made during the execution of the program
3. Run the program
4. Terminate

## Output

![](https://i.imgur.com/lvCGlRp.jpg)

## Result

The program has been executed successfully.The pthread calls have been made successfully through the runtime of the program.
