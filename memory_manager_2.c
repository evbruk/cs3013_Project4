#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <limits.h>


//memory storing the page tables
unsigned char memory[64];
unsigned char disk[15*5*4];

int freelist[4] = {0,0,0,0};
int writable[4] = {0,0,0,0};
//simulated hardware registers
int registers[4] = {-1, -1, -1, -1};

//counter of commands issued over time, used for LRU policy
int commandsEntered = 0;

//struct for each page table entry
struct pte {
	uint8_t address;
	uint8_t present; //if it is present/valid
	uint8_t write; //for write permission
	uint8_t useCounter; //used for the LRU eviction policy
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
				registers[processId] = i*16;
				freelist[i] = 1;
				printf("put page table for PID %d into physical frame %d\n", processId, i);
				break;			
			}
		}
		if(registers[processId] == -1)
		{
			//we must swap	
			printf("registers[processId] == -1 ******************\n");
		}
	}
	
	return registers[processId];	
}

int getPageNumber(int address)
{
	if( address < 16)
	{
		return 0;	
	}else if(address > 15 && address < 32)
	{
		return 1;	
	}else if(address > 31 && address < 48)
	{
		return 2;	
	}else if(address > 47)
	{
		return 3;	
	}
	return -1;
}

int getDiskAddress(int pid)
{
	return pid * 5;
}

//physical_address is the physical address of
int swap()
{
	//copy memory from memory + physical address of page frame, to disk, width of 16 bytes
	
	int diskOffset = 0; //so that the page table isn't overwritten if vpn is 0
	int pageToBeSwapped = 0;
	
	int smallestUses = 10000;
	int smallestAddress = 0;
	int vpn_of_eviction = 0;
	int pid_of_eviction = 0;
	
	struct pte * evicted_page;
	for(int i = 0; i < 4; i++)
	{
		//page table exists
		if(registers[i] != -1)
		{
			int page_table_address = registers[i];	
			
			for(int j = 0; j < 4; j++)
			{
				struct pte * pageTableEntry = (struct pte *)memory + page_table_address + j;
				if(pageTableEntry->address != 0)
				{
					//page table entry is in use
					if(pageTableEntry->useCounter < smallestUses)
					{
						smallestUses = pageTableEntry->useCounter;
						smallestAddress = pageTableEntry->address;
						vpn_of_eviction = j; //keep track for later
						pid_of_eviction = i;
						pageTableEntry->present = 0;
						evicted_page = pageTableEntry;		
					}
				}
			}
		}
	}

	pageToBeSwapped = smallestAddress;
	diskOffset = vpn_of_eviction + 1; //j is the "vpn"

	for(int i = 0; i < 4; i++)
	{
		if(registers[i] == pageToBeSwapped) //make sure that we put the page table at index 0
		{
			//we are swapping out a page table so we should stick it at index 0
			//for that process on disk
			diskOffset = 0;	
		}	
	}
	
	int index = getDiskAddress(pid_of_eviction) + diskOffset;
	memcpy((disk + (16*index)), (memory + pageToBeSwapped), 16);
	printf("Swapped frame %d to disk at swap slot %d\n", getPageNumber(pageToBeSwapped), pid_of_eviction);
	evicted_page->address = 16*index;
	return pageToBeSwapped; //return the address of the evicted page
}

//physical_address == base address of page in the disk (grabbed from the pte)
int fetch(int pid, int vpn, int disk_address)
{
	//this might get tricky if this is a page table?	
	int diskAddress = getDiskAddress(pid) + vpn + 1;
	//if a page table we just won't have an offset
	
	//find free page and get physical_address from there.
	int physical_address = -1;
	for(int i = 0; i < 4; i++)
	{
		if(freelist[i] == 0)
		{
			physical_address = i*16;
		}
	}
	
	if(physical_address == -1)
	{
		//swap out and grab the evicted address
		physical_address = swap();	
	}
	
	memcpy((memory + physical_address), (disk + disk_address), 16);
	return physical_address;
}

