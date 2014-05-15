#define NULL 0

#include "string.h"

char inb( unsigned short port );
void callfn();
long get_eip();
void irq_disable();
void irq_enable();
void reset();
void real_mode_sw_int_call();

void FATInitialize();

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

}_attribute__((packed)) fat_extBS_32_t;

typedef struct fat_extBS_16
{
	//extended fat12 and fat16 stuff
	unsigned char		bios_drive_num;
	unsigned char		reserved1;
	unsigned char		boot_signature;
	unsigned int		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

}_attribute__((packed)) fat_extBS_16_t;

typedef struct fat_BS
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	        bytes_per_sector;
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

}_attribute__((packed)) fat_BS_t;

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

}_attribute__((packed)) directory_entry_t;

// For use with real_mode_sw_int_call function; holds register parameters
// and interrupt type. It is also used to return any register values. All
// general purpose registers (AX, BX, CX and DX) can also be accessed via
// their high and low bytes, e.g., AH and AL.
typedef struct real_mode_sw_int {

   unsigned short flags;
   
   unsigned short di;
   unsigned short si;
   unsigned short bp;
   unsigned short sp_ignored;	// SP is not restored from POPA
   
   union {
      unsigned short bx;
      struct {
         unsigned char bl;
         unsigned char bh;
      };
   };
   
   union {
      unsigned short dx;
      struct {
         unsigned char dl;
         unsigned char dh;
      };
   };
   
   union {
      unsigned short cx;
      struct {
         unsigned char cl;
         unsigned char ch;
      };
   };
   
   union {
      unsigned short ax;
      struct {
         unsigned char al;
         unsigned char ah;
      };
   };
   
   unsigned char  opcode;	// Suggestion: 0xCD
   unsigned char  operand;

} Real_Mode_SW_Int;

// For use with Int 13h extensions, functions 42h (extended read) and
// 43h (extended write).
typedef struct disk_address_packet {

   unsigned char      packet_size;	// 0x10
   unsigned char      resv1;	      // 0x00
   unsigned char      num_blocks;	// Maximum 0x7F
   unsigned char      resv2;	      // 0x00
   unsigned short     offset;	      // Real-mode transfer buffer
   unsigned short     segment;	   // Real-mode transfer buffer
   unsigned long long LBA_sector;	// Sector to read/write

} Disk_Address_Packet;

extern unsigned long part_start_lba;
extern unsigned long part_length;
extern unsigned char bios_disk_number;
extern unsigned long _start;
extern Real_Mode_SW_Int real_mode_linear_sw_int;
extern unsigned long real_mode_linear_esp;
extern unsigned long XXX;
extern unsigned long YYY;
extern Disk_Address_Packet disk_address_packet;

unsigned short *video = ( unsigned short * ) 0xB8000;

unsigned short fat_type = 0;
fat_BS_t bootsect;
unsigned int first_fat_sector;

#define END_SECTOR_32 0x0FFFFFF8
#define BAD_SECTOR_32 0x0FFFFFF7
#define FREE_SECTOR_32 0x00000000
#define END_SECTOR_16 0xFF8
#define BAD_SECTOR_16 0xFF7
#define FREE_SECTOR_16 0x000
#define END_SECTOR_8 END_SECTOR_16
#define BAD_SECTOR_8 BAD_SECTOR_16
#define FREE_SECTOR_8 FREE_SECTOR_16


#define TTY 0x03F8
#define DLLB	 0
#define IER	 1
#define DLHB	 1
#define IIR	 2
#define FCR	 2
#define LCR	 3
#define MCR	 4
#define LSR	 5
#define MSR	 6
#define SCRATCH	 7

// send byte to I/O port
static inline void outb( unsigned short port, unsigned char val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* TODO: Is it wrong to use 'N' for the port? It's not a 8-bit constant. */
    /* TODO: Should %1 be %w1? */
    /* TODO: Is there any reason to force the use of eax and edx? */
}

// send a character to the serial port console
void putcc( char c ) {

   if ( c == 0x0A )
      putcc( 0x0D );

   while ( ( inb( TTY + LSR ) & 0x20 ) == 0x00 )
      ;
   
   outb( TTY, c );

}

// get a character from the serial port console
// echoes back to the console if echo is non-zero
char getcc( int echo ) {

   char c;

   while ( ( inb( TTY + LSR ) & 0x01 ) == 0x00 )
      ;
   
   c = inb( TTY );
   
   if ( echo )
      putcc( c );
   
   return c;

}

