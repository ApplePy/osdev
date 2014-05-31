#include "FAT.h"

#ifdef _MSC_VER
//THE FOLLOWING CODE IS FOR TESTING ON A DISK IMAGE AND IS NOT TO BE USED IN PRODUCTION!

// int13h return codes supplied from: http://home.teleport.com/~brainy/interrupts.htm

int int13h_read(unsigned long sector, unsigned char num) //a testing implementation of the real function to allow for testing on a disk contained in a file, rather than a real disk.
{
	// Bounds check; only read sectors on the booted partition
	if (sector + num > part_length)
		return -1;

	// Sanity check; BIOS int13h can only read up to 0x7F blocks
	if (num > 0x7F)
		return -1;
	
	//close file if already open; close it if it is
	if (fakeDisk != NULL)
	{
		if (fclose(fakeDisk) == 0)
		{
			if (fakeDisk != NULL)
				fakeDisk = NULL;
		}
		else
		{
			fprintf(stderr, "Error closing testing file.\n");
			return EOF;
		}
	}

	//open file (should be guarenteed to be closed and NULL)
	if (fakeDisk == NULL)
	{
		if ((fakeDisk = fopen("C:\\Users\\Darryl\\Desktop\\part.img", "r")) == NULL)
		{
			fprintf(stderr, "Error opening testing file.\n");
			return -5;
		}
	}

	//seek to particular part of drive
	if (fseek(fakeDisk, sector * bootsect.bytes_per_sector, SEEK_SET) == -1)
	{
		fprintf(stderr, "Error seeking.\n");
		return 0x40;
	}

	//read in contents
	if (fread(DISK_READ_LOCATION, bootsect.bytes_per_sector, num, fakeDisk) != num)
	{
		fprintf(stderr, "Error reading file.\n");
		return 0x04;
	}

	//close file after read finished.
	if (fclose(fakeDisk) == 0)
	{
		if (fakeDisk != NULL)
			fakeDisk = NULL;
	}
	else
	{
		fprintf(stderr, "Error closing testing file.\n");
		return EOF;
	}

	return 0;
}

int int13h_read_o(unsigned long sector, unsigned char num, unsigned long memoffset)
{
	// Bounds check; only read sectors on the booted partition
	if (sector + num > part_length)
		return -1;

	//Size check; only read blocks if the resulting size read will still be between 0x40000 and 0x80000
	if (memoffset + num * 512 > (0x80000 - 0x40000))
		return -1;

	// Sanity check; BIOS int13h can only read up to 0x7F blocks
	if (num > 0x7F)
		return -1;


	//close file if already open; close it if it is
	if (fakeDisk != NULL)
	{
		if (fclose(fakeDisk) == 0)
		{
			if (fakeDisk != NULL)
				fakeDisk = NULL;
		}
		else
		{
			fprintf(stderr, "Error closing testing file.\n");
			return EOF;
		}
	}

	//open file (should be guarenteed to be closed and NULL)
	if (fakeDisk == NULL)
	{
		if ((fakeDisk = fopen("C:\\Users\\Darryl\\Desktop\\part.img", "r")) == NULL)
		{
			fprintf(stderr, "Error opening testing file.\n");
			return -5;
		}
	}

	//seek to particular part of drive
	if (fseek(fakeDisk, sector * bootsect.bytes_per_sector, SEEK_SET) == -1)
	{
		fprintf(stderr, "Error seeking.\n");
		return 0x40;
	}

	//read in contents
	if (fread(DISK_READ_LOCATION + memoffset, bootsect.bytes_per_sector, num, fakeDisk) != num)
	{
		fprintf(stderr, "Error reading file.\n");
		return 0x04;
	}

	//close file after read finished.
	if (fclose(fakeDisk) == 0)
	{
		if (fakeDisk != NULL)
			fakeDisk = NULL;
	}
	else
	{
		fprintf(stderr, "Error closing testing file.\n");
		return EOF;
	}

	return 0;
}

int int13h_write(unsigned long sector, unsigned char num) //a testing implementation of the real function to allow for testing on a disk contained in a file, rather than a real disk.
{

	// Bounds check; only write sectors on the booted partition
	if (sector + num > part_length)
		return -1;

	// Sanity check; BIOS int13h can only write up to 0x7F blocks
	if (num > 0x7F)
		return -1;

	//close file if already open; close it if it is
	if (fakeDisk != NULL)
	{
		if (fclose(fakeDisk) == 0)
		{
			if (fakeDisk != NULL)
				fakeDisk = NULL;
		}
		else
		{
			fprintf(stderr, "Error closing testing file.\n");
			return EOF;
		}
	}

	//open file (should be guarenteed to be closed and NULL)
	if (fakeDisk == NULL)
	{
		if ((fakeDisk = fopen("C:\\Users\\Darryl\\Desktop\\part.img", "r+")) == NULL)
		{
			fprintf(stderr, "Error opening testing file.\n");
			return -5;
		}
	}

	//seek to particular part of drive
	if (fseek(fakeDisk, sector * bootsect.bytes_per_sector, SEEK_SET) == -1)
	{
		fprintf(stderr, "Error seeking.\n");
		return 0x40;
	}

	//write in contents
	if (fwrite(DISK_WRITE_LOCATION, bootsect.bytes_per_sector, num, fakeDisk) != num)
	{
		fprintf(stderr, "Error writing file.\n");
		return 0xCC;
	}

	//close file after write finished.
	if (fclose(fakeDisk) == 0)
	{
		if (fakeDisk != NULL)
			fakeDisk = NULL;
	}
	else
	{
		fprintf(stderr, "Error closing testing file.\n");
		return EOF;
	}

	return 0;
}

