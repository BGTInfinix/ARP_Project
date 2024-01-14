# INC=-I headers
all:  master server drone keyboard window watchdog obstacle target 

# clean:
# 	rm build/*

master: 
	gcc master.c -o build/master

server: 
	gcc server.c -o build/server 

drone: 
	gcc drone.c -o build/drone 

obstacle: 
	gcc obstacle.c -o build/obstacle 

target: 
	gcc target.c -o build/target 

keyboard: 
	gcc keyboardManager.c -o build/keyboard 

window: 
	gcc window.c -o build/window -lncurses

watchdog: 
	gcc watchdog.c -o build/watchdog


# server: 
# 	gcc $(INC) server.c -o server -lncurses

