#ifndef FAT_H_
#define FAT_H_

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#endif

#include "string.h"
#include "lib_c.h"
#include "lib_asm.h"

//comment this out when not testing FAT's functionality
#define TESTING_CODE

//FAT constant values
#define END_CLUSTER_32 0x0FFFFFF8 //Use OSDev.org's suggestion of 0x0FFFFFF8 even though MSYS docs > OSdev.org.
#define BAD_CLUSTER_32 0x0FFFFFF7
#define FREE_CLUSTER_32 0x00000000
#define END_CLUSTER_16 0xFFF8
#define BAD_CLUSTER_16 0xFFF7
#define FREE_CLUSTER_16 0x0000
#define END_CLUSTER_12 0xFF8
#define BAD_CLUSTER_12 0xFF7
#define FREE_CLUSTER_12 0x000

#define CLEAN_EXIT_BMASK_16 0x8000
#define HARD_ERR_BMASK_16 0x4000
#define CLEAN_EXIT_BMASK_32 0x08000000
#define HARD_ERR_BMASK_32 0x04000000

#define FILE_READ_ONLY 0x01
#define FILE_HIDDEN 0x02
#define FILE_SYSTEM 0x04
#define FILE_VOLUME_ID 0x08
#define FILE_DIRECTORY 0x10
#define FILE_ARCHIVE 0x20
#define FILE_LONG_NAME (FILE_READ_ONLY|FILE_HIDDEN|FILE_SYSTEM|FILE_VOLUME_ID)
#define FILE_LONG_NAME_MASK (FILE_READ_ONLY|FILE_HIDDEN|FILE_SYSTEM|FILE_VOLUME_ID|FILE_DIRECTORY|FILE_ARCHIVE)
#define FILE_LAST_LONG_ENTRY 0x40
#define ENTRY_FREE 0xE5
#define ENTRY_END 0x00
#define ENTRY_JAPAN 0x05
#define LAST_LONG_ENTRY 0x40

#define GET_CLUSTER_FROM_ENTRY(x) (x.low_bits | (x.high_bits << (fat_type / 2)))
#define GET_ENTRY_LOW_BITS(x) (x & (2^(fat_type / 2) - 1))
#define GET_ENTRY_HIGH_BITS(x) (x >> (fat_type / 2))
#define CONCAT_ENTRY_HL_BITS(high, low) ((high << (fat_type / 2)) | low)

#ifndef NULL
#define NULL 0
#endif

//Declarations to use when testing with a virtual disk
#ifdef _MSC_VER
#define DISK_READ_LOCATION testing_disk
#define DISK_WRITE_LOCATION testing_disk

extern unsigned long part_length;

FILE *fakeDisk;
char testing_disk[512 * 0x7F]; //simulates 0x40000 read-in address location

int int13h_read(unsigned long sector, unsigned char num);
int int13h_read_o(unsigned long sector, unsigned char num, unsigned long memoffset);
int int13h_write(unsigned long sector, unsigned char num);
int int13h_write_o(unsigned long sector, unsigned char num, unsigned long memoffset);

void printss(char *s);
void printsss(char *s, int n);
void printhex(unsigned long num, int digits);

#else
#ifndef DISK_READ_LOCATION
#define DISK_READ_LOCATION 0x40000
#endif
#ifndef DISK_WRITE_LOCATION
#define DISK_WRITE_LOCATION 0x40000
#endif

extern int int13h_read(unsigned long sector, unsigned char num);
extern int int13h_read_o(unsigned long sector, unsigned char num, unsigned long memoffset);
extern int int13h_write(unsigned long sector, unsigned char num);
extern int int13h_write_o(unsigned long sector, unsigned char num, unsigned long memoffset);
#endif

#ifdef _MSC_VER
#pragma pack(push, default_val)
#pragma pack(1)
#endif


//FAT directory and bootsector structures
typedef struct fat_extBS_32
{
	//extended fat32 stuff
	unsigned int		table_size_32;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

}
#ifndef _MSC_VER
__attribute__((packed))
#endif
fat_extBS_32_t;

typedef struct fat_extBS_16
{
	//extended fat12 and fat16 stuff
	unsigned char		bios_drive_num;
	unsigned char		reserved1;
	unsigned char		boot_signature;
	unsigned int		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

}
#ifndef _MSC_VER
__attribute__((packed))
#endif
fat_extBS_16_t;

typedef struct fat_BS
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;

	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char		extended_section[54];

}
#ifndef _MSC_VER
__attribute__((packed))
#endif
fat_BS_t;

/* from http://wiki.osdev.org/FAT */

typedef struct directory_entry
{
	unsigned char file_name[11];
	unsigned char attributes;
	unsigned char reserved0;
	unsigned char creation_time_tenths;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short last_accessed;
	unsigned short high_bits;
	unsigned short last_modification_time;
	unsigned short last_modification_date;
	unsigned short low_bits;
	unsigned int file_size;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
directory_entry_t;

typedef struct fsInfo
{
	unsigned int lead_signature; //should contain 0x41615252
	unsigned char reserved1[480];
	unsigned int structure_signature; //should contain 0x61417272
	unsigned int free_space; //contains last known free cluster count. 0xFFFFFFFF indicates count is unknown.
	unsigned int last_written; //contains last-written cluster number to help FAT drivers find a free cluster. 0xFFFFFFFF indicates that cluster number is unknown.
	unsigned char reserved2[12];
	unsigned int trail_signature; //should contain 0xAA550000
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
FSInfo_t;

typedef struct long_entry
{
	unsigned char order;
	unsigned char first_five[10]; //first 5, 2-byte characters
	unsigned char attributes; //MUST BE FILE_LONG_NAME
	unsigned char type; //indicates a sub-component of a long name (leave as 0)
	unsigned char checksum;
	unsigned char next_six[12]; //next 6, 2-byte characters
	unsigned short zero; //must be zero - otherwise meaningless
	unsigned char last_two[4]; //last 2, 2-byte characters
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
long_entry_t;

#ifdef _MSC_VER
#pragma pack(pop, default_val)
#endif

//Global variables
unsigned int fat_type;
unsigned int first_fat_sector;
unsigned int first_data_sector;
unsigned int total_clusters;
fat_BS_t bootsect;

//FAT functions
int directorySearch(const char* filepart, const unsigned int cluster, directory_entry_t* file, unsigned int* entryOffset);
int directoryList(const unsigned int cluster, unsigned char attributesToAdd, short exclusive);
int FATInitialize();
int FATRead(unsigned int clusterNum);
int FATWrite(unsigned int clusterNum, unsigned int clusterVal);
int getFile(const char* filePath, char** fileContents, directory_entry_t* fileMeta, unsigned int readInOffset);
int putFile(const char* filePath, char** fileContents, directory_entry_t* fileMeta);
int clusterRead(unsigned int clusterNum, unsigned int clusterOffset);
int clusterWrite(void* contentsToWrite, unsigned int contentSize, unsigned int contentBuffOffset, unsigned int clusterNum);
char* convertToFATFormat(char* input);
void convertFromFATFormat(char* input, char* output);
unsigned char ChkSum(unsigned char *pFcbName);
int UpdateBootSect(fat_BS_t newContents); //don't forget the backup bootsector!
int UpdateFSInfo(FSInfo_t newInfo); //don't forget the backup FSInfo! (if one exists)
unsigned short CurrentTime();
unsigned char CurrentTimeTenths();
unsigned short CurrentDate();
unsigned int allocateFreeFAT();

#endif