int int13h_write_o(unsigned long sector, unsigned char num, unsigned long memoffset)
{
	// Bounds check; only write sectors on the booted partition
	if (sector + num > part_length)
		return -1;

	//Size check; only write blocks if the resulting size written will still be between 0x40000 and 0x80000
	if (memoffset + num * 512 > (0x80000 - 0x40000))
		return -1;

	// Sanity check; BIOS int13h can only write up to 0x7F blocks
	if (num > 0x7F)
		return -1;


	//close file if already open; close it if it is
	if (fakeDisk != NULL)
	{
		if (fclose(fakeDisk) == 0)
		{
			if (fakeDisk != NULL)
				fakeDisk = NULL;
		}
		else
		{
			fprintf(stderr, "Error closing testing file.\n");
			return EOF;
		}
	}

	//open file (should be guarenteed to be closed and NULL)
	if (fakeDisk == NULL)
	{
		if ((fakeDisk = fopen("C:\\Users\\Darryl\\Desktop\\part.img", "r+")) == NULL)
		{
			fprintf(stderr, "Error opening testing file.\n");
			return -5;
		}
	}

	//seek to particular part of drive
	if (fseek(fakeDisk, sector * bootsect.bytes_per_sector, SEEK_SET) == -1)
	{
		fprintf(stderr, "Error seeking.\n");
		return 0x40;
	}

	//write in contents
	if (fread(DISK_WRITE_LOCATION + memoffset, bootsect.bytes_per_sector, num, fakeDisk) != num)
	{
		fprintf(stderr, "Error writing file.\n");
		return 0xCC;
	}

	//close file after write finished.
	if (fclose(fakeDisk) == 0)
	{
		if (fakeDisk != NULL)
			fakeDisk = NULL;
	}
	else
	{
		fprintf(stderr, "Error closing testing file.\n");
		return EOF;
	}

	return 0;
}

void printss(char *s)
{
	printf(s);
}

void printsss(char *s, int n)
{
	unsigned int iter = 0;
	for (iter = 0; iter < n; iter++)
	{
		printf(&s[iter]);
	}
}

void printhex(unsigned long num, int digits)
{
	int i, buf;

	for (i = 0; i < digits; i++) {

		buf = (num >> ((digits - i - 1) * 4)) & 0xF;
		if (buf < 10)
			buf += '0';
		else
			buf += 'A' - 10;
		printss((char*)buf);
	}
}
#endif

//Initializes struct "bootsect" to store critical data from the boot sector of the volume
int FATInitialize()
{
	//reads the first sector of the FAT
	if (int13h_read(0, 1) != 0)
	{
		printss("Function FATInitialize: Error reading the first sector of FAT!\n");
		return -1;
	}

	fat_BS_t* bootstruct = (fat_BS_t*)DISK_READ_LOCATION;

	total_clusters = bootstruct->total_sectors_16 / bootstruct->sectors_per_cluster;

	if (total_clusters == 0) //there's more than 65535 sectors, find the real number
	{
		total_clusters = bootstruct->total_sectors_32 / bootstruct->sectors_per_cluster;
	}

	if (total_clusters < 4085)
	{
		fat_type = 12;
		first_data_sector = bootstruct->reserved_sector_count + bootstruct->table_count * bootstruct->table_size_16 + (bootstruct->root_entry_count * 32 + bootstruct->bytes_per_sector - 1) / bootstruct->bytes_per_sector; //Explanation: the first data sector is after the reserved sectors, the FAT table sectors, and the root directory sectors. The size of the root directory is found by multiplying the amount of root entries by the size of each entry (32 bytes), then adding bytes per sector - 1 so that when divided by bytes per sector, the calculation starts at 1 sector, not zero, while also maintaining 1 sector when there are exactly 512 bytes worth of entries.
	}
	else
	{
		if (total_clusters < 65525)
		{
			fat_type = 16;
			first_data_sector = bootstruct->reserved_sector_count + bootstruct->table_count * bootstruct->table_size_16 + (bootstruct->root_entry_count * 32 + bootstruct->bytes_per_sector - 1) / bootstruct->bytes_per_sector; //Explanation: the first data sector is after the reserved sectors, the FAT table sectors, and the root directory sectors. The size of the root directory is found by multiplying the amount of root entries by the size of each entry (32 bytes), then adding bytes per sector - 1 so that when divided by bytes per sector, the calculation starts at 1 sector, not zero, while also maintaining 1 sector when there are exactly 512 bytes worth of entries.
		}
		else
		{
			fat_type = 32;
			first_data_sector = bootstruct->reserved_sector_count + bootstruct->table_count * ((fat_extBS_32_t*)(bootstruct->extended_section))->table_size_32;
		}
	}

	if (memcpy(&bootsect, bootstruct, 512) != &bootsect)
	{
		printss("Function FATInitialize: Not sure what values memcpy returns. Continuing execution...\n");
	}

	first_fat_sector = bootstruct->reserved_sector_count;

	return 0;
}

