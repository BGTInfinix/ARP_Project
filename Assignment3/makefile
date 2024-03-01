# INC=-I headers
all:  master server drone keyboard window watchdog obstacle target 

clean:
	rm -rf build
	mkdir build

master: 
	gcc -o build/master src/master.c src/logfile.c -I include

server: 
	gcc -o build/server src/server.c src/socket.c src/logfile.c -I include

drone: 
	gcc -o build/drone src/drone.c src/logfile.c -lm -I include

obstacle: 
	gcc -o build/obstacle src/obstacle.c src/socket.c -I include

target: 
	gcc -o build/target src/target.c src/socket.c -I include

keyboard: 
	gcc -o build/keyboard src/keyboardManager.c -I include

window: 
	gcc -o build/window src/window.c src/logfile.c -lncurses -I include

watchdog: 
	gcc -o build/watchdog src/watchdog.c -I include


# server: 
# 	gcc $(INC) server.c -o server -lncurses

