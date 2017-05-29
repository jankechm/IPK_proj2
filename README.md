# Client for computing mathematic tasks

The client connects to a server, receives mathematic task, solves it and sends back the result.

## Syntax

    ipk-client hostname

*hostname* can be valid IPv4 or IPv6 address or a domain name of the server.

## Description

The communication between the client and the server is done by sending messages HELLO, SOLVE, RESULT and BYE.

The client begins the communication by setting up a TCP connection with the server on the port 55555.
After that, the client sends the message "HELLO id\n", where id is a md5 hash of the student login.

The server can answer by the message "SOLVE number operation number\n".
Allowed operations are +, -, *, /.
The client computes the mathematic task and sends the solution in the message "RESULT solution\n".
If the client is unable to solve the mathematic task (dividing by zero, overflow), it sends the message "RESULT ERROR\n".

The server can continue by another SOLVE message, or by the message "BYE secret\n".
The client writes out the value of secret to the stdout and closes the connection.

## Return value

On success, the program returns EXIT_SUCCESS. If an error occurs, the program returns EXIT_FAILURE.

## Examples

Connecting to a public server:

	ipk-client 147.229.13.210
	ipk-client 2001:67c:1220:80c::93e5:dd2
	ipk-client pcgregr.fit.vutbr.cz

Connecting to the localhost:

	ipk-client 127.0.0.1
	ipk-client ::1
	ipk-client localhost

## Bugs

If the server unexpectedly closes the connection, the client will be still waiting for the BYE message ad infinitum.