//read FAT table
//This function deals in absolute data clusters
int FATRead(unsigned int clusterNum)
{
	if (fat_type == 32)
	{
		unsigned int cluster_size = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
		unsigned char FAT_table[32 * 1024] = { '\0' }; //Takes into consideration the largest standard cluster size (32kB) since arrays can't be dynamically allocated without "new" :/
		unsigned int fat_offset = clusterNum * 4;
		unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
		unsigned int ent_offset = fat_offset % cluster_size;

		//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
		if (int13h_read(fat_sector, 1) != 0)
		{
			printss("Function FATRead: Could not read sector that contains FAT32 table entry needed.\n");
			return -1;
		}
		if (memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector) != &FAT_table);
		{
			printss("Function FATRead: Not sure what values memcpy returns. Continuing execution...\n");
		}

		//remember to ignore the high 4 bits.
		unsigned int table_value = *(unsigned int*)&FAT_table[ent_offset] & 0x0FFFFFFF;

		//the variable "table_value" now has the information you need about the next cluster in the chain.
		return table_value;
	}
	else if (fat_type == 16)
	{
		unsigned int cluster_size = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
		unsigned char FAT_table[32 * 1024]; //Takes into consideration the largest standard cluster size (32kB) since arrays can't be dynamically allocated without "new" :/
		unsigned int fat_offset = clusterNum * 2;
		unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
		unsigned int ent_offset = fat_offset % cluster_size;

		//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
		if (int13h_read(fat_sector, 1) != 0)
		{
			printss("Function FATRead: Could not read sector that contains FAT16 table entry needed.\n");
			return -1;
		}
		if (memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector) != &FAT_table);
		{
			printss("Function FATRead: Not sure what values memcpy returns. Continuing execution...\n");
		}

		unsigned short table_value = *(unsigned short*)&FAT_table[ent_offset];

		//the variable "table_value" now has the information you need about the next cluster in the chain.
		return table_value;
	}
	/*else if (fat_type == 12)
	{
	unsigned int cluster_size = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
	unsigned char FAT_table[32 * 1024]; //Takes into consideration the largest standard cluster size (32kB) since arrays can't be dynamically allocated without "new" :/
	unsigned int fat_offset = clusterNum + (clusterNum / 2);// multiply by 1.5
	unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
	unsigned int ent_offset = fat_offset % cluster_size;

	//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".

	unsigned short table_value = *(unsigned short*)&FAT_table[ent_offset];

	if (current_cluster & 0x0001)
	table_value = table_value >> 4;
	else
	table_value = table_value & 0x0FFF;
	}*/
	else
	{
		printss("Function FATRead: Invalid fat_type value. The value was (in hex): ");
		printhex(fat_type, 8);
		printss("\n");
		return -1;
	}
}

int FATWrite(unsigned int clusterNum, unsigned int clusterVal)
{
	if (fat_type == 32)
	{
		unsigned int cluster_size = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
		unsigned char FAT_table[32 * 1024] = { '\0' }; //Takes into consideration the largest standard cluster size (32kB) since arrays can't be dynamically allocated without "new" :/
		unsigned int fat_offset = clusterNum * 4;
		unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
		unsigned int ent_offset = fat_offset % cluster_size;

		//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
		if (int13h_read(fat_sector, 1) != 0)
		{
			printss("Function FATWrite: Could not read sector that contains FAT32 table entry needed.\n");
			return -1;
		}
		if (memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector) != &FAT_table);
		{
			printss("Function FATWrite: Not sure what values memcpy returns. Continuing execution...\n");
		}

		//copy clusterVal into FAT_table
		*(unsigned int*)&FAT_table[ent_offset] = clusterVal;

		//send modified FAT_table back to disk
		if (memcpy((char*)DISK_WRITE_LOCATION, &FAT_table, bootsect.bytes_per_sector) != DISK_WRITE_LOCATION)
		{
			printss("Function FATWrite: Not sure what values memcpy returns. Continuing execution...\n");
		}
		if (int13h_write(fat_sector, 1) != 0)
		{
			printss("Function FATWrite: Could not write new FAT32 cluster number to sector.\n");
			return -1;
		}

		return 0;
	}
	else if (fat_type == 16)
	{
		unsigned int cluster_size = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
		unsigned char FAT_table[32 * 1024]; //Takes into consideration the largest standard cluster size (32kB) since arrays can't be dynamically allocated without "new" :/
		unsigned int fat_offset = clusterNum * 2;
		unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
		unsigned int ent_offset = fat_offset % cluster_size;

		//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
		if (int13h_read(fat_sector, 1) != 0)
		{
			printss("Function FATWrite: Could not read sector that contains FAT16 table entry needed.\n");
			return -1;
		}
		if (memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector) != &FAT_table);
		{
			printss("Function FATWrite: Not sure what values memcpy returns. Continuing execution...\n");
		}

		//copy clusterVal into FAT_table
		*(unsigned short*)&FAT_table[ent_offset] = (unsigned short)clusterVal;

		//send modified FAT_table back to disk
		if (memcpy((char*)DISK_WRITE_LOCATION, &FAT_table, bootsect.bytes_per_sector) != DISK_WRITE_LOCATION)
		{
			printss("Function FATWrite: Not sure what values memcpy returns. Continuing execution...\n");
		}
		if (int13h_write(fat_sector, 1) != 0)
		{
			printss("Function FATWrite: Could not write new FAT16 cluster number to sector.\n");
			return -1;
		}

		return 0;
	}
	/*else if (fat_type == 12)
	{
		NULL; //Not Implemented!
	}*/
	else
	{
		printss("Function FATWrite: Invalid fat_type value. The value was (in hex): ");
		printhex(fat_type, 8);
		printss("\n");
		return -1;
	}
}

