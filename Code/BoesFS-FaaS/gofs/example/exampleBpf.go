package main

import (
	"bufio"
	"errors"
	"log"
	"os"
	"strings"
	"time"

	"github.com/cilium/ebpf"
)

// clang -target bpf -Wall -O2 -emit-llvm -g -Iinclude -c xconnect.c -o - | llc -march=bpf -mcpu=probe -filetype=obj -o xconnect.o

func main() {
	// Allow the current process to lock memory for eBPF resources.
	// if err := rlimit.RemoveMemlock(); err != nil {
	// 	log.Fatal(err)
	// }
	spec, err := ebpf.LoadCollectionSpec("../test/sockex1_kern.o")
	if err != nil {
		panic(err)
	}

	var objs struct {
		XCProg *ebpf.Program `ebpf:"bpf_prog1"`
		XCMap  *ebpf.Map     `ebpf:"my_map"`
	}
	// Load pre-compiled programs and maps into the kernel.
	if err := spec.LoadAndAssign(&objs, nil); err != nil {
		panic(err)
	}
	defer objs.XCProg.Close()
	defer objs.XCMap.Close()
	println("load ok")

	// Get the first-mounted cgroupv2 path.
	// cgroupPath, err := detectCgroupPath()
	// if err != nil {
	// 	log.Fatal(err)
	// }

	// Link the count_egress_packets program to the cgroup.
	// l, err := link.AttachCgroup(link.CgroupOptions{
	// 	Path:    cgroupPath,
	// 	Attach:  ebpf.AttachCGroupInetEgress,
	// 	Program: objs.XCProg,
	// })
	// if err != nil {
	// 	log.Fatal(err)
	// }
	// defer l.Close()

	log.Println("Counting packets...")

	// Read loop reporting the total amount of times the kernel
	// function was entered, once per second.
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	for range ticker.C {
		var value uint64
		if err := objs.XCMap.Lookup(uint32(0), &value); err != nil {
			log.Fatalf("reading map: %v", err)
		}
		log.Printf("number of packets: %d\n", value)
	}
}

// detectCgroupPath returns the first-found mount point of type cgroup2
// and stores it in the cgroupPath global variable.
func detectCgroupPath() (string, error) {
	f, err := os.Open("/proc/mounts")
	if err != nil {
		return "", err
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		// example fields: cgroup2 /sys/fs/cgroup/unified cgroup2 rw,nosuid,nodev,noexec,relatime 0 0
		fields := strings.Split(scanner.Text(), " ")
		if len(fields) >= 3 && fields[2] == "cgroup2" {
			return fields[1], nil
		}
	}

	return "", errors.New("cgroup2 not mounted")
}
