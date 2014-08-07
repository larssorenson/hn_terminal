all:
	gcc hn.c -Wall -o hn

debug:
	gcc hn.c -Wall -o hn -D DEBUG
