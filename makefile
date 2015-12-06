all: net

net:
	gcc -g -o client myclient.c
	gcc -g -o server myserver.c

cav:
	git add -A
	git commit -m "Added stuff. - JCav"
	git push

get:
	git add -A
	git commit -m "Added stuff. - JGet"
	git push

clean:
	rm -rf *.o *.dSYM client server