unsigned int allocateFreeFAT()
{
	unsigned int free_cluster = BAD_CLUSTER_12;
	unsigned int bad_cluster = BAD_CLUSTER_12;
	unsigned int end_cluster = BAD_CLUSTER_12;

	if (fat_type == 32)
	{
		free_cluster = FREE_CLUSTER_32;
		bad_cluster = BAD_CLUSTER_32;
		end_cluster = END_CLUSTER_32;
	}
	else if (fat_type == 16)
	{
		free_cluster = FREE_CLUSTER_16;
		bad_cluster = BAD_CLUSTER_16;
		end_cluster = END_CLUSTER_16;
	}
	else if (fat_type == 12)
	{
		free_cluster = FREE_CLUSTER_12;
		bad_cluster = BAD_CLUSTER_12;
		end_cluster = END_CLUSTER_12;
	}
	else
	{
		printss("Function allocateFreeFAT: fat_type is not valid!\n");
		return BAD_CLUSTER_12;
	}

	unsigned int cluster = 2;
	unsigned int clusterStatus = free_cluster;

	while (cluster < total_clusters)
	{
		clusterStatus = FATRead(cluster);
		if (clusterStatus == free_cluster)
		{
			if (FATWrite(cluster, end_cluster) == 0)
				return cluster;
			else
			{
				printss("Function allocateFreeFAT: Error occurred with FATWrite, aborting operations...\n");
				return bad_cluster;
			}
		}
		else if (clusterStatus < 0)
		{
			printss("Function allocateFreeFAT: Error occurred with FATRead, aborting operations...\n");
			return bad_cluster;
		}

		cluster++; //cluster is taken, check the next one
	}
	return bad_cluster;
}

//Reads one cluster and dumps it to DISK_READ_LOCATION, offset "cluster_size" number of bytes from DISK_READ_LOCATION
//This function deals in absolute data clusters
int clusterRead(unsigned int clusterNum, unsigned int clusterOffset)
{
	unsigned int start_sect = (clusterNum - 2) * (unsigned short)bootsect.sectors_per_cluster + first_data_sector; //Explanation: Since the root cluster is cluster 2, but data starts at first_data_sector, subtract 2 to get the proper cluster offset from zero.

	if (int13h_read_o(start_sect, (unsigned short)bootsect.sectors_per_cluster, clusterOffset * (unsigned short)bootsect.sectors_per_cluster * (unsigned short)bootsect.bytes_per_sector) != 0)
	{
		printss("Function clusterRead: An error occured with int13h_read_o, the area in DISK_READ_LOCATION + 0x");
		printhex(clusterOffset, 8);
		printss(" is now in an unknown state.\n");
		return -1;
	}
	else
		return 0;
}

//Deals in absolute clusters
//contentsToWrite: contains a pointer to the data to be written to disk
//contentSize: contains how big contentsToWrite's data is (in bytes)
//contentBuffOffset: sets how far offset from DISK_WRITE_LOCATION to place the data from contentsToWrite in preparation for writing to disk (in clusters)
//clusterNum: Specifies the on-disk cluster to write the data to
int clusterWrite(void* contentsToWrite, unsigned int contentSize, unsigned int contentBuffOffset, unsigned int clusterNum)
{
	unsigned int byteOffset = contentBuffOffset * (unsigned short)bootsect.sectors_per_cluster * (unsigned short)bootsect.bytes_per_sector; //converts cluster memory offset into bytes

	//copy contents to be written to disk to the memory write location
	if (memcpy((char*)DISK_WRITE_LOCATION + byteOffset, contentsToWrite, contentSize) != (DISK_WRITE_LOCATION + byteOffset))
	{
		printss("Function clusterWrite: Not sure what values memcpy returns. Continuing execution...\n");
	}

	unsigned int start_sect = (clusterNum - 2) * (unsigned short)bootsect.sectors_per_cluster + first_data_sector; //Explanation: Since the root cluster is cluster 2, but data starts at first_data_sector, subtract 2 to get the proper cluster offset from zero.

	if (int13h_write_o(start_sect, (unsigned short)bootsect.sectors_per_cluster, byteOffset) != 0)
	{
		printss("Function clusterWrite: An error occured with int13h_write_o, the area in sector ");
		printhex(start_sect, 8);
		printss(" through to sector ");
		printhex(((unsigned short)bootsect.sectors_per_cluster) + start_sect, 2);
		printss(" are now in an unknown state.\n");
		return -1;
	}
	else
		return 0;
}

//. and .. entries not supported yet!

