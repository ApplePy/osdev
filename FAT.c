#include "FAT.h"

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

void convertToFATFormat(char* input) //Works! (Does not consider making short forms of long names, nor support multiple periods in a name.)
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

void FATInitialize() //works!
{
	int13h_read(0, 1); //reads the first sector of the FAT

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

	memcpy(&bootsect, bootstruct, 512);

	first_fat_sector = bootstruct->reserved_sector_count;
}

//retrieves a specified file from the File System (readInOffset is in clusters)
int getFile(const char* filePath, char** fileContents, directory_entry_t* fileMeta, unsigned int readInOffset)
{
	char fileNamePart[256] = { '\0' }; //holds the part of the path to be searched
	unsigned short start = 3; //starting at 3 to skip the "C:\" bit
	unsigned int active_cluster = ((fat_extBS_32_t*)bootsect.extended_section)->root_cluster; //holds the cluster to be searched for directory entries related to the path
	directory_entry_t file_info; //holds found directory info

	//starting at 3 to skip the "C:\" bit
	for (unsigned int iterator = 3; filePath[iterator - 1] != '\0'; iterator++)
	{
		if (filePath[iterator] == '\\' || filePath[iterator] == '\0')
		{
			memset(fileNamePart, '\0', 256); //clean out fileNamePart before copy
			memcpy(fileNamePart, filePath + start, iterator - start); //hacked-together strcpy derivative...

			int retVal = directorySearch(fileNamePart, active_cluster, &file_info, NULL); //go looking for a directory in the specified cluster with the specified name

			if (retVal != 0) //no directory matching found
				return -1;

			start = iterator + 1;
			active_cluster = file_info.low_bits | (file_info.high_bits << 16); //shift the high bits into their appropriate spots, and OR with low_bits (could also add, I think) in prep for next search
		}
	}

	*fileMeta = file_info; //copy fileinfo over

	if ((file_info.attributes & FILE_DIRECTORY) != FILE_DIRECTORY) //if final directory listing found isn't a directory
	{
		if (readInOffset < 1 || (readInOffset * (unsigned short)bootsect.bytes_per_sector * (unsigned short)bootsect.sectors_per_cluster) + file_info.file_size > 262144) //prevent offsets that extend into FATRead's working range or outside the allocated BIOS int13h space
			return -3; //you cannot have an offset below 1, nor can you read in more than 256kB

		unsigned int cluster = file_info.low_bits | (file_info.high_bits << 16); //initialize file read-in with first cluster of file
		unsigned int clusterReadCount = 0;
		while (cluster < END_CLUSTER_32)
		{
			clusterRead(cluster, clusterReadCount + readInOffset); //Always offset by at least one, so any file operations happening exactly at DISK_READ_LOCATION (e.g. FAT Table lookups) don't overwrite the data (this is essentially backwards compatibility with previously written code)
			clusterReadCount++;
			cluster = FATRead(cluster);
		}

		*fileContents = DISK_READ_LOCATION + (unsigned short)bootsect.sectors_per_cluster * (unsigned short)bootsect.bytes_per_sector * readInOffset; //return a pointer in the BIOS read-in space where the file is.

		return 0; //file successfully found
	}
	else
		return -2; //the path specified was a directory
}

