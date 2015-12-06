all: net

net:
	gcc -pthread -o client myclient.c
	gcc -pthread -o server myserver.c -lm

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
