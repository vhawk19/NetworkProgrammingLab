# Study of system calls - Part 1

* [Aim](#Aim)
* [Theory](#Theory)
* [Program](#Program)
* [Execution Steps](#Execution)
* [Output](#Output)
* [Result](#Result)

## Aim
To obtain the process id, parent process id, real user id, real group id, effective user id and effective group id. 

## Theory
### 1.getpid()
Header File: unistd.h\
Syntax: pid_t getpid(void);\
Description: Returns the process id of the current calling process. It does not throw any error and is always successful. 
### 2.getppid()
Header File: unistd.h\
Syntax: pid_t getppid(void)\
Description: Returns the process ID of the parent of the calling process. If the calling process was created by the fork() function and the parent process still exists at the time of the getppid() function call, this function returns the process ID of the parent process. Otherwise, this function returns a value of 1 which is the process id for init process.

### 3.getuid()
Header File: unistd.h\
Syntax:  uid_t getuid(void);\
Description: Returns the real user id of the calling process. For a process, real user id is simply the user id of the user that started the process.
### 4.getgid()
Header File: unistd.h\
Syntax: gid_t getgid(void);\
Description: Returns the real group id of the calling process. 
### 5.geteuid()
Header File: unistd.h\
Syntax: uid_t geteuid(void); \
Description: Returns the effective user id of the calling process. For a process, effective user id is normally the same as Real UserID, but sometimes it is changed to enable a non-privileged user to access files that can only be accessed by a privileged user like root.
        

### 6.getegid()
Header File: unistd.h\
Syntax: pid_t getegid(void);\
Description: Returns the effective group id of the calling process. 


## Program
![](https://i.imgur.com/WpjEdT5.png)


## Execution
1. Start
2. Output values returned by the given syscalls getpid(), getppid(), getuid(), getgid(), geteuid, getegid()
3. Run the program
4. Terminate
        
## Output
![](https://i.imgur.com/Q89fnLh.png)

## Result
The program has been executed successfully. Thus the process id, the parent process id, real user id, effective user id, real group id and effective group id have been obtained successfully. 
