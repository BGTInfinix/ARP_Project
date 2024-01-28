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
- **Master:** Manages pipes for communication and at the same time it's working as a watchdog.
- **Server:** Prints information on Server console.
- **Window:** Displays the drone's movements, obstacles and targets. In the same time, you can see the location of the drone
- **Drone:** Simulates the drone's behavior and calcule the drone position depending on the obstacles.
- **Obstacle:** Simulates obstacles on the map.
- **Target:** Represents the target destination for the drone.
- **Keyboard:** Accepts keyboard input for controlling the drone.

## Features

- Multi-process system with independent components.
- Inter-process communication using pipes only.
- Simulated drone movement and obstacle avoidance.
- User interaction through keyboard input in keyboard window !!
- Watchdog process to monitor the health of other processes which is working in master.

## Prerequisites

- Linux-based operating system.
- GCC compiler.
- Konsole terminal emulator.

## Getting Started

1. Clone the repository:

   ```bash
   git clone https://github.com/NabBen/ARP.git
   cd Assignment2

2. Compile the project:
   ```bash
   make
   
3. Run the project: 
   ```bash
   ./build/master

## Terminating the System

- To exit the program, press 'Q' key when prompted
