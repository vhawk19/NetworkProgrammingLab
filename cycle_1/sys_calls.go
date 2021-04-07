package main

import (
	"syscall"
	"fmt"
)

func main() {
	fmt.Printf("The current processs id is %d\n", syscall.Getpid())
	fmt.Printf("The parent processs id is %d\n", syscall.Getpid())
	fmt.Printf("The real user id is %d\n", syscall.Getpid())
	fmt.Printf("The group id is %d\n", syscall.Getpid())
	fmt.Printf("The effective user id is %d\n", syscall.Getpid())
	fmt.Printf("The effective group id is %d\n", syscall.Getpid())
}