// print a number in hexadecimal format
void printhex( unsigned long num, int digits ) {

   int i, buf;

   for ( i = 0; i < digits; i++ ) {
   
      buf = ( num >> ( ( digits - i - 1 ) * 4 ) ) & 0xF;
      if ( buf < 10 )
         buf += '0';
      else
         buf += 'A' - 10;
      putcc( buf );
   
   }
   
}

// print a null-terminated string
void printss( char *s ) {

   while ( *s ) {

      putcc( *s );
      s++;
   
   }

}

// print n characters from the string pointed to by s
// protect against control characters < 32 and > 127
void printsss( char *s, int n ) {

   while ( n-- ) {

      if ( *s > ' ' )
         putcc( *s );
      else
         putcc( '.' );

      s++;

   }

}

// converts to lowercase
char lowercase( char c ) {

   if ( ( c >= 'A' ) && ( c <= 'Z' ) )
      c += 'a' - 'A';

   return c;

}

// gets up to 8 hex digits & stores in num
// returns 0 on success and non-zero on invalid input
int gethex( unsigned long *num, int digits, int echo ) {

   char buf;
   int i;

   if ( echo )
      printss( " " );
   *num = 0;

   for ( i = 0; i < digits; i++ ) {
   
      buf = lowercase( getcc( echo ) );
      if ( ( buf < '0' ) || ( buf > 'f' ) || ( ( buf > '9' ) && ( buf < 'a' ) ) )
         return -1;
      if ( buf > '9' )
         buf -= 'a' - 10;
      else
         buf -= '0';
      *num = ( *num << 4 ) + buf;
      
   }

   return 0;
   
}

// To test 'c' command
void testfn() {

   putcc('A');

}

// enable A20 address line using the keyboard controller,
// to allow access to RAM > 1MB.
// Returns 0 on success, non-zero on failure.
int enableA20() {
   
//   irq_disable();
   
   int i;
   char status;
   
   // Make 5 attempts to enable A20
   for ( i = 0; i < 5; i++ ) {
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to read the current status
      outb( 0x0064, 0xD0 );
      
      // Wait for controller to be ready with data
      while ( ( inb( 0x0064 ) & 0x01 ) == 0 )
         ;
      
      // Read current port status
      status = inb( 0x0060 );
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to write status byte
      outb( 0x0064, 0xD1 );
      
      // Wait for controller to be ready for data
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Turn on A20 enable bit
      status |= 0x02;
      
      // Write new value
      outb( 0x0060, status );
      
      // Check that it was enabled
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to read the current status
      outb( 0x0064, 0xD0 );
      
      // Wait for controller to be ready with data
      while ( ( inb( 0x0064 ) & 0x01 ) == 0 )
         ;
      
      // Read current port status
      status = inb( 0x0060 );
      
      // Is A20 enabled?
      if ( status & 0x02 ) {
         
         printss( "\nA20 enabled using PRIMARY method on attempt " );
         printhex( i + 1, 1 );
         printss( "." );
         
//         irq_enable();
         
         return 0;
         
      }
      
   }
   
   // Initial attempt(s) to enable A20 has failed. Try a backup
   // method that is not supported on many chipsets but is the
   // only method that works on other chipsets.
   
   // Make 5 more attempts to enable A20
   for ( i = 0; i < 5; i++ ) {
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to turn on A20
      outb( 0x0064, 0xDF );
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to read the current status
      outb( 0x0064, 0xD0 );
      
      // Wait for controller to be ready with data
      while ( ( inb( 0x0064 ) & 0x01 ) == 0 )
         ;
      
      // Read current port status
      status = inb( 0x0060 );
      
      // Is A20 enabled?
      if ( status & 0x02 ) {
         
         printss( "\nA20 enabled using SECONDARY method on attempt " );
         printhex( i + 1, 1 );
         printss( "." );
         
//         irq_enable();
         
         return 0;
         
      }
      
   }
   
   printss( "\nAll attempts to enable A20 FAILED!" );
   
//   irq_enable();
   
   return -1;
   
}

