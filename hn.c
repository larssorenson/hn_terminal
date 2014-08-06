#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define DEBUG 
#define ARTICLES	30

char* getRequest(char* host, char* path)
{
	char* request = malloc(sizeof(char)*15000);
	if(request == NULL)
	{
		printf("MALLOC FAILED!\r\n");
		return NULL;
	}

	struct hostent *server;
	struct sockaddr_in serveraddr;
	int port = 80;
	#ifdef DEBUG
		printf("Host: %s\r\n", host);
		 
		printf("Path: %s\r\n", path);
	#endif
	 
	int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	 
	if (tcpSocket < 0)
	{
		printf("Error opening socket\r\n");
		return NULL;
	}
	#ifdef DEBUG
	else
		printf("Successfully opened socket\r\n");
	#endif
	 
	server = gethostbyname(host);
	 
	if (server == NULL)
	{
		printf("gethostbyname() failed\r\n");
	}
 
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	 
	bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	 
	serveraddr.sin_port = htons(port);
	 
	if (connect(tcpSocket, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
		printf("Error Connecting\r\n");
	#ifdef DEBUG
	else
		printf("Successfully Connected\r\n");
	#endif
	   
	bzero(request, 15000);
	 
	sprintf(request, "Get %s HTTP/1.1\r\nHost:%s\r\n\r\n", path, host);
	 
	#ifdef DEBUG
		printf("\n%s", request);
	#endif
	 
	if (send(tcpSocket, request, strlen(request), 0) < 0)
			printf("Error with send()\r\n");
	#ifdef DEBUG
		else
			printf("Successfully sent html fetch request\r\n");
	#endif
	 
	bzero(request, 15000);
	int total = 0;
	int read = 1000;
	while(read == 1000 && total < 15000)
	{
		read = recv(tcpSocket, request, 1000, 0);
		total += read;
		request += read;
		sleep(1);
	}
	
	request -= total;
	#ifdef DEBUG
		printf("\n%s", request);
	#endif
	 
	close(tcpSocket);
	return request;
	 
}

void main(int argc, char** argv)
{
	char* json = getRequest("api.ihackernews.com", "/page");
	if(json != NULL)
		printf("%s\r\n", json);
	else
	{
		printf("Failed to get articles!\r\n");
		return;
	}
	#ifdef DEBUG
		printf("Length of full response: %d\r\n", (int)strlen(json));
	#endif
	int headerLen = 0;
	while(*json != '{')
	{
		json++;
		headerLen++;
	}
	int len = strlen(json);
	#ifdef DEBUG
		printf("Response length: %d\r\n", len);
	#endif

	char** articles = malloc(sizeof(char*)*ARTICLES); // The front page shows 30 articles by default
	char** urls = malloc(sizeof(char*)*ARTICLES); // Array of urls for articles
	int total = 0;
	int articleLen = 0;
	int urlLen = 0;
	int currArticle = 0;
	while(*json != '\0' && currArticle < ARTICLES)
	{
		if(strstr(json, "title") == json)
		{
				   // We're here
				   //    v
			json += 8; // [{"title":"...
				   //             ^
				   //   We move here
			total += 8;

			// Now we measure how long the article title is
			// So that we can malloc the right amount of space
			articleLen = strstr(json, "\"") - json;
			/*while(*json != '"')
			{
				articleLen++;
				json++;
			}

			json -= articleLen; // Move the pointer back*/
			articles[currArticle] = malloc(sizeof(char)*(articleLen+1)); // Malloc space for the article
			strncpy(articles[currArticle], json, articleLen); // Copy the article title into the buffer
			articles[currArticle][articleLen] = '\0'; // Null terminate
			#ifdef DEBUG
				printf("Found article: %s\r\n", articles[currArticle]);
			#endif
			json += articleLen + 9; //  V Move the pointer beyond the title, plus the 9 characters between it and the URL
						// "...","url":"...
						//              ^
			total += articleLen+9;
			articleLen = 0;

			urlLen = strstr(json, "\"")-json;
			/*while(*json != '"')
			{
				urlLen++; // same concept as before
				json++;
			}
			
			json -= urlLen;*/
			urls[currArticle] = malloc(sizeof(char)*(urlLen+1));
			strncpy(urls[currArticle], json, urlLen);
			urls[currArticle][urlLen] = '\0';
			#ifdef DEBUG
				printf("Found URL: %s\r\n\r\n", urls[currArticle]);
			#endif
			json += urlLen;
			total += urlLen;
			urlLen = 0;
			
			currArticle++;
		}

		json++;
		total++;

	}

	json -= total;
	#ifdef DEBUG
		printf("Read %d bytes\r\n", total);
		if(total != len)
		{
			printf("Uh oh! didn't read the same amount of bytes as received!\r\n");
		}
	#endif
	json -= headerLen; // Return to original pointer

	free(json);

}
