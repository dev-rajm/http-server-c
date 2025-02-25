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
	 * @brief This function sets the output and error buffer stream
	 * @param stdout Standard output stream.
	 * @param stderr Standard error stream.
	*/
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	printf("Logs from your program will appear here!\n");

	/**
	 * @var server_fd 	Store the file descriptor of the server's socket.
	 * @var client_addr_len  Store the length of the client address.
	 * @var client_addr	 Holds the client's IP address and the port number.
	 */
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	/**
	 * @fn socket Create a new socket.
	 * @param AF_INET Specifies the IPv4 address family.
	 * @param SOCK_STREAM Specifies a TCP socket (stream socket).
	 * @paragraph 0 Select the default protocol.
	 * 
	 * @return Socket file descriptor
	 */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	/**
	 * Checking if socket returns -1
	 * Socket failed to create a new socket then the file descriptor should be a negative value
	 * Print the error with `strerror` to display a human readable error message
	 * Program exit with code 1
	 */
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	/**
	 * @brief Set options on a socket, allowing the programmer to configure various properties of the socket.
	 * @param server_fd Server's file descriptor.
	 * @param SOL_SOCKET Specifies that the option is at the socket level.
	 * @param SO_REUSEABLE Allows the socket to reuse local addresses. This prevents "Address already in use" errors when restarting the server quickly.
	 * @param reuse Pinter to the reuse variable
	 * @param sizeof(reuse) Size of the reuse variable
	 * 
	 * @return Negative value if the setsockopt failed, positive otherwise
	 */
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	/**
	 * @brief Declare a sockaddr_in type variable serv_addr, holds the server's address informations
	 * @var .sin_family Address family (IPv4)
	 * @var .sin_port Set port number to 4221.
	 * @var sin_addr  Sets the IP address. 
	 */
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	/**
	 * @brief Bind to the associate server address
	 * 
	 * @return Non-zero value if binding failed, zero otherwise
	 */
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	/**
	 * @var connection_backlog Specifies the maximum number of pending connections the socket can handle.
	 * @fn listen() Put the socket in listening mode, ready to accept incoming connections.
	 * 
	 * @return Non-zero if listening failed, zero otherwise
	 */
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	// Client address length
	client_addr_len = sizeof(client_addr);
	
	/**
	 * @brief Accept incoming connections
	 * 
	 * @return Client file descriptor for new client socket
	 */
	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	
	//  Check if failed to accept incoming connections
	if(client_fd == -1) {
		printf("Accept failed: %s \n", strerror(errno));
		return 1;
	}
	printf("Client connected\n");

	/**
	 * @param cliend_fd Client file descriptor.
	 * @param readBuffer Points to a buffer where the message should be stored.
	 * @param path Specifies the length in bytes of the buffer pointed to by the buffer argument.
	 * @param flag Specifies the type of message reception.
	 * 
	 * @return The message in bytes.
	 */
	char readbuffer[1024];
	char path[512];
	char byteReceived = recv(client_fd, readbuffer, sizeof(readbuffer), 0);

	if(byteReceived == -1) {
		printf("Failed received byte message: %s \n", strerror(errno));
		return 1;
	}

	// Extract the path using space as a delimiter
	char *reqPath = strtok(readbuffer, " ");
	reqPath = strtok(NULL, " ");

	// Stored the value of send function returns
	int bytesSent;

	/**
	 * Compare the request path with the root path
	 * Send the corresponding responses, if path is `/` send res200
	 * else res404 for other paths
	 */
	if(strcmp(reqPath, "/") != 0) {
		char *response = "HTTP/1.1 404 Not Found\r\n\r\n"; // 404
		bytesSent = send(client_fd, response, strlen(response), 0);
	}
	else {
		char *response = "HTTP/1.1 200 OK\r\n\r\n"; // 200
		bytesSent = send(client_fd, response, strlen(response), 0);
	}

	/**
	 * Check if response sents successfully
	 * If bytesSent is negative then exit the program with code 1
	 */
	if(bytesSent < 0) {
		printf("Send failed\n");
		return 1;
	}
	
	// Close the server
	close(server_fd);

	return 0;
}