//receives the cluster to read for a directory and the requested file, and will iterate through the directory's clusters - returning the entry for the searched file/subfolder, or no file/subfolder
//return value holds success or failure code, file holds directory entry if file is found (can be NULL)
//entryOffset points to where the directory entry was found in sizeof(directory_entry_t) (can be NULL)
//returns: -1 is a general error, -2 is a "not found" error
int directorySearch(const char* filepart, const unsigned int cluster, directory_entry_t* file, unsigned int* entryOffset)
{
	char searchName[13] = { '\0' };
	strcpy(searchName, filepart);
	convertToFATFormat(searchName);

	//read cluster of the directory/subdirectory
	if (clusterRead(cluster, 0) != 0)
	{
		printss("Function directorySearch: clusterRead encountered an error. Aborting...\n");
		return -1;
	}
	directory_entry_t* file_metadata = (directory_entry_t*)DISK_READ_LOCATION;
	unsigned int meta_pointer_iterator_count = 0; //holds how many directories have been looked at

	//iterate through clusters, checking for a file name match
	while (1)
	{
		if (file_metadata->file_name[0] == ENTRY_END) //end of directory entries; searching can stop now
			break;
		else if (file_metadata->file_name[0] == ENTRY_FREE || strncmp((char*)file_metadata->file_name, searchName, 11) != 0) //if the file doesn't match 
		{
			if (meta_pointer_iterator_count < bootsect.bytes_per_sector * bootsect.sectors_per_cluster / sizeof(directory_entry_t) - 1) //if the pointer hasn't iterated outside of what that cluster can hold
			{
				file_metadata++;
				meta_pointer_iterator_count++;
			}
			else
			{
				unsigned int next_cluster = FATRead(cluster);

				if ((next_cluster >= END_CLUSTER_32 && fat_type == 32) || (next_cluster >= END_CLUSTER_16 && fat_type == 16) || (next_cluster >= END_CLUSTER_12 && fat_type == 12))
					break;
				else if (next_cluster < 0)
				{
					printss("Function directorySearch: FATRead encountered an error. Aborting...\n");
					return -1;
				}
				else
					return directorySearch(filepart, next_cluster, file, entryOffset); //search next cluster
			}
		}
		else
		{
			if (file != NULL)
			{
				if (memcpy(file, file_metadata, sizeof(directory_entry_t)) != file); //copy found data to file
				{
					printss("Function directorySearch: Not sure what values memcpy returns. Continuing execution...\n");
				}
			}

			if (entryOffset != NULL)
				*entryOffset = meta_pointer_iterator_count;

			return 0;
		}
	}

	return -2; //nothing found, return error.
}

//. and .. entries not supported yet!

//pass in the cluster to write the directory to and the directory struct to write.
//struct should only have a file name, attributes, and size. the rest will be filled in automatically
int directoryAdd(const unsigned int cluster, directory_entry_t* file)
{
	//read cluster of the directory/subdirectory
	if (clusterRead(cluster, 0) != 0)
	{
		printss("Function directoryAdd: clusterRead encountered an error. Aborting...\n");
		return -1;
	}
	directory_entry_t* file_metadata = (directory_entry_t*)DISK_READ_LOCATION;
	unsigned int meta_pointer_iterator_count = 0; //holds how many directories have been looked at

	while (1)
	{
		if (file_metadata->file_name[0] != (char)ENTRY_FREE && file_metadata->file_name[0] != (char)ENTRY_END) //if the file directory slot isn't free
		{
			if (meta_pointer_iterator_count < bootsect.bytes_per_sector * bootsect.sectors_per_cluster / sizeof(directory_entry_t) - 1) //if the pointer hasn't iterated outside of what that cluster can hold
			{
				file_metadata++;
				meta_pointer_iterator_count++;
			}
			else
			{
				unsigned int next_cluster = FATRead(cluster);

				if ((next_cluster >= END_CLUSTER_32 && fat_type == 32) || (next_cluster >= END_CLUSTER_16 && fat_type == 16) || (next_cluster >= END_CLUSTER_12 && fat_type == 12)) //no free spaces left in the directory cluster, and no more clusters to search. Allocate a new one.
				{
					next_cluster = allocateFreeFAT();

					if ((next_cluster == BAD_CLUSTER_32 && fat_type == 32) || (next_cluster == BAD_CLUSTER_16 && fat_type == 16) || (next_cluster == BAD_CLUSTER_12 && fat_type == 12)) //allocation unsuccessful
					{
						printss("Function directoryAdd: allocation of new cluster failed. Aborting...\n");
						break;
					}

					//write the new cluster number to the previous cluster's FAT
					if (FATWrite(cluster, next_cluster) != 0)
					{
						printss("Function directoryAdd: extension of the cluster chain with new cluster failed. Aborting...\n");
						break;
					}
				}

				return directoryAdd(next_cluster, file);//search next cluster
			}
		}
		else
		{
			convertToFATFormat((char*)file->file_name); //convert name to FAT format before saving
			file->creation_date = CurrentDate();
			file->creation_time = CurrentTime();
			file->creation_time_tenths = CurrentTimeTenths();
			file->last_accessed = file->creation_date;
			file->last_modification_date = file->creation_date;
			file->last_modification_time = file->creation_time;

			//allocate new cluster for new file
			unsigned int new_cluster = allocateFreeFAT();

			if ((new_cluster == BAD_CLUSTER_32 && fat_type == 32) || (new_cluster == BAD_CLUSTER_16 && fat_type ==16) || (new_cluster == BAD_CLUSTER_12 && fat_type == 12)) //allocation unsuccessful
			{
				printss("Function directoryAdd: allocation of new cluster failed. Aborting...\n");
				break;
			}

			file->low_bits = GET_LOW_BITS(new_cluster);
			file->high_bits = GET_HIGH_BITS(new_cluster);

			//copy data to empty location
			if (memcpy(file_metadata, file, sizeof(directory_entry_t)) != file_metadata)
			{
				printss("Function directorySearch: Not sure what values memcpy returns. Continuing execution...\n");
			}
			if (clusterWrite(DISK_READ_LOCATION, bootsect.bytes_per_sector * bootsect.sectors_per_cluster, 0, cluster) != 0)
			{
				printss("Function clusterWrite: Writing new directory entry failed. Aborting...\n");
				break;
			}
			return 0;
		}
	}

	return -1; //return error.
}

