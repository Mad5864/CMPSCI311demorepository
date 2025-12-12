# CMPSCI311demorepository
CMPSC 311: TCP SERVER/CLIENT FINAL PROJECT 
By SMART GROUP 3: Michael DeSalis, Jaden Clay, Ariana Sookoo 

Project Description 
This report outlines the requirements for creating a chat application. Using the C 	programming language, client/server architecture, and the concepts of socket programming, the chat application supports multiple users joining the chat room. Once one user sends a message, all clients connected to the server receive the message. The design of the code also accounts for issues such as concurrency, race conditions, atomicity and it is displayed using Pygame GUI. 

Libraries to Install: 
Pygame: To run the GUI, users should install Pygame which runs best using Python version 3.12. 

Scope: 

Tcp_server: 
1. Scalable from one client to several. 
2. Receives multiple messages from the same client file descriptor. 
3. Push data from one client to several other clients. 
4. Prevent race conditions such as clients sending a message at the same time (mutexes). 

Tcp_client: 
1. Run until an interrupt signal is received. 
2. Receive data from the server. 
3. Send several messages instead of one. 
4. Graphical User Interface: 
5. Use a GUI library for the C program to interact with the program. 

Addressing System-Based Issues: 
- Concurrency: The server spawns a detached pthread per accepted connection (pthread_create + pthread_detach) so clients are handled concurrently by per-client threads. 

- Race conditions: The message_to_client function first takes the given message, then pthread_mutex_lock is used to stop any race conditions then the message is written to each client. 

- Atomicity: There is a shared state (clientNum) and broadcasts are protected by a single pthread_mutex_t (mtx), so updates/removals and the broadcast loop are mutually exclusive. Adding/removing sockets in clientNum and iterating/sending inside message_to_clients are executed under the mutex, preventing concurrent races on that array. 
