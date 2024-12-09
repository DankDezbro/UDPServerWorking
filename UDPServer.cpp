#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define PORTNUM 80
#define PACKETSIZE 1025 // 1KB + ACK
#define FILELEN 9319
#define NUMBEROFPACKETS 10


/** declare variable wsa **/
WSADATA wsa;

/** declare socket variables **/
struct sockaddr_in si_other;

int slen = sizeof(si_other);

SOCKET s;


typedef struct {
	int seqNum;
	char* data;
}packet;


int initalizeWinsock()
{
	printf("\n****** INITIALIZING WINSOCK ***********");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
	else printf("\nWINSOCK INITIALIZED");

	return 0;
}

int initializeSocket(int portNum)
{
	/*****  CREATE SERVER SOCKET  ****/
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
		return 1;
	}
	else printf("\nUDP SERVER SOCKET CREATED");
	si_other.sin_addr.s_addr = INADDR_ANY; // or INADDR_ANY
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(portNum);
	return 0;
}



int main()
{
	/*****  INITALIZE WINSOCK  ****/
	initalizeWinsock();

	/*****  CREATE SERVER SOCKET  ****/
	initializeSocket(PORTNUM);

	/*****  BIND SOCKET  ****/
	if (bind(s, (struct sockaddr*)&si_other, sizeof(si_other)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("\nSERVER SOCKET BIND SUCCESS");

	/*****  WAIT FOR DATA  ****/
	printf("\nWaiting for data...");

	char* buffer = (char*)malloc((FILELEN + 1));  //allocated memory to receiving buffer
	int currentVal = 0; //current value of the position within buffer
	char* seqNum = (char*)malloc(1);; //sequence number

	char* packet = (char*)malloc(PACKETSIZE); //allocate memory to hold packet
	int recv_len;

	for (int i = 0; i < NUMBEROFPACKETS; i++)
	{
		fflush(stdout);
		memset(packet, '\0', PACKETSIZE);  //clear buffer of previously received data

		/********  RECEIVE DATA PACKET - blocking *************/


		if ((recv_len = recvfrom(s, packet, PACKETSIZE, 0, (struct sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		else printf("\nSERVER received packet IPaddr %s Port %d  ", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));

		seqNum[0] = packet[0];
		for (int j = 0; j < recv_len; j++)
		{
			packet[j] = packet[j + 1];
		}
		recv_len--;
		for (int j = 0; j < recv_len; j++)
		{
			buffer[j + currentVal] = packet[j];
		}
		currentVal = currentVal + recv_len; //update position within buffer
		cout << "\nSERVER received packet: " << i << " sequence number: " << seqNum[0] << " bytes received: " << currentVal << endl;
		sendto(s, seqNum, 1, 0, (struct sockaddr*)&si_other, slen);
		seqNum++;
	}


	/*********** write contents of buffer to a file ************/
	FILE* writefp = fopen("testReceived.jpg", "wb");
	if (writefp == NULL)
	{
		printf("\nError Opening Image-write");
		fclose(writefp);
		exit(0);
	}
	else printf("\nfile opened for writing");
	fwrite(buffer, FILELEN + 1, 1, writefp);
	fclose(writefp);
	printf("\nc SAVED image, press any key");

	closesocket(s);
	WSACleanup();
	return 0;
}