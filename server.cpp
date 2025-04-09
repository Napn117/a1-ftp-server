//=======================================================================================================================
// Course: 159.342
// Description: Cross-platform, Active mode FTP SERVER, Start-up Code for Assignment 1
//
// This code gives parts of the answers away but this implementation is only IPv4-compliant.
// Remember that the assignment requires a fully IPv6-compliant cross-platform FTP server that can communicate with a
// built-in FTP client either in Windows 11, Ubuntu Linux or MacOS.
//
// This program is cross-platform but your assignment will be marked only in Windows 11.
//
// You must change parts of this program to make it IPv6-compliant (replace all data structures and functions that work only with IPv4).
//
// Hint: The sample TCP server codes show the appropriate data structures and functions that work in both IPv4 and IPv6.
//       We also covered IPv6-compliant data structures and functions in our lectures.
//
// Author: n.h.reyes@massey.ac.nz
//=======================================================================================================================

#define USE_IPV6 true // if set to false, IPv4 addressing scheme will be used; you need to set this to true to
											// enable IPv6 later on.  The assignment will be marked using IPv6!

#if defined __unix__ || defined __APPLE__
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h> //used by getnameinfo()
#include <iostream>

#elif defined __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h> //required by getaddrinfo() and special constants
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
//   #include <WinSock2.h>
#define WSVERS MAKEWORD(2, 2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
// The high-order byte specifies the minor version number;
// the low-order byte specifies the major version number.
WSADATA wsadata; // Create a WSADATA object called wsadata.
#endif

#define BUFFER_SIZE 256

enum class FileType
{
	BINARY,
	TEXT,
	UNKNOWN
};

FileType file_type;

//********************************************************************
// MAIN
//********************************************************************
int main(int argc, char *argv[])
{
	//********************************************************************
	// INITIALIZATION
	//********************************************************************

	file_type = FileType::UNKNOWN;

#if defined __unix__ || defined __APPLE__
	// nothing to do here

#elif defined _WIN32
	int err = WSAStartup(WSVERS, &wsadata);

	if (err != 0)
	{
		WSACleanup();
		// Tell the user that we could not find a usable WinsockDLL
		printf("WSAStartup failed with error: %d\n", err);
		exit(1);
	}
#endif
	struct sockaddr_storage remoteaddr;
	struct sockaddr_storage local_data_addr_act;
	struct addrinfo *result = NULL, hints;
	int iResult;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

#if defined __unix__ || defined __APPLE__
	int s, ns;							 // socket declaration
	int ns_data, s_data_act; // socket declaration
#elif defined _WIN32
	SOCKET s, ns;								// socket declaration
	SOCKET ns_data, s_data_act; // socket declaration
#endif

	char send_buffer[BUFFER_SIZE], receive_buffer[BUFFER_SIZE];

#if defined __unix__ || defined __APPLE__
	ns_data = -1;
#elif defined _WIN32
	ns_data = INVALID_SOCKET;
#endif

	int active = 0;
	int n, bytes, addrlen;
#define DEFAULT_PORT "1234"
	char portNumber[NI_MAXSERV];
	char clientHost[NI_MAXHOST];
	char clientService[NI_MAXSERV];
	printf("\n============================================\n");
	printf("   << 159.342 Cross-platform FTP Server >>");
	printf("\n============================================\n");
	printf("   submitted by:   Jesse Easton-Harris, Luke Roycroft  ");
	printf("\n           date: 9/4/2025 ");
	printf("\n============================================\n");

	memset(&remoteaddr, 0, sizeof(remoteaddr)); // clean up the structure

	//********************************************************************
	// SOCKET
	//********************************************************************

	s = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

#if defined __unix__ || defined __APPLE__
	if (s < 0)
	{
		printf("socket failed\n");
	}
#elif defined _WIN32
	if (s == INVALID_SOCKET)
	{
		printf("socket failed\n");
	}
#endif

	// CONTROL CONNECTION:  port number = content of argv[1]
	if (argc == 2)
	{
		iResult = getaddrinfo(NULL, argv[1], &hints, &result);
		sprintf(portNumber, "%s", argv[1]);
	}
	/*NOTE: otherwise here is the default port to use*/
	else
	{
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		sprintf(portNumber, "%s", DEFAULT_PORT);
	}

	//********************************************************************
	// BIND
	//********************************************************************

	iResult = bind(s, result->ai_addr, (int)result->ai_addrlen);

// if error is detected, then clean-up
#if defined __unix__ || defined __APPLE__
	if (iResult == -1)
	{
		printf("\nbind failed\n");
		freeaddrinfo(result);
		close(s); // close socket
	}
#elif defined _WIN32
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(s);
		WSACleanup();
#endif
	return 1;
}

