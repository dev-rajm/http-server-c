# Simple HTTP Server in C

This repository contains a basic HTTP server implementation written in C. It demonstrates server-side programming concepts, including socket programming, request parsing, and response generation.

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-strated)
  - [Prerequisites](#prerequisites)
  - [Compilation](#compilation)
  - [Running the Server](#running-the-server)
- [Implementation Details](#implementation-details)
  - [Socket Programming](#socket-programming)
  - [Request Parsing](#request-parsing)
  - [Response Generation](#response-generation)
  - [Error Handling](#error-handling)
- [Code Explanation](#code-explanation)
- [Future Enhancements](#future-enhancements)
- [Contributing](#contributing)
- [License](#license)

## Features

- Handles basic HTTP GET requests.
- Parses HTTP headers (e.g., User-Agent).
- Sends HTTP responses with status codes and content.
- Supports `/`, `/echo` and `/user-agent` endpoints.
- Simple socket-based server using `socket`, `bind`, `listen`, `accept`.
- Demonstrates basic error handling.
- SO_REUSEADDR implemented to prevent "Address already in use" errors.
- Multi-threading or asynchronous I/O for handling multiple clients concurrently.

## Getting Started

### Prerequisites

- A C compiler (e.g., GCC).
- A Unix-like OS (Linux, Mac) or WSL (Windows Subsystem for Linux).

### Compilation

1. Clone the repository:

   ```bash
   git clone https://github.com/dev-rajm/http-server-c.git
   cd http-server-c
   ```

2. Compile the source code:
   ```bash
   gcc /app/server.c
   ```

### Running the Server

1. Execute the compiled binary:

   ```bash
   ./a.out
   ```

2. The server will start listening on port `4221`.

## Usage

1.  Open the Postman or use a tool like `curl`.
2.  Access the server using `http://localhost:4221/[path]`.
    - Example: `http://localhost:4221/`
    - Example: `http://localhost:4221/echo/hello`
    - Example: `http://localhost:4221/user-agent`

Example using curl:

```bash
curl -v http://localhost:4221/
curl -v http://localhost:4221/echo/hello
curl -v --header "User-Agent: foobar/1.2.3" http://localhost:4221/user-agent
```

## Implementation Details

### Socket Programming

- The server uses the `socket`, `bind`, `listen`, and `accept` system calls to create and manage network connections.
- It listens for incoming client connections on port `4221`.
- `SO_REUSEADDR` is used to allow immediate reuse of the port.

### Request Parsing

- The server reads the HTTP request from the client socket using `recv`.
- It parses the request line to extract the path.
- It parses the headers to extract the User-Agent.
- Uses `strtok` and `strncmp` for parsing.

### Response Generation

- The server generates HTTP responses based on the received requests.
- It includes the appropriate status codes (200 OK, 404 Not Found) and content.
- `/`: Returns a simple 200 OK response.
- `/echo/[content]`: Returns the content in the response body.
- `/user-agent`: Returns the User-Agent header value in the response body.
- Any other path returns a 404 Not Found response.

### Error Handling

- Basic error handling is implemented for socket operations and request parsing using `errno` and `strerror`.
- Checks return values of system calls and prints error messages.

## Code Explanation

- Socket Creation and Binding:
  - `socket()` creates the socket.
  - `setsockopt()` sets the SO_REUSEADDR option.
  - `bind()` associates the socket with the server address.
- Listening and Accepting Connections:
  - `listen()` puts the socket in listening mode.
  - `accept()` accepts incoming client connections.
- Request Handling:
  - `recv()` receives the client's request.
  - `strtok()` and `strncmp()` are used to parse the request and headers.
  - Conditional statements handle different request paths.
- Response Sending:
  - `send()` sends the HTTP response to the client.
  - `sprintf()` is used to format the response.
- Error Checking:
  - Error checks are performed after each system call.

## Future Enhancements

- Implement support for other HTTP methods (POST, PUT, DELETE).
- Add support for serving static files.
- Improve error handling and logging.
- Add HTTPS support.
- Add more robust header parsing.
- Add configuration file support.
- Add more robust request parsing.
