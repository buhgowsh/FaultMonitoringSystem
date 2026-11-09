# FaultMonitoringSystem
A lightweight Linux application that mointors running processes, checks their resource usage against administrator-defined rules, and taskes automatic action when a rule is violated.

## Overview
Many processes run at once on a Linux system, and some can consume excessive CPU or memory, slowing down the system or interfering with other applications. The Fault Monitoring System automates the detection of these conditions. Administrators define thresholds for individual processes, and the system responds according to the rules they set, reducing the need for constant manual supervision.


## Table of Contents
- [Features](#features)
- [How It Works](#how-it-works)
- [Tech Stack](#tech-stack)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Rules](#rules)
- [Logs](#logs)
- [Team](#team)


## Features
- Per-process thresholds for CPU utilization, physical memory, and virtual memory usage
- Configurable actions when a threshold is exceeded: halt the process, log the event, or increase or decrease its priority
- Process discovery to list running processes and select which ones to monitor
- Rule management to create and modify rules through the user interface
- Event logging that records detected faults and the actions taken
- Separate log viewer for browsing the fault history without interacting with the kernel directly


## How It Works
[diagram]


## Tech Stack
| Layer | Technology |
|---|---|
| Backend | C |
| Frontend | C++, Qt |
| Rule parser | Flex, Bison |
| Data source | Linux `/proc` filesystem |
| Presistent storage | Plain `.txt` files |


## Project Structure
```
.
├── backend/
│   ├── main.c          # 
│   ├── read_proc.c     # 
│   ├── rules.c         # 
│   ├── log.c           # 
│   └── ...             # 
├── gui/
│   ├──                 # 
│   └── ...             # 
├── ...                 
│   ├── ...             # 
│   └── ...             # 
└── README.md
```


## Getting Started
### Prerequisites
- Linux distro
- GCC
- Flex and Bison
- Qt
- `make` or CMake

### Install
```bash
```

### Build
```bash
```

### Run
```bash
```


## Usage


## Rules


## Logs


## Team
|Name|Role|
|----|----|
|Maizie|
|Josh|
|Noah|
|Ivan|
|Matt|
|Carlos|
|Ryan|

**Sponsor:** UCF Senior Design