freeaddrinfo(result);

//********************************************************************
// LISTEN
//********************************************************************
listen(s, 5);

//********************************************************************
// INFINITE LOOP
//********************************************************************
int count = 0;
//====================================================================================
while (1)
{ // Start of MAIN LOOP
	//====================================================================================
	addrlen = sizeof(remoteaddr);
	//********************************************************************
	// NEW SOCKET newsocket = accept  //CONTROL CONNECTION
	//********************************************************************
	printf("\n------------------------------------------------------------------------\n");
	printf("SERVER is waiting for an incoming connection request at port:%d", atoi(portNumber));
	printf("\n------------------------------------------------------------------------\n");

#if defined __unix__ || defined __APPLE__
	ns = accept(s, (struct sockaddr *)(&remoteaddr), (socklen_t *)&addrlen);
#elif defined _WIN32
		ns = accept(s, (struct sockaddr *)(&remoteaddr), &addrlen);
#endif

#if defined __unix__ || defined __APPLE__
	if (ns < 0)
		break;
#elif defined _WIN32
		if (ns == INVALID_SOCKET)
			break;
#endif
	getnameinfo((struct sockaddr *)&remoteaddr, addrlen, clientHost, sizeof(clientHost), clientService, sizeof(clientService), NI_NUMERICHOST);

	printf("\n============================================================================\n");
	printf("connected to [CLIENT's IP %s , port %d] through SERVER's port %d",
				 clientHost, atoi(clientService), atoi(portNumber));
	printf("\n============================================================================\n");

	//********************************************************************
	// Respond with welcome message
	//*******************************************************************

	count = snprintf(send_buffer, BUFFER_SIZE, "220 FTP Server ready. \r\n");

	if (count >= 0 && count < BUFFER_SIZE)
	{
		bytes = send(ns, send_buffer, strlen(send_buffer), 0);
	}

	//********************************************************************
	// COMMUNICATION LOOP per CLIENT
	//********************************************************************
	bool isConnected = false;
	while (1)
	{
		n = 0;

		while (1)
		{
			//********************************************************************
			// RECEIVE MESSAGE AND THEN FILTER IT
			//********************************************************************

			bytes = recv(ns, &receive_buffer[n], 1, 0); // receive byte by byte...

			if ((bytes < 0) || (bytes == 0))
				break;
			if (receive_buffer[n] == '\n')
			{ /*end on a LF*/
				receive_buffer[n] = '\0';
				break;
			}
			if (receive_buffer[n] != '\r')
				n++; /*Trim CRs*/
		}

		if (bytes == 0)
			printf("\nclient has gracefully exited.\n"); // 2022

		if ((bytes < 0) || (bytes == 0))
			break;

		printf("[DEBUG INFO] command received:  '%s\\r\\n' \n", receive_buffer);

		//********************************************************************
		// PROCESS COMMANDS/REQUEST FROM USER
		//********************************************************************
		if (strncmp(receive_buffer, "USER", 4) == 0)
		{
			char username[] = "napoleon";
			char input_user[50];
			int scannedItems = sscanf(receive_buffer, "USER %s", input_user);

			if (scannedItems < 1)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			if (strcmp(input_user, username) == 0)
			{
				printf("Logging in... \n");
				count = snprintf(send_buffer, BUFFER_SIZE, "331 Username accepted. Password required...\r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			else
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "530 Public login fail \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
		}
		//---
		if (strncmp(receive_buffer, "PASS", 4) == 0)
		{
			char password[] = "342";
			char input_password[50];
			int scannedItems = sscanf(receive_buffer, "PASS %s", input_password);

			if (scannedItems < 1)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			if (strcmp(input_password, password) == 0)
			{
				printf("Logging in... \n");
				isConnected = true;
				count = snprintf(send_buffer, BUFFER_SIZE, "230 OK Successful log in\r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			else
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "530 Public login fail \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
		}
		//---
		if (strncmp(receive_buffer, "SYST", 4) == 0)
		{
			if (isConnected == false)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "530 Public login fail \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			else
			{
				printf("Information about the system \n");
				count = snprintf(send_buffer, BUFFER_SIZE, "215 Windows 64-bit\r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
		}

		//---
		if (strncmp(receive_buffer, "TYPE", 4) == 0)
		{
			if (isConnected == false)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "530 Public login fail \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			else
			{
				bytes = 0;
				printf("<--TYPE command received.\n\n");

				char objType;
				int scannedItems = sscanf(receive_buffer, "TYPE %c", &objType);
				if (scannedItems < 1)
				{
					count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments\r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					}
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
					if (bytes < 0)
						break;
				}

				switch (toupper(objType))
				{
				case 'I':
					file_type = FileType::BINARY;
					printf("using binary mode to transfer files.\n");
					count = snprintf(send_buffer, BUFFER_SIZE, "200 command OK.\r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					}
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
					if (bytes < 0)
						break;

					break;
				case 'A':
					file_type = FileType::TEXT;
					printf("using ASCII mode to transfer files.\n");
					count = snprintf(send_buffer, BUFFER_SIZE, "200 command OK.\r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					}
					printf("[DEBUG INFO] <-- %s\n", send_buffer);

					if (bytes < 0)
						break;

					break;
				default:
					count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments\r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					}
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
					if (bytes < 0)
						break;
					break;
				}
			}
		}
		//---
		if (strncmp(receive_buffer, "STOR", 4) == 0)
		{
			printf("unrecognised command \n");
			count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
			if (count >= 0 && count < BUFFER_SIZE)
			{
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
			}
			printf("[DEBUG INFO] <-- %s\n", send_buffer);
			if (bytes < 0)
				break;
		}
		//---
		if (strncmp(receive_buffer, "RETR", 4) == 0)
		{
			if (isConnected == false)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "530 Public login fail \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			else
			{
				char file_requested[50];
				int scannedItems = sscanf(receive_buffer, "RETR %s", file_requested);

				if (scannedItems < 1)
				{
					count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments \r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					}
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
					if (bytes < 0)
						break;
				}

				FILE *fin;
				fin = fopen(file_requested, file_type == FileType::BINARY ? "rb" : "r");

				count = snprintf(send_buffer, BUFFER_SIZE, "150 Opening data connection... \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
				}
				char text_buffer[80];

				printf("transferring file...\n");

				s_data_act = socket(AF_INET6, SOCK_STREAM, 0);
				active = 1; // flag for active connection
				char dataHost[NI_MAXHOST];
				char dataService[NI_MAXSERV];
				getnameinfo((struct sockaddr *)&local_data_addr_act, addrlen, dataHost, sizeof(dataHost), dataService, sizeof(dataService), NI_NUMERICHOST);

				if (connect(s_data_act, (struct sockaddr *)&local_data_addr_act, sizeof(struct sockaddr_in6)) != 0)
				{
					printf("trying connection in %s %d\n", dataHost, atoi(dataService));
					printf("%d", WSAGetLastError());
					count = snprintf(send_buffer, BUFFER_SIZE, "425 Something is wrong, can't start active connection... \r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);

						printf("[DEBUG INFO] <-- %s\n", send_buffer);
					}

#if defined __unix__ || defined __APPLE__
					close(s_data_act);
#elif defined _WIN32
						closesocket(s_data_act);
#endif
				}
				else
				{
					count = snprintf(send_buffer, BUFFER_SIZE, "200 EPRT Command successful\r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
						printf("[DEBUG INFO] <-- %s\n", send_buffer);
						printf("Connected to client\n");
					}
				}
				if (file_type == FileType::TEXT)
				{
					while (!feof(fin))
					{
						strcpy(text_buffer, "");
						if (fgets(text_buffer, 78, fin) != NULL)
						{

							count = snprintf(send_buffer, BUFFER_SIZE, "%s", text_buffer);
							printf("%i\n", count);
							if (count >= 0 && count < BUFFER_SIZE)
							{

								if (active == 0)
									send(ns_data, send_buffer, strlen(send_buffer), 0);
								else
									send(s_data_act, send_buffer, strlen(send_buffer), 0);
							}
						}
					}
				}
				else
				{
					while ((count = fread(send_buffer, sizeof(char), BUFFER_SIZE, fin)) > 0)
					{
						printf("%d\n", count);
						if (active == 0)
						{
							send(ns_data, send_buffer, count, 0);
						}
						else
						{
							send(s_data_act, send_buffer, count, 0);
						}
					}
				}

				fclose(fin);
				count = snprintf(send_buffer, BUFFER_SIZE, "226 File transfer complete, closing data connection \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
				}

#if defined __unix__ || defined __APPLE__
				if (active == 0)
					close(ns_data);
				else
					close(s_data_act);

#elif defined _WIN32
					if (active == 0)
						closesocket(ns_data);
					else
						closesocket(s_data_act);
#endif
			}
		}
		//---
		if (strncmp(receive_buffer, "OPTS", 4) == 0)
		{
			printf("unrecognised command \n");
			count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
			if (count >= 0 && count < BUFFER_SIZE)
			{
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
			}
			printf("[DEBUG INFO] <-- %s\n", send_buffer);
			if (bytes < 0)
				break;
		}
		//---
		if (strncmp(receive_buffer, "EPRT", 4) == 0)
		{
			if (isConnected == false)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "530 Public login fail \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			else
			{
				int protocol_number;
				char ip_address[50];
				int port_number;
				int counter = 0;
				char *tokenPtr = strtok(receive_buffer, "|");
				while (tokenPtr != NULL)
				{
					switch (counter)
					{
					case 0:
						break;
					case 1:
						protocol_number = atoi(tokenPtr);
						break;
					case 2:
						strcpy(ip_address, tokenPtr);
						break;
					case 3:
						port_number = atoi(tokenPtr);
						break;
					default:
						break;
					}
					counter++;
					tokenPtr = strtok(NULL, "|");
				}
				printf("\nip: %s", ip_address);
				if (protocol_number == 1)
				{
					struct sockaddr_in *addr4 = (sockaddr_in *)&local_data_addr_act;
					addr4->sin_family = AF_INET;
					addr4->sin_port = htons(port_number);
					inet_pton(AF_INET, ip_address, &(addr4->sin_addr));
				}
				else
				{
					struct sockaddr_in6 *addr6 = (sockaddr_in6 *)&local_data_addr_act;
					addr6->sin6_family = AF_INET6;
					addr6->sin6_port = htons(port_number);
					inet_pton(AF_INET6, ip_address, &(addr6->sin6_addr));
				}

				count = snprintf(send_buffer, BUFFER_SIZE, "200 EPRT Command successful\r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			//---
			if (strncmp(receive_buffer, "CWD", 3) == 0)
			{
				printf("unrecognised command \n");
				count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
		}
		//---
		if (strncmp(receive_buffer, "QUIT", 4) == 0)
		{
			printf("Quit \n");
			count = snprintf(send_buffer, BUFFER_SIZE, "221 Connection close by client\r\n");
			if (count >= 0 && count < BUFFER_SIZE)
			{
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
			}
			printf("[DEBUG INFO] <-- %s\n", send_buffer);
			if (bytes < 0)
				break;
		}
		//---
		if (strncmp(receive_buffer, "PORT", 4) == 0)
		{
			//  local variables
			int act_port[2];
			int act_ip[4], port_dec;
			char ip_decimal[NI_MAXHOST];

			printf("===================================================\n");
			printf("\tActive FTP mode, the client is listening... \n");

			int scannedItems = sscanf(receive_buffer, "PORT %d,%d,%d,%d,%d,%d",
																&act_ip[0], &act_ip[1], &act_ip[2], &act_ip[3],
																&act_port[0], &act_port[1]);

			if (scannedItems < 6)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			struct sockaddr_in *ipv4_address = (sockaddr_in *)&local_data_addr_act;
			ipv4_address->sin_family = AF_INET;

			count = snprintf(ip_decimal, NI_MAXHOST, "%d.%d.%d.%d", act_ip[0], act_ip[1], act_ip[2], act_ip[3]);

			if (!(count >= 0 && count < BUFFER_SIZE))
				break;

			printf("\tCLIENT's IP is %s\n", ip_decimal);
			ipv4_address->sin_addr.s_addr = inet_addr(ip_decimal);

			port_dec = act_port[0];
			port_dec = port_dec << 8;
			port_dec = port_dec + act_port[1];
			printf("\tCLIENT's Port is %d\n", port_dec);
			printf("===================================================\n");
			ipv4_address->sin_port = htons(port_dec);
		}
		//---
		// technically, LIST is different than NLST,but we make them the same here
		if ((strncmp(receive_buffer, "LIST", 4) == 0) || (strncmp(receive_buffer, "NLST", 4) == 0))
		{
			if (isConnected == false)
			{
				count = snprintf(send_buffer, BUFFER_SIZE, "530 Public login fail \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				}
				printf("[DEBUG INFO] <-- %s\n", send_buffer);
				if (bytes < 0)
					break;
			}
			else
			{
#if defined __unix__ || defined __APPLE__

				int i = system("ls -la > tmp.txt"); // change that to 'dir', so windows can understand

#elif defined _WIN32

					int i = system("dir > tmp.txt");
#endif
				printf("The value returned by system() was: %d.\n", i);

				FILE *fin;

				fin = fopen("tmp.txt", "r"); // open tmp.txt file

				// snprintf(send_buffer,BUFFER_SIZE,"125 Transfering... \r\n");
				// snprintf(send_buffer,BUFFER_SIZE,"150 Opening ASCII mode data connection... \r\n");
				count = snprintf(send_buffer, BUFFER_SIZE, "150 Opening data connection... \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
				}
				char text_buffer[80];
				printf("transferring file...\n");

				s_data_act = socket(AF_INET6, SOCK_STREAM, 0);
				active = 1; // flag for active connection
				char dataHost[NI_MAXHOST];
				char dataService[NI_MAXSERV];
				getnameinfo((struct sockaddr *)&local_data_addr_act, addrlen, dataHost, sizeof(dataHost), dataService, sizeof(dataService), NI_NUMERICHOST);

				if (connect(s_data_act, (struct sockaddr *)&local_data_addr_act, sizeof(struct sockaddr_in6)) != 0)
				{
					printf("trying connection in %s %d\n", dataHost, atoi(dataService));
					printf("%d", WSAGetLastError());
					count = snprintf(send_buffer, BUFFER_SIZE, "425 Something is wrong, can't start active connection... \r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);

						printf("[DEBUG INFO] <-- %s\n", send_buffer);
					}

#if defined __unix__ || defined __APPLE__
					close(s_data_act);
#elif defined _WIN32
						closesocket(s_data_act);
#endif
				}
				else
				{
					count = snprintf(send_buffer, BUFFER_SIZE, "200 EPRT Command successful\r\n");
					if (count >= 0 && count < BUFFER_SIZE)
					{
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
						printf("[DEBUG INFO] <-- %s\n", send_buffer);
						printf("Connected to client\n");
					}
				}

				while (!feof(fin))
				{
					strcpy(text_buffer, "");
					if (fgets(text_buffer, 78, fin) != NULL)
					{

						count = snprintf(send_buffer, BUFFER_SIZE, "%s", text_buffer);
						if (count >= 0 && count < BUFFER_SIZE)
						{

							if (active == 0)
								send(ns_data, send_buffer, strlen(send_buffer), 0);
							else
								send(s_data_act, send_buffer, strlen(send_buffer), 0);
						}
					}
				}

				fclose(fin);
				count = snprintf(send_buffer, BUFFER_SIZE, "226 File transfer complete, closing data connection \r\n");
				if (count >= 0 && count < BUFFER_SIZE)
				{
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					printf("[DEBUG INFO] <-- %s\n", send_buffer);
				}

#if defined __unix__ || defined __APPLE__
				if (active == 0)
					close(ns_data);
				else
					close(s_data_act);

#elif defined _WIN32
					if (active == 0)
						closesocket(ns_data);
					else
						closesocket(s_data_act);

						// OPTIONAL, delete the temporary file
						// system("del tmp.txt");
#endif
			}
		}
		//---
		//=================================================================================
	} // End of COMMUNICATION LOOP per CLIENT
	//=================================================================================

	//********************************************************************
	// CLOSE SOCKET
	//********************************************************************

#if defined __unix__ || defined __APPLE__
	close(ns);
#elif defined _WIN32

		closesocket(ns);
#endif
	printf("DISCONNECTED from %s\n", clientHost); // IPv4 only, needs replacing

	//====================================================================================
} // End of MAIN LOOP
//====================================================================================

printf("\nSERVER SHUTTING DOWN...\n");

#if defined __unix__ || defined __APPLE__
close(s);

#elif defined _WIN32
	closesocket(s);
	WSACleanup();
#endif
return 0;
}