//recieves the cluster to read for a directory and the requested file, and will iterate through the directory's clusters - returning the entry for the searched file/subfolder, or no file/subfolder
//return value holds success or failure code, file holds directory entry if file is found
//entryOffset points to where the directory entry was found in sizeof(directory_entry_t) (can be NULL)
int directorySearch(const char* filepart, const unsigned int cluster, directory_entry_t* file, unsigned int* entryOffset) //works! (iterative functionality hasn't been tested by fire yet though)
{
	char searchName[13] = { '\0' };
	strcpy(searchName, filepart);
	convertToFATFormat(searchName);

	clusterRead(cluster, 0); //read cluster of the directory/subdirectory
	directory_entry_t* file_metadata = (directory_entry_t*)DISK_READ_LOCATION;
	unsigned int meta_pointer_iterator_count = 0; //holds how many directories have been looked at

	while (1)
	{
		if (strncmp((char*)file_metadata->file_name, searchName, 11) != 0) //if the file doesn't match 
		{
			if (meta_pointer_iterator_count < bootsect.bytes_per_sector * bootsect.sectors_per_cluster / sizeof(directory_entry_t) - 1) //if the pointer hasn't iterated outside of what that cluster can hold
			{
				file_metadata++;
				meta_pointer_iterator_count++;
			}
			else
			{
				unsigned int next_cluster = FATRead(cluster);

				if (next_cluster >= END_CLUSTER_32)
					break;
				else
					return directorySearch(filepart, next_cluster, file, entryOffset);//search next cluster
			}
		}
		else
		{
			memcpy(file, file_metadata, sizeof(directory_entry_t)); //copy found data to file
			if (entryOffset != NULL)
				*entryOffset = meta_pointer_iterator_count;
			return 0;
		}
	}

	return -1; //nothing found, return error.
}

//read FAT table
//This function deals in absolute data clusters
int FATRead(unsigned int clusterNum) //32 works! Don't know about 16 or 12
{
	if (fat_type == 32)
	{
		unsigned int cluster_size = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
		unsigned char FAT_table[32 * 1024] {'\0'}; //Takes into consideration the largest standard cluster size (32kB) since arrays can't be dynamically allocated without "new" :/
		unsigned int fat_offset = clusterNum * 4;
		unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
		unsigned int ent_offset = fat_offset % cluster_size;

		//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
		int13h_read(fat_sector, 1);
		memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector);

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
		int13h_read(fat_sector, 1);
		memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector);

		unsigned short table_value = *(unsigned short*)&FAT_table[ent_offset];

		//the variable "table_value" now has the information you need about the next cluster in the chain.
		return table_value;
	}
	else if (fat_type == 12)
	{
		NULL; //Not Implemented!
	}

	return -1;
}

//Reads one cluster and dumps it to DISK_READ_LOCATION, offset "cluster_size" number of bytes from DISK_READ_LOCATION
//This function deals in absolute data clusters
void clusterRead(unsigned int clusterNum, unsigned int clusterOffset) //Works!
{
	unsigned int start_sect = (clusterNum - 2) * (unsigned short)bootsect.sectors_per_cluster + first_data_sector; //Explanation: Since the root cluster is cluster 2, but data starts at first_data_sector, subtract 2 to get the proper cluster offset from zero.

	int13h_read_o(start_sect, (unsigned short)bootsect.sectors_per_cluster, clusterOffset * (unsigned short)bootsect.sectors_per_cluster * (unsigned short)bootsect.bytes_per_sector);
}

//Deals in absolute clusters
//contentsToWrite: contains a pointer to the data to be written to disk
//contentSize: contains how big contentsToWrite's data is (in bytes)
//contentBuffOffset: sets how far offset from DISK_WRITE_LOCATION to place the data from contentsToWrite in preparation for writing to disk (in clusters)
//clusterNum: Specifies the on-disk cluster to write the data to
void clusterWrite(void* contentsToWrite, unsigned int contentSize, unsigned int contentBuffOffset, unsigned int clusterNum) //Works!
{
	unsigned int byteOffset = contentBuffOffset * (unsigned short)bootsect.sectors_per_cluster * (unsigned short)bootsect.bytes_per_sector; //converts cluster memory offset into bytes

	memcpy((char*)DISK_WRITE_LOCATION + byteOffset, contentsToWrite, contentSize); //copy contents to be written to disk to the memory write location

	unsigned int start_sect = (clusterNum - 2) * (unsigned short)bootsect.sectors_per_cluster + first_data_sector; //Explanation: Since the root cluster is cluster 2, but data starts at first_data_sector, subtract 2 to get the proper cluster offset from zero.

	int13h_write_o(start_sect, (unsigned short)bootsect.sectors_per_cluster, byteOffset);
}

