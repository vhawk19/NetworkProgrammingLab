#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

int main()
{
    /* the size (in bytes) of shared memory object */
    const int SIZE = sizeof(int);

    /* name of the shared memory object */
    const char *name = "counter";

    /* shared memory file descriptor */
    int shm_fd;

    /* pointer to shared memory object */
    int *ptr;

    /* open the shared memory object */
    shm_fd = shm_open(name, O_RDWR, 0666);

    /* memory map the shared memory object */
    ptr = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    /* create a semaphore for mutual exclusion */
    sem_t *semptr = sem_open("semaphO", O_CREAT, 0666, 1);

    pid_t pid;
    pid = fork();
    if (pid > 0)
    {
        printf("Child Process:\n");
        if (!sem_wait(semptr))
        { /* wait until semaphore != 0 */
            int i;
            i = *ptr;
            printf("Counter value: %d\n", i);
            i += 1;
            *ptr = i;
            sem_post(semptr);
        }
    }
    else if (pid == 0)
    {
        printf("Parent Process\n");
        if (!sem_wait(semptr))
        { /* wait until semaphore != 0 */
            int i;
            i = *ptr;
            printf("Counter value: %d\n", i);
            i += 1;
            *ptr = i;
            sem_post(semptr);
        }
    }
    else
    {
        perror("Fork Failure\n");
    }

    /* read from the shared memory object */
    printf("Value of counter after updation: %d\n", *ptr);

    /* remove the shared memory object */
    munmap(ptr, SIZE);
    close(shm_fd);
    sem_close(semptr);
    shm_unlink(name);
    return 0;
}
