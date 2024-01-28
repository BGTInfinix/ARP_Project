# Assignment 2 Nabil Bentefrit

This project is a multi-process system designed to simulate a drone control system. It consists of several independent processes, each representing a different aspect of the drone control system. The processes communicate with each other using inter-process communication (IPC) mechanisms like pipes.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [Terminating the System](#terminating-the-system)

## Introduction

The project comprises the following processes:
- **Server:** Manages shared memory for communication.
- **Window:** Displays information using shared memory.
- **Drone:** Simulates the drone's behavior.
- **Obstacle:** Simulates obstacles in the drone's path.
- **Target:** Represents the target destination for the drone.
- **Keyboard:** Accepts keyboard input for controlling the drone.

## Features

- Multi-process system with independent components.
- Inter-process communication using pipes and shared memory.
- Simulated drone movement and obstacle avoidance.
- User interaction through keyboard input.
- Watchdog process to monitor the health of other processes.

## Prerequisites

- Linux-based operating system.
- GCC compiler.
- Konsole terminal emulator.

## Getting Started

1. Clone the repository:

   ```bash
   git clone https://github.com/your/repository.git
   cd repository

2. Compile the project:
   ```bash
   make
   
3. Run the project: 
   ```bash
   ./build/master

## Terminated the System

- To exit the program, press 'Q' key when prompted
