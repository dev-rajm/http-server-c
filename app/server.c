#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {

	/** 
	 * @brief Set the output and error buffer streams.
	 * @param stdout Standard output stream.
	 * @param stderr Standard error stream.
	*/
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	printf("Logs from your program will appear here!\n");

	/**
	 * server_fd Store the server file descriptor.
	 * client_addr_len Store the length of the client address.
	 * client_addr Holds the client address informations.
	 */
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	/**
	 * @brief Create a new socket.
	 * @param AF_INET Specifies the address family.
	 * @param SOCK_STREAM Specifies a stream socket.
	 * @param 0 Select the default protocol(TCP).
	 * 
	 * @return Socket file descriptor
	 */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	// Checking socket creation.
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	/**
	 * @brief Set options on the socket.
	 * @param server_fd Server file descriptor.
	 * @param SOL_SOCKET Specifies that the option is at the socket level.
	 * @param SO_REUSEABLE Allows the socket to reuse local addresses. This prevents "Address already in use" errors when restarting the server quickly.
	 * @param reuse Pointer to the reuse variable.
	 * @param sizeof(reuse) Size of the reuse variable.
	 * 
	 * @return Negative value if the setsockopt failed, positive otherwise
	 */
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	/**
	 * @brief Declare a structure sockaddr_in type variable, holds the server address informations
	 * .sin_family Address family (IPv4).
	 * .sin_port Set port number to 4221.
	 * .sin_addr  Sets the IP address. 
	 */
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	/**
	 * @brief Bind to the associate server address.
	 * 
	 * @return Non-zero value if binding failed, zero otherwise.
	 */
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	/**
	 * @brief Put the socket in listening mode, ready to accept incoming connections.
	 * @param connection_backlog Specifies the maximum number of pending connections the socket can handle.
	 * 
	 * @return Non-zero if listening failed, zero otherwise.
	 */
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	printf("Waiting for a client to connect...\n");

	client_addr_len = sizeof(client_addr); // Client address length
	
	/**
	 * @brief Accept incoming connections.
	 * 
	 * @return Client file descriptor for new client socket.
	 */
	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	
	// If failed to accept incoming connections.
	if(client_fd == -1) {
		printf("Accept failed: %s \n", strerror(errno));
		return 1;
	}
	printf("Client connected\n"); // Connected

	/**
	 * @brief Receive message in bytes with stored in readbuffer.
	 * @param cliend_fd Client file descriptor.
	 * @param readBuffer Points to a buffer where the message should be stored.
	 * @param path Specifies the length in bytes of the buffer pointed to by the buffer argument.
	 * @param 0 Specifies the type of message reception.
	 * 
	 * @return Negative value if recv failed.
	 */
	char readbuffer[1024];
	char path[512];
	char byteReceived = recv(client_fd, readbuffer, sizeof(readbuffer), 0);

	// Check if recv failed
	if(byteReceived == -1) {
		printf("Failed received byte message: %s \n", strerror(errno));
		return 1;
	}

	// Create temporary buffer and copy the readbuffer to it.
	char tempBuffer[strlen(readbuffer) + 1];
	strcpy(tempBuffer, readbuffer); 

	// Extract the path
	char *reqPath = strtok(readbuffer, " ");
	reqPath = strtok(NULL, " ");

	// Create a duplicate copy of path
	char *reqPathCopy = strdup(reqPath);

	// Extract the text from /path/abc
	char *mainPath = strtok(reqPathCopy, "/");
	char *content = strtok(NULL, " "); // abc

	// Extract User-Agent from headers
	char *userAgent = NULL;
	char *headerContent = strtok(tempBuffer, "\r\n"); // Tokenize the first line of tempBuffer
	while(headerContent != NULL) {
		if(strncmp(headerContent, "User-Agent: ", 12)==0) {
			userAgent = headerContent + 12; // Skip 'User-Agent: ', stored the remaining content
			break;
		}
		headerContent = strtok(NULL, "\r\n"); // Update to next line
	}
	

	// Stored the value of send function returns
	int bytesSent;

	/**
	 * Compare the request path with '/'
	 * Send the corresponding response.
	 */
	if(strcmp(reqPathCopy, "/") == 0) {
		char *res = "HTTP/1.1 200 OK\r\n\r\n";
		bytesSent = send(client_fd, res, strlen(res), 0);
	}

	/**
	 * Compare the request path with /echo
	 * Get the content length
	 * Store the response in the output buffer
	 * Send the response to the clinet
	 */
	else if(strcmp(reqPathCopy, "/echo") == 0) {
		int contentLength = strlen(content);
		char response[512];
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", 
			contentLength, content);
		printf("Sending Response: %s\n", response);
		bytesSent = send(client_fd, response, strlen(response), 0);
	}

	/**
	 * Compare the request path with /user-agent
	 * Get the length of User-Agent
	 * Send response to the client
	 */
	else if(strcmp(reqPathCopy, "/user-agent") == 0) {
		int headerContentLength = strlen(userAgent);
		char response[512];
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", 
			headerContentLength, userAgent);
		printf("Sending Response: %s\n", response);
		bytesSent = send(client_fd, response, strlen(response), 0);
	}

	// Response with 404 if any other path encountered
	else {
		char *res = "HTTP/1.1 404 Not Found\r\n\r\n";
		bytesSent = send(client_fd, res, strlen(res), 0);
	}

	// Check if response sents successfully
	if(bytesSent < 0) {
		printf("Send failed\n");
		return 1;
	}
	
	// Close the server
	close(server_fd);

	return 0;
}