//retrieves a specified file from the File System (readInOffset is in clusters)
//Returns: -1 is general error, -2 is directory not found, -3 is path specified is a directory instead of a file
int getFile(const char* filePath, char** fileContents, directory_entry_t* fileMeta, unsigned int readInOffset)
{
	char fileNamePart[256]; //holds the part of the path to be searched

	unsigned short start = 3; //starting at 3 to skip the "C:\" bit
	unsigned int active_cluster;
	if (fat_type == 32)
		active_cluster = ((fat_extBS_32_t*)bootsect.extended_section)->root_cluster; //holds the cluster to be searched for directory entries related to the path
	else
	{
		printss("Function getFile: FAT16 and FAT12 are not supported!\n");
		return -1;
	}

	directory_entry_t file_info; //holds found directory info

	//starting at 3 to skip the "C:\" bit
	unsigned int iterator = 3;
	for (iterator = 3; filePath[iterator - 1] != '\0'; iterator++)
	{
		if (filePath[iterator] == '\\' || filePath[iterator] == '\0')
		{
			//clean out fileNamePart before copy
			if (memset(fileNamePart, '\0', 256) != fileNamePart)
			{
				printss("Function getFile: Not sure what values memset returns. Continuing execution...\n");
			}
			//hacked-together strcpy derivative...
			if (memcpy(fileNamePart, filePath + start, iterator - start) != fileNamePart)
			{
				printss("Function getFile: Not sure what values memcpy returns. Continuing execution...\n");
			}

			int retVal = directorySearch(fileNamePart, active_cluster, &file_info, NULL); //go looking for a directory in the specified cluster with the specified name

			if (retVal == -2) //no directory matching found
				return -2;
			else if (retVal == -1) //error occured
			{
				printss("Function getFile: An error occurred in directorySearch. Aborting...\n");
				return retVal;
			}

			start = iterator + 1;
			active_cluster = GET_CLUSTER_FROM_ENTRY(file_info); //shift the high bits into their appropriate spots, and OR with low_bits (could also add, I think) in prep for next search
		}
	}

	*fileMeta = file_info; //copy fileinfo over

	if ((file_info.attributes & FILE_DIRECTORY) != FILE_DIRECTORY) //if final directory listing found isn't a directory
	{
		if (readInOffset < 1 || (readInOffset * (unsigned short)bootsect.bytes_per_sector * (unsigned short)bootsect.sectors_per_cluster) + file_info.file_size > 262144) //prevent offsets that extend into FATRead's working range or outside the allocated BIOS int13h space
			return -3; //you cannot have an offset below 1, nor can you read in more than 256kB

		unsigned int cluster = GET_CLUSTER_FROM_ENTRY(file_info); //initialize file read-in with first cluster of file
		unsigned int clusterReadCount = 0;
		while (cluster < END_CLUSTER_32)
		{
			clusterRead(cluster, clusterReadCount + readInOffset); //Always offset by at least one, so any file operations happening exactly at DISK_READ_LOCATION (e.g. FAT Table lookups) don't overwrite the data (this is essentially backwards compatibility with previously written code)
			clusterReadCount++;
			cluster = FATRead(cluster);
			if (cluster == BAD_CLUSTER_32)
			{
				printss("Function getFile: the cluster chain is corrupted with a bad cluster. Aborting...\n");
				return -1;
			}
			else if (cluster == -1) //THIS WILL *NOT* WORK!!!! SIGN PROBLEM
			{
				printss("Function getFile: an error occurred in FATRead. Aborting...\n");
				return -1;
			}
		}

		*fileContents = DISK_READ_LOCATION + (unsigned short)bootsect.sectors_per_cluster * (unsigned short)bootsect.bytes_per_sector * readInOffset; //return a pointer in the BIOS read-in space where the file is.

		return 0; //file successfully found
	}
	else
		return -3; //the path specified was a directory
}

