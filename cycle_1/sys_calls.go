package main

import (
	"syscall"
	"fmt"
)

func main() {
	fmt.Printf("The current processs id is %d\n", syscall.Getpid())
	fmt.Printf("The parent processs id is %d\n", syscall.Getppid())
	fmt.Printf("The real user id is %d\n", syscall.Getuid())
	fmt.Printf("The group id is %d\n", syscall.Getgid())
	fmt.Printf("The effective user id is %d\n", syscall.Geteuid())
	fmt.Printf("The effective group id is %d\n", syscall.Getegid())
}


