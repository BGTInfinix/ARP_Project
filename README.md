# Assignment 3 Nabil Bentefrit

This project is a multi-process system designed to simulate a drone control system. It consists of several independent processes, each representing a different aspect of the drone control system. The processes communicate with each other using inter-process communication (IPC) mechanisms like pipes. Moreover, Drone & Obstacle are connected by sockets with Server that deal with the informations and send them to the correct process via pipes

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [Terminating the System](#terminating-the-system)

## Introduction

The project comprises the following processes:
- **Master:** Manages pipes for communication and at the same time it's working as a watchdog.
- **Server:** Server management information on Server console.
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
   cd Assignment3

2. Compile the project:
   ```bash
   make
   
3. Run the project: 
   ```bash
   ./build/master server hostname port
   ./build/master client port

## Commands
In the 'keyboard' konsole, use the keys:  

`w` `e` `r` &nbsp; &nbsp;    `7` `8` `9`     

`s` `d` `f`  or              `4` `5` `6`     

`x` `c` `v` &nbsp; &nbsp;    `1` `2` `3`  

`p` : to reset game after end of the game
` ` : to send "stop" to every process

## Terminating the System

- To exit the program, press 'Q' key when prompted
