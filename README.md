# ARP_Project
Advanced Programming and Robotic Project

# Assignment 1
Team Member: Nabil Bentefrit

## master.c

The `master.c` file is a component of the program responsible for forking the other processes. It executes all the actives components of the program. 

## server.c

The main role `server.c` file is to create and update the shared memory made for the drone's position, as well as the creation of a semaphore that will allow to have synchronized access to this shared memory. This shared memory is only accessible between server.c and drone.c.

## keyboardManager.c

The `keyboardManager.c` file is made to manage the user input from the keyboard and communicate the corresponding commands to the `drone.c` file to control the drone's movement via pipe. The input has to be done from keyboardmanager window and not from the blackboard ! 

## drone.c

The `drone.c` takes the decision and apply a force on the drone depending on the input in keyboardmanager. Then it send the new position to window.c with pipes and the log to server.c with shared memory. 


## window.c

The `window.c` creates the graphical interface to the game and refresh it everytime a new character has been input on keyboardmanager.c


## Instruction for installing and running
These instructions will help you get a copy of the project up and running on your local machine.


### Build and Execute the Project
In the 'Assignment1' directory, hit:

```bash
$ make
```
to compile all the necessary files. And to run the project, enter:

```bash
$ ./build/master
```

IN CASE !! 
press Quit to quit the programm