// Read sector(s) from booted partition to 0x4000:0000
// Returns 0 on success and non-zero on failure
int int13h_read( unsigned long sector_offset, unsigned char num_blocks ) {
   
   // Bounds check; only read sectors on the booted partition
   if ( sector_offset + num_blocks > part_length )
      return -1;
   
   // Sanity check; BIOS int13h can only read up to 0x7F blocks
   if ( num_blocks > 0x7F )
      return -1;
   
   disk_address_packet.packet_size = 0x10;
   disk_address_packet.resv1 = 0x00;
   disk_address_packet.num_blocks = num_blocks;
   disk_address_packet.resv2 = 0x00;
   disk_address_packet.offset = 0x0000;
   disk_address_packet.segment = 0x4000;
   disk_address_packet.LBA_sector = ( unsigned long long ) part_start_lba + ( unsigned long long ) sector_offset;
   
   real_mode_linear_sw_int.ah = 0x42;   // Extended read
   real_mode_linear_sw_int.dl = bios_disk_number;
   real_mode_linear_sw_int.si = 0x0040; // Pointer to dap (in real mode memory...DS:SI)
   real_mode_linear_sw_int.opcode = 0xCD;
   real_mode_linear_sw_int.operand = 0x13;
   
   real_mode_sw_int_call();
   
   return real_mode_linear_sw_int.ah;
   
}

// Write sector(s) to booted partition from 0x4000:0100
// Returns 0 on success and non-zero on failure
int int13h_write( unsigned long sector_offset, char num_blocks ) {
   
   // Bounds check; only write sectors on the booted partition
   if ( sector_offset + num_blocks > part_length )
      return -1;
   
   // Sanity check; BIOS int13h can only write up to 0x7F blocks
   if ( num_blocks > 0x7F )
      return -1;
   
   disk_address_packet.packet_size = 0x10;
   disk_address_packet.resv1 = 0x00;
   disk_address_packet.num_blocks = num_blocks;
   disk_address_packet.resv2 = 0x00;
   disk_address_packet.offset = 0x0000;
   disk_address_packet.segment = 0x4000;
   disk_address_packet.LBA_sector = ( unsigned long long ) part_start_lba + ( unsigned long long ) sector_offset;
   
   real_mode_linear_sw_int.al = 0x00;   // Write with verify off
   real_mode_linear_sw_int.ah = 0x43;   // Extended write
   real_mode_linear_sw_int.dl = bios_disk_number;
   real_mode_linear_sw_int.si = 0x0040; // Pointer to dap (in real mode memory...DS:SI)
   real_mode_linear_sw_int.opcode = 0xCD;
   real_mode_linear_sw_int.operand = 0x13;
   
   real_mode_sw_int_call();
   
   return real_mode_linear_sw_int.ah;
   
}

