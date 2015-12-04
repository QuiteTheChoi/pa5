net:
	gcc -g -o clie myclient.c
	gcc -g -o serv myserver.c

cav:
	git add -A
	git commit -m "Added stuff. - JCav"
	git push

get:
	git add -A
	git commit -m "Added stuff. - JGet"
	git push

clean:
	rm -rf *.o *.dSYM clie serv
