#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//memory storing the page tables
unsigned char memory[64];

int freelist[4] = {0,0,0,0};
int writable[4] = {0,0,0,0};
//simulated hardware registers
int registers[4] = {-1, -1, -1, -1};

//struct for each page table entry
struct pte {
	uint8_t address:6;
	uint8_t present:1; //if it is present/valid
	uint8_t write:1; //for write permission
};

int allocate(int processId, int value)
{
	//first, check if a page table exists
	//registers[processId] == address of the process page table
	if(registers[processId] == -1)
	{
		//page table does not exist, we must create it
		for(int i = 0; i < 4; i++)
		{
			if(freelist[i] == 0)
			{
				//page is available
				registers[processId] = i*16;
				freelist[i] = 1;
				if(value) //if the page should be writable.
				{
					writable[i] = 1;			
				}
				break;
				//create and allocate a bunch of pte?	
			}	
		}
	}
	int page_table_address = registers[processId];
	
	//cast the array to a struct pte pointer (1 byte in size)
	struct pte * page_table_pointer;
	page_table_pointer = (struct pte *)&registers;
	//increment the pointer to point to the start of our page table
	page_table_pointer += page_table_address;
	
	printf("page table address for process %d: %d \n", processId, page_table_address);
	//create a page table entry to map between the virtual and physical address
	for(int i = 0; i < 4; i++)
	{
		if(freelist[i] == 0)
		{
			//page frame is free
			freelist[i] == 1;
			//create a page table entry to map between the virtual and physical address
			
			
		
			//registers[processId] + offset?
			//create a page table entry		
		}	
	}

	//add case for full memory (return 1)
	return 0;
}

int main(int argc, char * argv[])
{
	
	printf("Initializing... \n");
	
	while(1)
	{
		printf("Enter Input: ");
		int pid, instruction,virtual_address,value;

		scanf("%d,%d,%d,%d", &pid, &instruction, &virtual_address, &value);
		printf("Input: %d, %d, %d, %d \n", pid, instruction, virtual_address, value);
		//first 2 bits of virtual address is the VPN
		int offset_mask = 15;
		int vpn_mask = 48;
		int vpn = virtual_address & vpn_mask;
		int offset = virtual_address & offset_mask;

		printf("vpn: %d \n", vpn);
		printf("offset: %d \n", offset);

		int page_table_address = registers[pid];
		switch(instruction)
		{
			case 0:
				printf("Allocating...\n");
				//check freelist, get a page, add a pte for that vpn.
				
				int page_table_index = vpn*16;
				int address = -1;
				for(int i = 0; i < 4; i++)
				{
					if(freelist[i] == 0)
					{
						//found an empty page
						address = i*16; //base address
						freelist[i] = 1;
											
					}
				}				
				int pte_address = page_table_index + offset;

				//create page table entry struct by casting the pointer
				struct pte * pageTableEntry = (struct pte *)memory + pte_address;
				
				pageTableEntry->address = address;
				pageTableEntry->write = value;
				
				//int result = allocate(pid, value);
				

				//where to set the valid bit?
				if(result == 1)
				{
					printf("Allocation failed! memory full.");			
				}
			break;
			case 1:
				printf("Storing...\n");
				
			break;
			case 2:
				printf("Loading...");
				//index of the whole memory block [0, 63]			
				int page_table_address = registers[pid];
				int translated_address = page_table_address + offset;
				int value = memory[translated_address];
				printf("loaded value %d \n", value);
			break;
		}

	}
	
}
