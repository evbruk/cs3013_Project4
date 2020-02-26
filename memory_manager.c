#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


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

int getPageTable(int processId)
{
	if(registers[processId] == -1)
	{
		for(int i = 0; i < 4; i++)
		{
			if(freelist[i] == 0)
			{
				registers[i] = i*16;
				freelist[i] = 1;
				printf("put page table for PID %d into physical frame %d\n", processId, i);
				break;			
			}
		}	
	}else{
		return registers[processId];
	}
}

int main(int argc, char * argv[])
{
	
	printf("Initializing... \n");
	
	while(1)
	{
		printf("Instruction? ");
		int pid, virtual_address,value;
		char * instruction;
		char * input;
		scanf("%s", input);
		printf("input: %s \n", input);		
		
		char * pidString = strtok(input, ",");
		char * endptr;
		pid = strtol(pidString, &endptr, 10);
		char * splitWord = 0;		
		int count = 0;
		splitWord = pidString;
		while( splitWord != NULL)
		{
			splitWord = strtok(NULL, ",");
			if(splitWord != NULL)
			{
				if(count == 0)
				{
					instruction = splitWord;					
				}else if(count == 1)
				{
					virtual_address = strtol(splitWord, &endptr, 10);				
				}else if(count == 2)
				{
					value = strtol(splitWord, &endptr, 10);						
				}
				count++;
			}			
		}
		printf("PID: %d instruction: %s value: %d \n", pid, instruction, value);
		printf("virtual address: %d \n", virtual_address);
		//first 2 bits of virtual address is the VPN
		int offset_mask = 15;
		int vpn_mask = 48;
		int vpn = virtual_address & vpn_mask;
		int offset = virtual_address & offset_mask;

		printf("vpn: %d \n", vpn);
		printf("offset: %d \n", offset);

		//gets the page table address, if there isn't a page table, it allocates one.
		int page_table_address = getPageTable(pid);
		
		if( strcmp(instruction, "allocate") == 0 )
		{
			printf("Allocating...\n");
			//check freelist, get a page, add a pte for that vpn.
			
			//page_table_address = vpn*16;
			//if the page table can be fetched from the pid, why does the VPN even exist?
			//there are actually only 4 page table entries, 0,1,2, or 3 (so we add vpn)
			int page_table_entry_index = page_table_address + vpn;
			
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
			
			//create page table entry struct by casting the pointer
			struct pte * pageTableEntry = (struct pte *)memory + page_table_entry_index;
			
			pageTableEntry->address = address;
			pageTableEntry->write = value;
			int frameNumber = (address % 16) + 1;
			printf("Mapped virtual address %d into physical frame %d\n", (address % 16),  frameNumber);
			//int result = allocate(pid, value);
			

			//where to set the valid bit?
			
		}
		if( strcmp(instruction, "load") == 0 )
		{
			int page_table_entry_index = page_table_address + vpn;
			struct pte * pageTableEntry = (struct pte *)memory + page_table_entry_index;
			int value_index = pageTableEntry->address + offset;
			int read_value = memory[value_index];
		}
		if( strcmp(instruction, "store") == 0)
		{
			int page_table_entry_index = page_table_address + vpn;
			struct pte * pageTableEntry = (struct pte *)memory + page_table_entry_index;
			int value_index = pageTableEntry->address + offset;
			memory[value_index] = value;
		}

	}
	
}
