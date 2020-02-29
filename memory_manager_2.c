#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
//#include <limits.h>


//memory storing the page tables
unsigned char memory[64];
unsigned char disk[15*5*4];

int freelist[4] = {0,0,0,0};
int writable[4] = {0,0,0,0};
//simulated hardware registers
int registers[4] = {-1, -1, -1, -1};
int pageTableUses[4] = {0,0,0,0};
int pageTablePresent[4] = {1,1,1,1};

int swap();
int fetch(int pid, int vpn, int address);
//counter of commands issued over time, used for LRU policy
int commandsEntered = 0;

//struct for each page table entry
struct pte {
	uint8_t address;
	uint8_t present; //if it is present/valid
	uint8_t write; //for write permission
	uint8_t useCounter; //used for the LRU eviction policy
};


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
			int address = swap();
			registers[processId] = address;
		}
	}
	
	if(pageTablePresent[processId])
	{
		pageTableUses[processId] = commandsEntered;
		return registers[processId];
	}else
	{
		//fetch
		printf("Fetching page table from disk for PID %d\n", processId);
		fetch(processId, -1, registers[processId]);
		pageTablePresent[processId] = 1;
	}
		
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

//physical_address == base address of page in the disk (grabbed from the pte)
int fetch(int pid, int vpn, int disk_address)
{
	//this might get tricky if this is a page table?
	int diskAddress = getDiskAddress(pid) + vpn + 1;
	if(vpn == -1) //flag for page table
	{
		diskAddress = getDiskAddress(pid);
		pageTablePresent[pid] = 1;
	}
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
	printf("swapped disk slot %d into frame %d\n", pid, getPageNumber(physical_address));
	return physical_address;
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
		if(pageTableUses[i] < smallestUses)
		{
			smallestUses = pageTableUses[i];
			smallestAddress = registers[i];
			diskOffset = 0;
			pid_of_eviction = i;
			break;
		}
	}
	
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
						
						evicted_page = pageTableEntry;
						diskOffset = vpn_of_eviction + 1; //j is the "vpn"	
					}
				}
			}
		}
	}

	pageToBeSwapped = smallestAddress;
	
	//this may be redundant
	for(int i = 0; i < 4; i++)
	{
		if(registers[i] == pageToBeSwapped) //make sure that we put the page table at index 0
		{
			//we are swapping out a page table so we should stick it at index 0
			//for that process on disk
			diskOffset = 0;	
		}	
	}
	if(diskOffset > 0)
	{
		evicted_page->present = 0;
		int index = getDiskAddress(pid_of_eviction) + diskOffset;
		memcpy((disk + (16*index)), (memory + pageToBeSwapped), 16);
		printf("Swapped frame %d to disk at swap slot %d\n", getPageNumber(pageToBeSwapped), pid_of_eviction);
		evicted_page->address = 16*index;
	
	}else
	{
		//evicting a page table, so
		printf("swapping page table of process %d to disk. \n", pid_of_eviction);
		pageTablePresent[pid_of_eviction] = 0;
		pageTableUses[pid_of_eviction] = 0;
		int index = getDiskAddress(pid_of_eviction);
		registers[pid_of_eviction] = 16*index;
		memcpy((disk + (16*index)), (memory + pageToBeSwapped), 16);
		
	}
	return pageToBeSwapped; //return the address of the evicted page
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
		
		char * pidString = strtok(input, ",");
		char * endptr;
		if(pidString == NULL)
		{
			printf("Error. Invalid input.\n");
			continue;
		}
		pid = strtol(pidString, &endptr, 10);
		if(*endptr != '\0' || pid > 3 || pid < 0)
		{
			printf("Error. Invalid input.\n");
			continue;
		}
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
		commandsEntered++;
		//printf("PID: %d instruction: %s value: %d \n", pid, instruction, value);
		//printf("virtual address: %d \n", virtual_address);
		//first 2 bits of virtual address is the VPN
		int offset_mask = 15;
		int vpn_mask = 48;
		int vpn = (virtual_address & vpn_mask) >> 4;
		int offset = virtual_address & offset_mask;

		//printf("vpn: %d \n", vpn);
		//printf("offset: %d \n", offset);

		//gets the page table address, if there isn't a page table, it allocates one.
		int page_table_address = getPageTable(pid);
		
		if( strcmp(instruction, "allocate") == 0 )
		{
			printf("Allocating...\n");
			//check freelist, get a page, add a pte for that vpn.
			
			int page_table_entry_index = page_table_address + vpn;
			
			
			struct pte * pageTableEntry = (struct pte *)memory + page_table_entry_index;
			
			/*
			if(memory + page_table_entry_index) > page_table_addres && (memory + page_table_entry_index < page_table_entry_index) + 16)
			*/ //Maybe something like this? ^^^
			if(pageTableEntry->address != 0)
			{
				if(pageTableEntry->write == 1)
				{
					printf("Error: virtual page %d is already mapped with rw_bit=1\n", vpn);
				}else
				{
					printf("Updating permissions for virtual page %d (frame %d) \n", vpn, getPageNumber(pageTableEntry->address));
					pageTableEntry->write = value;
				}
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
					
				}
				int frameNumber = getPageNumber(address);
				pageTableEntry->address = address;
				pageTableEntry->write = value;
				pageTableEntry->present = 1;
				pageTableEntry->useCounter = commandsEntered;
				printf("Mapped virtual address %d into physical frame %d\n", virtual_address,  frameNumber);			
			}
				
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