//writes a new file to the file system
//filepath: specifies the path to where the file will be written
//filecontents: contains the char array to the data that will be written
//fileMeta: contains the metadata that will be written. struct should only have a file name, attributes, and size. the rest will be filled in automatically
//returns: -1 is general error, -2 indicates a bad path, -3 indicates file with same name already exists, -4 indicates file size error
int putFile(const char* filePath, char** fileContents, directory_entry_t* fileMeta)
{
	char fileNamePart[256]; //holds the part of the path to be searched

	unsigned short start = 3; //starting at 3 to skip the "C:\" bit
	unsigned int active_cluster; //holds the cluster to be searched for directory entries related to the path
	if (fat_type == 32)
	 active_cluster = ((fat_extBS_32_t*)bootsect.extended_section)->root_cluster;
	else
	{
		printss("Function putFile: FAT16 and FAT12 are not supported!\n");
		return -1;
	}

	directory_entry_t file_info; //holds found directory info

	//starting at 3 to skip the "C:\" bit
	unsigned int iterator = 3;
	for (iterator = 3; filePath[iterator - 1] != '\0'; iterator++)
	{
		if (filePath[iterator] == '\\' || filePath[iterator] == '\0')
		{
			//clean out fileNamePart before copy
			if (memset(fileNamePart, '\0', 256) != fileNamePart)
			{
				printss("Function putFile: Not sure what values memset returns. Continuing execution...\n");
			}
			//hacked-together strcpy derivative...
			if (memcpy(fileNamePart, filePath + start, iterator - start) != fileNamePart)
			{
				printss("Function putFile: Not sure what values memcpy returns. Continuing execution...\n");
			}

			int retVal = directorySearch(fileNamePart, active_cluster, &file_info, NULL); //go looking for a directory in the specified cluster with the specified name

			if (retVal == -2) //no directory matching found
				return -2;
			else if (retVal == -1) //error occured
			{
				printss("Function putFile: An error occurred in directorySearch. Aborting...\n");
				return retVal;
			}

			start = iterator + 1;
			active_cluster = GET_CLUSTER_FROM_ENTRY(file_info); //shift the high bits into their appropriate spots, and OR with low_bits (could also add, I think) in prep for next search
		}
	}

	//directory to receive the file is now found, and its cluster is stored in active_cluster. Search the directory to ensure the specified file name is not already in use
	int retVal = directorySearch(fileMeta->file_name, active_cluster, NULL, NULL);
	if (retVal == -1)
	{
		printss("Function putFile: directorySearch encountered an error. Aborting...\n");
		return -1;
	}
	else if (retVal != -2)
	{
		printss("Function putFile: a file matching the name given already exists. Aborting...\n");
		return -3;
	}

	if ((file_info.attributes & FILE_DIRECTORY) == FILE_DIRECTORY) //if final directory listing found is a directory
	{
		if (directoryAdd(active_cluster, fileMeta) != 0)
		{
			printss("Function putFile: directoryAdd encountered an error. Aborting...\n");
			return -1;
		}

		//now filling file_info with the information of the file directory entry
		retVal = directorySearch(fileMeta->file_name, active_cluster, &file_info, NULL);
		if (retVal == -2)
		{
			printss("Function putFile: directoryAdd did not properly write the new file's entry to disk. Aborting...\n");
			return -2;
		}
		else if (retVal != 0)
		{
			printss("Function putFile: directorySearch has encountered an error. Aborting...\n");
			return -1;
		}

		active_cluster = GET_CLUSTER_FROM_ENTRY(file_info);
		unsigned int dataLeftToWrite = file_info.file_size;
		while (dataLeftToWrite > 0)
		{
			unsigned int dataWrite = 0;
			if (dataLeftToWrite >= bootsect.bytes_per_sector * bootsect.sectors_per_cluster)
				dataWrite = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
			else
				dataWrite = dataLeftToWrite;

			//Always offset by at least one, so any file operations happening exactly at DISK_READ_LOCATION (e.g. FAT Table lookups) don't overwrite the data (this is essentially backwards compatibility with previously written code)
			if (clusterWrite(*fileContents + (file_info.file_size - dataLeftToWrite), dataWrite, 1, active_cluster) == 0)
			{
				printss("Function putFile: clusterWrite encountered an error. Aborting...\n");
				return -1;
			}

			dataLeftToWrite -= dataWrite; //subtract the bytes that were just written

			//if there's no data left to write, exit
			if (dataLeftToWrite <= 0)
				break;
			
			//there's more data to write, so allocate new cluster, change fat of current cluster to point to new cluster, and change active cluster to new cluster

			unsigned int new_cluster = allocateFreeFAT();

			if ((new_cluster == BAD_CLUSTER_32 && fat_type == 32) || (new_cluster == BAD_CLUSTER_16 && fat_type == 16) || (new_cluster == BAD_CLUSTER_12 && fat_type == 12)) //allocation error
			{
				printss("Function putFile: allocateFreeFAT encountered an error. Aborting...\n");
				return -1;
			}
			if (FATWrite(active_cluster, new_cluster) != 0)
			{
				printss("Function putFile: FATWrite encountered an error. Aborting...\n");
				return -1;
			}
			active_cluster = new_cluster;
		}

		return 0; //file successfully written
	}
	else
		return -2; //invalid path!
}

//clock hasn't been implemented yet
unsigned short CurrentTime()
{
	return 0;
}

//clock nor date has been implemented yet
unsigned short CurrentDate()
{
	return 0;
}