int main(int argc, char * argv[])
{
	
	printf("Initializing... \n");
	
	while(1)
	{
		printf("Instruction? ");
		int pid, virtual_address,value;
		char * instruction;
		char input[256];
		scanf("%s", input);
		printf("input: %s \n", input);		
		
		
		//printf("before strtok call \n");
		char * pidString = strtok(input, ",");
		//printf("after strtok call \n");
		char * endptr;
		pid = strtol(pidString, &endptr, 10);
		char * splitWord = 0;		
		int count = 0;
		splitWord = pidString;
		printf("before processing commands\n");
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
		commandsEntered++;
		//printf("PID: %d instruction: %s value: %d \n", pid, instruction, value);
		//printf("virtual address: %d \n", virtual_address);
		//first 2 bits of virtual address is the VPN
		int offset_mask = 15;
		int vpn_mask = 48;
		int vpn = virtual_address & vpn_mask;
		int offset = virtual_address & offset_mask;

		//printf("vpn: %d \n", vpn);
		//printf("offset: %d \n", offset);

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
			
					
			//create page table entry struct by casting the pointer
			struct pte * pageTableEntry = (struct pte *)memory + page_table_entry_index;
			if(pageTableEntry->address != 0)
			{
				printf("Updating permissions for virtual page %d (frame %d) \n", vpn, getPageNumber(pageTableEntry->address));
				pageTableEntry->write = value;
			}else{
				
				int address = -1;
				for(int i = 0; i < 4; i++)
				{
					if(freelist[i] == 0)
					{
						//found an empty page
						address = i*16; //base address
						freelist[i] = 1;
						break;		
					}
				}
				if ( address == -1)
				{
					printf("Swapping... \n");
					int swappedPage = swap();
					address = swappedPage;					
					//we must swap a page out to disk
					//how to determine index of the disk array? -->vpn
					//pick a page, figure out the vpn that points to it, store that in the owner process's disk
					//figuring out the VPN: find the owner Process, search for address in page table
						//might involve searching every page table

					//get page table entry for swapped out portion (got from search) and update the present bit to 0			
				}
				int frameNumber = getPageNumber(address);
				pageTableEntry->address = address;
				pageTableEntry->write = value;
				pageTableEntry->present = 1;
				pageTableEntry->useCounter = commandsEntered;
				printf("Mapped virtual address %d into physical frame %d\n", virtual_address,  frameNumber);			
			}
			//int result = allocate(pid, value);
			

			//where to set the valid bit?
			
		}
		if( strcmp(instruction, "load") == 0 )
		{
			int page_table_entry_index = page_table_address + vpn;
			struct pte * pageTableEntry = (struct pte *)memory + page_table_entry_index;
			int address = 0;
			if(pageTableEntry->present == 0)
			{
				printf("fetching from disk...\n");
				address = fetch(pid, vpn, pageTableEntry->address);
				pageTableEntry->address = address;
				//swap(pid, pageTableEntry->address); //modify function to support fetching too		
			}
			
			
			int value_index = pageTableEntry->address + offset;
			int read_value = memory[value_index];
			pageTableEntry->useCounter = commandsEntered;
			printf("The value %d is virtual address %d \n", read_value, virtual_address);

		}
		if( strcmp(instruction, "store") == 0)
		{
			int page_table_entry_index = page_table_address + vpn;
			struct pte * pageTableEntry = (struct pte *)memory + page_table_entry_index;
			pageTableEntry->useCounter = commandsEntered;
			if(pageTableEntry->write == 0)
			{
				printf("Error: writes are not allowed to this page \n");
			}else if(pageTableEntry->present == 0)
			{
				printf("Fetching from disk... \n");
				int address = fetch(pid, vpn, pageTableEntry->address);
				pageTableEntry->address = address;
				int index = pageTableEntry->address + offset;
				memory[index] = value;
				printf("Store value %d at virtual address %d (physical address %d)\n", value, virtual_address, index);
				//swap(pid, pageTableEntry->address);//change function to support fetching too.			
			}
			else
			{
				int value_index = pageTableEntry->address + offset;
				memory[value_index] = value;
			
				printf("Store value %d at virtual address %d (physical address %d)\n", value, virtual_address, value_index);
			}		
			
		}

	}
	
}
