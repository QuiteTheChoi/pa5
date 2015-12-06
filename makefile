all: net

net:
	gcc -pthread -o client myclient.c
	gcc -lm -pthread -o server myserver.c

cav:
	git add -A
	git commit -m "Added stuff. - JCav"
	git push

get:
	git add -A
	git commit -m "Added stuff. - JGet"
	git push

clean:
	rm -rf *.o client server
