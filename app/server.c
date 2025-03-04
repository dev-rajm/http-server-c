#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

// Handle client actions
void *handle_client(void *arg) {
	int client_fd = *(int *)arg;
	free(arg);

	char readbuffer[1024];
	char path[512];
	ssize_t byteReceived = recv(client_fd, readbuffer, sizeof(readbuffer), 0);

	if(byteReceived == -1) {
		printf("Failed received byte message: %s \n", strerror(errno));
		close(client_fd);
		pthread_exit(NULL);
	}

	readbuffer[byteReceived] = '\0';

	char tempBuffer[strlen(readbuffer) + 1];
	strcpy(tempBuffer, readbuffer);

	// Extract request path from client_fd
	char *reqPath = strtok(readbuffer, " ");
	reqPath = strtok(NULL, " ");

	char *reqPathCopy = strdup(reqPath);

	// Extract the text from /path/abc
	char *mainPath = strtok(reqPathCopy, "/");
	char *content = strtok(NULL, " ");

	// Extract header
	char *userAgent = NULL;
	char *headerContent = strtok(tempBuffer, "\r\n");
	while(headerContent != NULL) {
		if(strncmp(headerContent, "User-Agent: ", 12)==0) {
			userAgent = headerContent + 12; // Skip 'User-Agent: ', stored the remaining content
			break;
		}
		headerContent = strtok(NULL, "\r\n");
	}

	ssize_t bytesSent;

	// Compare the request path with '/'
	if(strcmp(reqPathCopy, "/") == 0) {
		char *res = "HTTP/1.1 200 OK\r\n\r\n";
		bytesSent = send(client_fd, res, strlen(res), 0);
	}

	// Compare the request path with /echo
	else if(strcmp(reqPathCopy, "/echo") == 0) {
		int contentLength = strlen(content);
		char response[512];
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", 
			contentLength, content);
		printf("Sending Response: %s\n", response);
		bytesSent = send(client_fd, response, strlen(response), 0);
	}

	// Compare the request path with /user-agent
	else if(strcmp(reqPathCopy, "/user-agent") == 0) {
		int userAgentLength = strlen(userAgent);
		char response[512];
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", 
			userAgentLength, userAgent);
		printf("Sending Response: %s\n", response);
		bytesSent = send(client_fd, response, strlen(response), 0);
	}

	// Compare the request path with /files
	else if(strcmp(reqPathCopy, "/files") == 0) {
		printf("Filename: %s", content);
		char *fileContent;
		FILE *fptr;
		char *dir = "./tmp/";
		char *path = strcat(dir, content);
		printf("File full path: %s", path);
		fptr = fopen(path, "r");

		if(fptr==NULL) {
			char *res = "HTTP/1.1 404 Not Found\r\n\r\n";
			bytesSent = send(client_fd, res, strlen(res), 0);
		}
		fgets(fileContent, 100, fptr);
		size_t fileContentLength = strlen(fileContent);
		char response[512];
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %ld\r\n\r\n%s", 
		fileContentLength, fileContent);
		printf("Sending Response: %s\n", response);
		bytesSent = send(client_fd, response, strlen(response), 0);

		fclose(fptr);
	}

	// Response 404 for other endpoints
	else {
		char *res = "HTTP/1.1 404 Not Found\r\n\r\n";
		bytesSent = send(client_fd, res, strlen(res), 0);
	}

	// Check if response sents successfully
	if(bytesSent < 0) {
		printf("Send failed.\n");
	}

	close(client_fd);
	free(reqPathCopy);
	pthread_exit(NULL);

}

int main() {
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	printf("Logs from your program will appear here!\n");

	/**
	 * server_fd: Store the server file descriptor.
	 * client_addr_len: Store the length of the client address.
	 * client_addr: Holds the client address informations.
	 */
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	// Create a new server socket.
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	// Checking socket creation.
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Set options on the socket.
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	// Bind to the associate server address.
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	// Put the socket in listening mode, ready to accept incoming connections.
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	printf("Waiting for a client to connect...\n");

	client_addr_len = sizeof(client_addr); // Client address length

	// Accept incoming connections.
	while(1) {
		int *client_fd_ptr = malloc(sizeof(int));
		*client_fd_ptr = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		
		if(*client_fd_ptr == -1) {
			printf("Accept failed: %s \n", strerror(errno));
			free(client_fd_ptr);
			continue;
		}
		printf("Client connected.\n");

		pthread_t thread_id;
		if(pthread_create(&thread_id, NULL, handle_client, client_fd_ptr) != 0) {
			printf("Failed to create thread.\n");
			close(*client_fd_ptr);
			free(client_fd_ptr);
		}
		pthread_detach(thread_id);
	}

	// Close the server
	close(server_fd);
	return 0;
}
