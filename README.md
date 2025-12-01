COMP 2401 - Final Project: Ghost Hunting Simulation
Author: Ocean Yu
Student ID: 101348561
Date: November 30, 2025
YOU CAN ACCESS THIS PROJECT AT https://github.com/oceanyu1/Ghost-Game-Simulating-the-Hunts

Compilation/Execution:

To Compile:
  - Open a terminal in the project directory and run:
    $ make

  - This will compile all source files and put them all in an file called
    'simulation'

To Run:
    $ ./simulation

To Clean:
  - To remove all generated CSV log files, object files, and the executable:
    $ make clean

Testing Options (Makefile):
  - The Makefile currently has 2 config options for CFLAGS:
    
  - Standard Mode (Default/Also checks for memory leaks):
	CFLAGS = -Wall -Wextra -g -pthread
	Use this for running with Valgrind to check for memory leaks.
	Command: valgrind --leak-check=full ./simulation

  - Race Condition Check:
	To check for race conditions, uncomment the second CFLAGS line in the 
	Makefile and comment out the default standard mode:
	CFLAGS = -Wall -Wextra -g -pthread -fsanitize=thread
	Command: ./simulation

Design Notes:

- Git/Github: (YOU CAN ACCESS THIS PROJECT AT https://github.com/oceanyu1/Ghost-Game-Simulating-the-Hunts)
  - Note that I started tracking my progress for this assignment on Github when I was more than 
  	halfway done the project (didn't realize you can get marks for putting it online!), with defs.h,
  	hunter.c, ghost.c, and partial main.c already complete (and a lot of the code drafted/planned out),
  	so that's why the git commits only start on the 28th.

- Changing Device:
  - In the assignment specifications, it was stated that there should be a small chance every
	turn for the hunter to go back to the van and switch devices. This was implemented in 
	hunter.c (Line 149). The chance of them changing devices wasn't explicitly stated, so 
	I set it to a 10% chance.

- Deadlock Prevention:
  - To prevent deadlocks when an entity moves between rooms (requiring two locks),
    my implementation sorts the room pointers by memory address. It always 
    acquires the lock for the lower address first, then the higher address, forcing
    one hunter to wait while the other hunter moves.
    
    For example, if you have Kitchen at address 100 and Hallway at address 200, and
    Hunter 1 moving from Kitchen -> Hallway, and Hunter 2 going the other way
	(Hallway -> Kitchen), both hunters would start to try and lock Kitchen.
	
	For this example, let's say Hunter 1 locked the Kitchen first. Since Hunter 1 has the 
	Kitchen, Hunter 2 will wait for Hunter 1. Hunter 1 will now lock the Hallway, and 
	now Hunter 1 has both rooms locked. He can then safely move from Kitchen -> Hallway,
	and then unlock both rooms.
	
	Now Hunter 2 will wake up, lock the Kitchen, lock the Hallway, and move.
    
- Hunter Strategy:
  - Hunters use a "Breadcrumb" stack. Every time they enter a new room, they
    push it to the stack. When returning to the Van, they pop from the stack.

- Ghost Logic:
  - The Ghost stops running if the "main" thread sets its running flag to false 
    (when hunters win), or if its boredom counter exceeds the maximum.
  - A specific mutex was added to the Ghost structure to prevent race conditions
    when reading/writing the 'running' flag.

- Hunter IDs:
  - I know that in the project overview video, the prof made it so that you could manually type the IDs
  	of each hunter. I decided that the program creating its own IDs for each hunter is probably a lot
  	easier, so I did it that way as I didn't see any explicit instructions that state you need to
  	manually type the hunter ID.

Discussions:
  - Discussed the concept of "Deadlock Prevention by ordering locks", and also general design of defs.h with 
    Duc Tran Phan.
    
Assumptions:
  - I assumed that if a hunter goes into the same room as a ghost, even though they get scared, they still 
  	proceed to search for evidence with the ghost present. (very brave hunter)
  	
  - It's stated that the final output has to have a "ghost guess" at the end. I assume thats its less of a
  	"guess" and more of an explicit naming of the ghost that matches the evidence.

  - Less of an "assumption" and more of a design choice, but I also created a MAX_HUNTERS variable (defs.h)
  	so you can choose how many hunters you want as the max.
      
Sources:

- Course Lectures: 
  - Slides on Pthreads and Synchronization (Semaphores).
  - Slides on Linked Lists and Dynamic Memory.

- Other:
  - https://www.w3schools.com/c/ 								(Online resource on C)
  - https://eric-lo.gitbook.io/synchronization 					(More info on semaphores/race conditions)
  - https://www.geeksforgeeks.org/c/thread-functions-in-c-c/ 	(Info on pthread)
  - https://www.w3schools.com/c/ref_stdlib_srand.php 			(srand function)
  - https://heycoach.in/blog/deadlock-prevention-in-c-programs/ (Preventing Deadlocks)

   