int main(void) {

   char buf, *addr, checksum;
   int i, count, lba_sector, num_blocks;
 
   printss( "\nSecureOS Bootloader v2.0 -> Stage 2\n" );
   printss( "Copyright (C) 2001, 2013\n" );
   printss( "Enter \"?\" for help\n" );
   printss( "Boot ROM date = " );
   printsss( ( char * ) 0xFFFF5, 8 );
   printss( "\nEIP = " );
   printhex( get_eip(), 8 );
   enableA20();
   
   printss( "\npart_start_lba=" );
   printhex( part_start_lba, 8 );
   printss( "\npart_length=" );
   printhex( part_length, 8 );
   printss( "\nbios_disk_number=" );
   printhex( bios_disk_number, 2 );
   
   printss( "\n_start=" );
   printhex( ( unsigned long ) &_start, 8 );
   printss( "\nreal_mode_linear_sw_int=" );
   printhex( ( unsigned long ) &real_mode_linear_sw_int, 8 );
   printss( "\nreal_mode_linear_esp=" );
   printhex( ( unsigned long ) &real_mode_linear_esp, 8 );
   printss( "\nXXX=" );
   printhex( ( unsigned long ) &XXX, 8 );
   printss( "\nYYY=" );
   printhex( ( unsigned long ) &YYY, 8 );
   
   while ( 1 ) {

      printss( "\n>" );

      buf = lowercase( getcc( 1 ) );

      if ( buf == 'b' ) {

         printss( "\nRebooting!!\n" );
         reset();

      }
      
      else if ( buf == 'c' ) {
      
         if ( gethex( ( void * ) &addr, 8, -1 ) != 0 )
            continue;

         callfn( addr );

      }
      
      else if ( buf == 'd' ) {
      
         if ( gethex( ( void * ) &addr, 8, -1 ) != 0 )
            continue;

         if ( gethex( ( void * ) &count, 8, -1 ) != 0 )
            continue;

         count += ( unsigned long ) addr & 0x0000000F;
         addr = ( char * ) ( ( unsigned long ) addr & 0xFFFFFFF0 );
         checksum = 0;
         
         while ( count > 0 ) {
         
            printss( "\n" );
            printhex( ( unsigned long ) addr, 8 );
            printss( " " );
            
            for ( i = 0; i < 16; i++, addr++ ) {
            
               printhex( *addr, 2 );
               printss( " " );
            
            }
            
            addr -= 16;
            
            for ( i = 0; i < 16; i++, addr++ ) {
            
               printsss( addr, 1 );
               checksum += *addr;
            
            }
            
            count -= 16;
         
         }

         printss( "\nChecksum = " );
         printhex( checksum, 2 );
         
      }
      
      else if ( ( buf == 'f' ) || ( buf == 's' ) ) {
      
         if ( gethex( ( void * ) &addr, 8, ( buf == 's' ) ? 0 : -1 ) != 0 )
            continue;

         addr = ( char * ) ( ( unsigned long ) addr & 0xFFFFFFF0 );

         for ( i = 0; ; i++, addr++ ) {
         
            if ( ( i % 16 == 0 ) && ( buf != 's' ) ) {
            
               printss( "\n" );
               printhex( ( unsigned long ) addr, 8 );
            
            }
            
            if ( gethex( ( void * ) &count, 2, ( buf == 's' ) ? 0 : -1 ) != 0 )
               break;
      
            *addr = *( ( char * ) &count );
      
         }
      
      }
      
      else if ( buf == 'i' ) {
      
         if ( gethex( ( void * ) &addr, 4, -1 ) != 0 )
            continue;
         
         printss( "\n" );
         printhex( inb( ( unsigned long ) addr ), 2 );
      
      }
         
      else if ( buf == 'o' ) {
      
         if ( gethex( ( void * ) &addr, 4, -1 ) != 0 )
            continue;
         
         if ( gethex( ( void * ) &count, 2, -1 ) != 0 )
            continue;
         
         outb( ( unsigned long ) addr, count );
      
      }
      
      else if ( buf == 'm' ) {
      
         if ( gethex( ( void * ) &addr, 8, -1 ) != 0 )
            continue;
         
         printss( "\n" );
         printhex( *addr, 2 );
         
         if ( gethex( ( void * ) &count, 2, -1 ) != 0 )
            continue;
         
         *addr = count;
      
      }
      
      else if ( buf == 'r' ) {
         
         if ( gethex( ( void * ) &lba_sector, 8, -1 ) != 0 )
            continue;
         
         if ( gethex( ( void * ) &num_blocks, 2, -1 ) != 0 )
            continue;
         
         int13h_read( lba_sector, num_blocks );

      }
      
      else if ( buf == 'w' ) {
         
         if ( gethex( ( void * ) &lba_sector, 8, -1 ) != 0 )
            continue;
         
         if ( gethex( ( void * ) &num_blocks, 2, -1 ) != 0 )
            continue;
         
         int13h_write( lba_sector, num_blocks );

      }
      
      else if ( buf == '?' ) {
         
         printss( "\nInternal commands:" );
         printss( "\nb                           - reboot" );
         printss( "\nc <addr>                    - call function" );
         printss( "\nd <addr> <count>            - dump range of memory" );
         printss( "\nf <addr>                    - fill range of memory" );
         printss( "\ni <port>                    - read byte from I/O port" );
         printss( "\nm <addr>                    - modify memory location" );
         printss( "\no <port> <byte>             - write byte to I/O port" );
         printss( "\nr <lba_sector> <num_blocks> - read disk sector(s) to 0x00040000" );
         printss( "\ns <addr>                    - \"silent\" fill (same as f without echo)" );
         printss( "\nw <lba_sector> <num_blocks> - write disk sector(s) from 0x00040000" );
         
      }

   }

   return 0;

}

/*///////////////////Temporary Declarations/////////////////*/
int directorySearch(const char* filepart, const unsigned int cluster, directory_entry_t* file);
void FATInitialize();
int getFile(const char* filePath, char** filePointer);
int FATRead(unsigned int clusterNum);
void clusterRead(unsigned int clusterNum);


void FATInitialize()
{
	int13h_read(1, 1); //reads the first sector of the FAT

	fat_BS_t* bootstruct = (fat_BS_t*)0x40000;

	unsigned int total_clusters = bootstruct->total_sectors_16 / bootstruct->sectors_per_cluster;

	if (total_clusters == 0)
	{
		total_clusters = bootstruct->total_sectors_32 / bootstruct->sectors_per_cluster;
	}

	if (total_clusters < 4085)
	{
		fat_type = 12;
	}
	else
	{
		if (total_clusters < 65525)
		{
			fat_type = 16;
		}
		else
		{
			fat_type = 32;
		}
	}

	memcpy(&bootsect, bootstruct, 512);
	
	first_fat_sector = bootstruct->reserved_sector_count;
}

