# Mini Container Runtime with Kernel Memory Monitor

## Overview

This project implements a lightweight container runtime in user space along with a Linux kernel module for memory monitoring and enforcement.

The system supports:

* Creating isolated containers using Linux namespaces
* Running multiple containers concurrently
* Capturing container logs
* Tracking container metadata
* Enforcing memory limits via a kernel module

---

## Features

### 1. Container Runtime

* Uses `clone()` with namespaces:

  * UTS (hostname isolation)
  * Mount namespace
* Supports multiple containers running simultaneously
* Containers run in background

### 2. Logging System

* Pipes used to capture stdout/stderr from containers
* Logs stored per container in:

  ```
  boilerplate/logs/<container_id>.log
  ```

### 3. Container Management

Commands implemented:

```
./engine start <id> <rootfs> <command>
./engine ps
./engine stop <id>
```

* `start` → launches container
* `ps` → lists running containers
* `stop` → kills container

---

### 4. Kernel Module (monitor.ko)

* Character device: `/dev/container_monitor`
* Uses `ioctl` for communication
* Tracks container PIDs
* Periodically checks memory usage
* Kills containers exceeding limit

---

## Architecture

User Space:

* `engine.c` → container runtime + CLI
* Pipes → logging
* File (`containers.txt`) → metadata tracking

Kernel Space:

* `monitor.c`
* Tracks processes using linked list
* Kernel thread periodically checks memory
* Sends `SIGKILL` when limit exceeded

---

## Memory Enforcement

* Memory usage obtained via:

  ```c
  task->mm->total_vm << PAGE_SHIFT
  ```

* Default limits:

  * Soft limit: 1 MB
  * Hard limit: 2 MB

* When exceeded:

  ```
  Monitor: Killing PID XXXX
  ```

---

## How to Run

### 1. Setup root filesystem

```
mkdir rootfs-base
tar -xzf alpine-minirootfs.tar.gz -C rootfs-base

cp -a rootfs-base rootfs-alpha
mkdir -p rootfs-alpha/proc
```

---

### 2. Build runtime

```
cd boilerplate
make
```

---

### 3. Build and load kernel module

```
make
sudo insmod monitor.ko
```

Create device:

```
cat /proc/devices | grep container_monitor
sudo mknod /dev/container_monitor c <MAJOR> 0
sudo chmod 666 /dev/container_monitor
```

---

### 4. Run container

```
sudo ./engine start alpha ../rootfs-alpha /bin/sh
```

---

### 5. View logs

```
cat logs/alpha.log
```

---

### 6. Check kernel output

```
dmesg | tail
```

---

## Example Output

```
Monitor: Registered PID 7626
Monitor: PID 7626 mem=1650688
Monitor: Killing PID 7626
```

---

## Screenshots (included)

1. Container running (`ps`)
2. Multiple containers
3. Logging output
4. Kernel module loaded
5. Memory enforcement (kill)

---