//clock hasn't been implemented yet
unsigned char CurrentTimeTenths()
{
	return 0;
}

/*-----------------------------------------------------------------------------
ChkSum()
Returns an unsigned byte checksum computed on an unsigned byte
array. The array must be 11 bytes long and is assumed to contain
a name stored in the format of a MS-DOS directory entry.
Passed: pFcbName Pointer to an unsigned byte array assumed to be
11 bytes long.
Returns: Sum An 8-bit unsigned checksum of the array pointed
to by pFcbName.
------------------------------------------------------------------------------*/
unsigned char ChkSum(unsigned char *pFcbName)
{
	short FcbNameLen;
	unsigned char Sum;
	Sum = 0;
	for (FcbNameLen = 11; FcbNameLen != 0; FcbNameLen--) {
		// NOTE: The operation is an unsigned char rotate right
		Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
	}
	return (Sum);
}
/* ChkSum function courtesy of http://staff.washington.edu/dittrich/misc/fatgen103.pdf */

//Notes: Does not consider making short forms of long names, nor support multiple periods in a name.
void convertToFATFormat(char* input)
{
	unsigned int counter = 0;
	unsigned int inputLength = strlen(input);

	while (counter < inputLength) //iterate through input, converting the characters to uppercase
	{
		if ((short)input[counter] >= 97 && (short)input[counter] <= 122)
			input[counter] -= 32;
		counter++;
	}

	char searchName[13] = { '\0' };
	unsigned short dotPos = 0;

	counter = 0;
	while (counter <= 8) //copy all the characters from filepart into searchname until a dot or null character is encountered
	{
		if (input[counter] == '.' || input[counter] == '\0')
		{
			dotPos = counter;
			counter++; //iterate off dot
			break;
		}
		else
		{
			searchName[counter] = input[counter];
			counter++;
		}
	}

	if (counter > 9) //a sanity check in case there was a dot-less 11 character filename
	{
		counter = 8;
		dotPos = 8;
	}

	unsigned short extCount = 8;
	while (extCount < 11) //add the extension to the end, putting spaces where necessary
	{
		if (input[counter] != '\0')
			searchName[extCount] = input[counter];
		else
			searchName[extCount] = ' ';

		counter++;
		extCount++;
	}

	counter = dotPos; //reset counter to position of the dot

	while (counter < 8) //if the dot is within the 8 character limit of the name, iterate through searchName putting in spaces up until the extension
	{
		searchName[counter] = ' ';
		counter++;
	}

	strcpy(input, searchName); //copy results back to input
}

#ifdef _MSC_VER
int main()
{
	FATInitialize();
	FATRead(2);
	directory_entry_t entry;
	directorySearch("example.txt", 2, &entry, NULL);
	char* file;
	getFile("C:\\folder\\foldtest.exp", &file, &entry, 1);

	if (FATRead(5) == FREE_CLUSTER_32)
	{
		FATWrite(5, BAD_CLUSTER_32);
		int retVal = FATRead(5);
		FATWrite(5, FREE_CLUSTER_32);
	}
	else
		NULL;

	if (FATRead(5) == FREE_CLUSTER_32)
	{
		clusterWrite("Testing1", 9, 1, 5);
		FATWrite(5, FREE_CLUSTER_32);
		clusterRead(5, 0);
	}

	int clustAlloCheck[30] = { -1 };

	unsigned int ptr = 0;

	while (ptr < 30)
	{
		clustAlloCheck[ptr] = allocateFreeFAT();
		ptr++;
	}

	ptr = 0;

	while (ptr < 30)
	{
		FATWrite(clustAlloCheck[ptr], FREE_CLUSTER_32);
		ptr++;
	}

	char* contents;
	directory_entry_t folder;

	getFile("C:\\folder", &contents, &folder, 1);

	directory_entry_t newFile;
	newFile.attributes = 0x00;
	strcpy((char*)(newFile.file_name), "ADDTEST.EXP");
	newFile.file_size = 0;

	directoryAdd(GET_CLUSTER_FROM_ENTRY(folder), &newFile);

	getFile("C:\\folder\\addtest.exp", &contents, &folder, 1);


	return 0;
}
#endif


//NOTE: Absolute data clusters start at #2, not 1 or 0, while relative cluster numbers start relative to the first data cluster (2). E.g: The relative cluster number of the first data cluster is 0.
//NOTE: Long file names are UNICODE. Figure out how linux compiler handles unicode.

/*New functions:
clusterWrite() done! //writes clusters to disk
FATWrite() done! //writes FAT values to FAT
directoryAdd() done! //write directory entries to disk, applies the creation/modification time/date, and allocates the first cluster of the file
putFile() //grabs a file to be created, writes its directory information, uses directoryAdd to find a place for it, writes the first cluster's worth of data, then allocates a new cluster and continues until data is all written
mkdir() //a wrapper for directoryAdd() that creates folder entries, and the associated . and .. entries.
allocateFreeFAT() done! //a wrapper for FATRead() that iterates through FAT table looking for a place to allocate.
rmdir()
delFile()
editFile()
directoryEdit() (also can delete)
directoryList()
CurrentTime()
CurrentDate()
CurrentTimeTenths()*/

//Note: find a way to read directly off the disk instead of loading it in memory.