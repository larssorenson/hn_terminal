all:
	gcc hn.c -Wall -o hn

debug:
	gcc hn.c -Wall -g -o hn -D DEBUG
	
install: all
	sudo mv hn /bin/
	
clean:
	rm -rf *.o *~ hn