int FATWrite(unsigned int clusterNum, unsigned int clusterVal) //Works!
{
	if (fat_type == 32)
	{
		unsigned int cluster_size = bootsect.bytes_per_sector * bootsect.sectors_per_cluster;
		unsigned char FAT_table[32 * 1024] {'\0'}; //Takes into consideration the largest standard cluster size (32kB) since arrays can't be dynamically allocated without "new" :/
		unsigned int fat_offset = clusterNum * 4;
		unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
		unsigned int ent_offset = fat_offset % cluster_size;

		//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
		int13h_read(fat_sector, 1);
		memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector);

		//copy clusterVal into FAT_table
		*(unsigned int*)&FAT_table[ent_offset] = clusterVal;

		//send modified FAT_table back to disk
		memcpy((char*)DISK_WRITE_LOCATION, &FAT_table, bootsect.bytes_per_sector);
		int13h_write(fat_sector, 1);

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
		int13h_read(fat_sector, 1);
		memcpy(&FAT_table, (char*)DISK_READ_LOCATION, bootsect.bytes_per_sector);

		//copy clusterVal into FAT_table
		*(unsigned short*)&FAT_table[ent_offset] = (unsigned short)clusterVal;

		//send modified FAT_table back to disk
		memcpy((char*)DISK_WRITE_LOCATION, &FAT_table, bootsect.bytes_per_sector);
		int13h_write(fat_sector, 1);

		return 0;
	}
	else if (fat_type == 12)
	{
		NULL; //Not Implemented!
	}

	return -1;
}

//pass in the cluster to write the directory to and the directory struct to write.
//struct should only have a file name, attributes, and size. the rest will be filled in automatically
int directoryAdd(const unsigned int cluster, directory_entry_t* file)
{
	clusterRead(cluster, 0); //read cluster of the directory/subdirectory
	directory_entry_t* file_metadata = (directory_entry_t*)DISK_READ_LOCATION;
	unsigned int meta_pointer_iterator_count = 0; //holds how many directories have been looked at

	while (1)
	{
		if (file_metadata->file_name[0] != (char)0xE5 && file_metadata->file_name[0] != (char)0x00) //if the file doesn't match 
		{
			if (meta_pointer_iterator_count < bootsect.bytes_per_sector * bootsect.sectors_per_cluster / sizeof(directory_entry_t) - 1) //if the pointer hasn't iterated outside of what that cluster can hold
			{
				file_metadata++;
				meta_pointer_iterator_count++;
			}
			else
			{
				unsigned int next_cluster = FATRead(cluster);

				if (next_cluster >= END_CLUSTER_32) //no free spaces left in the directory cluster, and no more clusters to search. Allocate a new one.
				{
					next_cluster = allocateFreeFAT();

					if (next_cluster == BAD_CLUSTER_32 || next_cluster == BAD_CLUSTER_16 || next_cluster == BAD_CLUSTER_12) //allocation unsuccessful
						break;

					FATWrite(cluster, next_cluster); //write the new cluster number to the previous cluster's FAT
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

			unsigned int new_cluster = allocateFreeFAT();
			
			if (new_cluster == BAD_CLUSTER_32 || new_cluster == BAD_CLUSTER_16 || new_cluster == BAD_CLUSTER_12) //allocation unsuccessful
				break;

			file->low_bits = new_cluster & 0x0000FFFF;
			file->high_bits = new_cluster >> 16;

			memcpy(file_metadata, file, sizeof(directory_entry_t)); //copy data to empty location
			clusterWrite(DISK_READ_LOCATION, bootsect.bytes_per_sector * bootsect.sectors_per_cluster, 0, cluster);
			return 0;
		}
	}

	return -1; //nothing found, return error.
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

unsigned int allocateFreeFAT() //works!
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
		bad_cluster = BAD_CLUSTER_16;
		end_cluster = END_CLUSTER_12;
	}
	else
		return BAD_CLUSTER_12;

	unsigned int cluster = 2;
	unsigned int clusterStatus = free_cluster;

	while (cluster < total_clusters)
	{
		clusterStatus = FATRead(cluster);
		if (clusterStatus == free_cluster)
		{
			FATWrite(cluster, end_cluster);
			return cluster;
		}
		cluster++;
	}
	return bad_cluster;
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

	directoryAdd(folder.low_bits | (folder.high_bits) << 16, &newFile);

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