int getFile(const char* filePath, char** filePointer)
{
	char fileNamePart[255] = { '\0' };
	unsigned short start = 3;
	directory_entry_t file_info;

	//starting at 3 to skip the "C:\" bit
	for (unsigned int iterator = 3; filePath[iterator] != '\0'; iterator++)
	{
		if (filePath[iterator] == '\\')
		{
			memcpy(fileNamePart, filePath + start, iterator - start - 1); //hacked-together strcpy...

			unsigned int active_cluster = 2; //just a default cluster in case something goes haywire
			
			if (start == 3)
				active_cluster = ((fat_extBS_32_t*)bootsect.extended_section)->root_cluster;

			int retVal = directorySearch(fileNamePart, active_cluster, &file_info);

			start = iterator + 1;
			///THIS FUNCTION NOT FINISHED
		}
	}
}

//recieves the cluster to read for a directory and the requested file, and will iterate through the directory's clusters - returning the entry for the searched file/subfolder, or no file/subfolder
//return value holds success or failure code, file holds directory entry if file is found
int directorySearch(const char* filepart, const unsigned int cluster, directory_entry_t* file)
{
	clusterRead(cluster);
	directory_entry_t* file_metadata = (directory_entry_t*)0x40000;
	unsigned int meta_pointer_iteratator_count = 0;

	while (1)
	{
		if (strcpy(file_metadata->file_name, filepart) != 0) //STRCPY NOT IMPLEMENTED
		{
			if (meta_pointer_iteratator_count < bootsect.bytes_per_sector * bootsect.sectors_per_cluster / sizeof(directory_entry_t)-1) //if the pointer hasn't iterated outside of what that cluster can hold
			{
				file_metadata++;
				meta_pointer_iteratator_count++;
			}
			else
			{
				unsigned int next_cluster = FATRead(cluster);

				if (next_cluster >= BAD_SECTOR_32)
					break;
				else
					return directorySearch(filepart, cluster, file);//search next cluster
			}
		}
		else
		{
			memcpy(file, file_metadata, sizeof(directory_entry_t)); //copy found data to file
			return 0;
		}
	}

	return -1;
}

//read FAT table
//This function deals in absolute data clusters
int FATRead(unsigned int clusterNum)
{
	if (fat_type == 32)
	{
	    unsigned int cluster_size = bootsect.bytes_per_sector; //is actually sector size
		unsigned char FAT_table [32 * 1024]; //Takes into consideration the largest standard sector size since arrays can't be dynamically allocated without "new"
		unsigned int fat_offset = clusterNum * 4;
		unsigned int fat_sector = first_fat_sector + (fat_offset / cluster_size);
		unsigned int ent_offset = fat_offset % cluster_size;

		//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
		int13h_read(fat_sector, 1);
		memcopy(&FAT_table, 0x40000, bootsect.bytes_per_sector);

		//remember to ignore the high 4 bits.
		unsigned int table_value = *(unsigned int*)&FAT_table[ent_offset] & 0x0FFFFFFF;

		//the variable "table_value" now has the information you need about the next cluster in the chain.
		return table_value;
	}
	else if (fat_type == 16 || fat_type == 12)
	{
		NULL; //not implemented
	}
	else
		NULL; //Error

	return -1;
}



//Reads one cluster and dumps it to 0x40000
//NOTE: Absolute data clusters start at #2, not 1 or 0, while relative cluster numbers start relative to the first data cluster (2). E.g: The relative cluster number of the first data cluster is 0.
//This function deals in absolute data clusters
void clusterRead(unsigned int clusterNum)
{
	if (fat_type == 32)
	{
		fat_extBS_32_t* fat32Data = (fat_extBS_32_t*)(bootsect.extended_section);

		unsigned int start_sect = clusterNum * bootsect.sectors_per_cluster + bootsect.reserved_sector_count + fat32Data->table_size_32 * bootsect.table_count; //translation: get the start sector of the cluster wanted, but since there are reserved sectors for the boot sector, etc. as well as the FAT, add on the sizes of the reserved and the FAT to skip them.

		int13h_read(start_sect, bootsect.sectors_per_cluster);
	}
	else if (fat_type == 16 || fat_type == 12)
	{
		unsigned int start_sect = clusterNum * bootsect.sectors_per_cluster + bootsect.reserved_sector_count + bootsect.table_size_16 * bootsect.table_count; //translation: get the start sector of the cluster wanted, but since there are reserved sectors for the boot sector, etc. as well as the FAT, add on the sizes of the reserved and the FAT to skip them.

		int13h_read(start_sect, bootsect.sectors_per_cluster);
	}
	else
		NULL; //Error
}