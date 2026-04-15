# Mini Container Runtime with Kernel Monitor

---

# 1. Team Information

| Name      | SRN |
| --------- | --- |
| Your Name | SRN |
| Teammate  | SRN |

---

# 2. Build, Load, and Run Instructions

## Environment

Tested on:

* Ubuntu 22.04 / 24.04 VM

---

## Build

```bash
make
```

---

## Load Kernel Module

```bash
sudo insmod monitor.ko
```

---

## Verify Device

```bash
ls -l /dev/container_monitor
```

📸 **Screenshot Placeholder:**
`screenshots/device.png`
(Shows character device successfully created)

---

## Start Supervisor

```bash
sudo ./engine supervisor ./rootfs-base
```

📸 **Screenshot Placeholder:**
`screenshots/supervisor.png`
(Shows supervisor running)

---

## Prepare Root Filesystems

```bash
cp -a ./rootfs-base ./rootfs-alpha
cp -a ./rootfs-base ./rootfs-beta
mkdir -p ./rootfs-alpha/proc
mkdir -p ./rootfs-beta/proc
```

---

## Start Containers

```bash
sudo ./engine start alpha ./rootfs-alpha /bin/sh --soft-mib 48 --hard-mib 80
sudo ./engine start beta ./rootfs-beta /bin/sh --soft-mib 64 --hard-mib 96
```

📸 **Screenshot Placeholder:**
`screenshots/start.png`
(Shows containers starting)

---

## List Containers

```bash
sudo ./engine ps
```

📸 **Screenshot Placeholder:**
`screenshots/ps.png`
(Shows metadata tracking)

---

## View Logs

```bash
sudo ./engine logs alpha
```

📸 **Screenshot Placeholder:**
`screenshots/logs.png`
(Shows container logs)

---

## Stop Containers

```bash
sudo ./engine stop alpha
sudo ./engine stop beta
```

📸 **Screenshot Placeholder:**
`screenshots/stop.png`
(Shows containers being stopped)

---

## Inspect Kernel Logs

```bash
dmesg | tail
```

📸 **Screenshot Placeholder (MOST IMPORTANT):**
`screenshots/kill.png`
(Shows memory usage + container kill)

---

## Unload Module

```bash
sudo rmmod monitor
```

📸 **Screenshot Placeholder:**
`screenshots/unload.png`
(Shows module removal)

---

# 3. Demo with Screenshots

## 3.1 Multi-container Supervision

![Multi Container](screenshots/multi.png)
Two containers running under one supervisor.

---

## 3.2 Metadata Tracking

![PS Output](screenshots/ps.png)
Shows tracked container metadata.

---

## 3.3 Logging System

![Logs](screenshots/logs.png)
Logs captured via pipe-based system.

---

## 3.4 CLI and IPC

![CLI](screenshots/cli.png)
Command issued via CLI and processed.

---

## 3.5 Soft-limit Warning

![Soft Limit](screenshots/soft.png)
Kernel log showing memory warning.

---

## 3.6 Hard-limit Enforcement

![Kill](screenshots/kill.png)
Container killed after exceeding memory limit.

---

## 3.7 Scheduling Experiment

![Scheduling](screenshots/scheduling.png)
Different behavior with scheduling priorities.

---

## 3.8 Clean Teardown

![Cleanup](screenshots/cleanup.png)
No zombie processes after shutdown.

---

# 4. Engineering Analysis

### Namespace Isolation

UTS and mount namespaces isolate hostname and filesystem.

### Process Management

Containers created using `clone()` and tracked via host PIDs.

### Logging System

Pipes redirect stdout/stderr to log files.

### Kernel Monitoring

Kernel module tracks memory using `task->mm`.

### Scheduling

Linux scheduling behavior observed using `nice`.

---

# 5. Design Decisions and Tradeoffs

| Subsystem      | Decision               | Tradeoff           | Justification                        |
| -------------- | ---------------------- | ------------------ | ------------------------------------ |
| Namespaces     | Disabled PID namespace | Less isolation     | Required for correct kernel tracking |
| Logging        | Pipe-based             | Blocking initially | Simpler design                       |
| Kernel Monitor | Kernel thread          | Slight overhead    | Reliable execution                   |
| Metadata       | File-based             | Not persistent     | Easy implementation                  |

---

# 6. Scheduler Experiment Results

Example comparison:

| Nice Value | Behavior         |
| ---------- | ---------------- |
| -5         | Higher CPU usage |
| 10         | Lower priority   |

Conclusion:
Lower nice value → higher scheduling priority.

---
