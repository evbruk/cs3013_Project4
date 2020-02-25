#include <stdio.h>
#include <stdlib.h>

unsigned char memory[64];

int main(int argc, char * argv[])
{

	printf("Initializing... \n");
	printf("Grabbing Input: ");
	int pid, instruction,virtual_address,value;

	scanf("%d,%d,%d,%d", &pid, &instruction, &virtual_address, &value);
	printf("Input: %d, %d, %d, %d \n", pid, instruction, virtual_address, value);
	//first 2 bits of virtual address is the VPN
	int offset_mask = 15;
	int vpn_mask = 48;
	int vpn = virtualAddress & vpn_mask;
	int offet = virtualAddress & offset_mask;

	printf("vpn: %d \n", vpn);
	printf("offset: %d \n", offset);

	
}
