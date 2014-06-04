#define NULL 0
#define TESTING

#include "lib_asm.h"
#include "lib_c.h"
#include "string.h"
#include "FAT.h"

// Included in stage2.asm
void real_mode_sw_int_call();

// Included in this file
int enableA20();
int int13h_read(  unsigned long sector_offset, unsigned char num_blocks );
int int13h_read_o(unsigned long sector_offset, unsigned char num_blocks, unsigned long readLocationOffset);
int int13h_write( unsigned long sector_offset, unsigned char num_blocks );
int int13h_write_o(unsigned long sector_offset, unsigned char num_blocks, unsigned long writeLocationOffset);

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

// Some "variables" defined in stage2.asm
extern unsigned long part_start_lba;
extern unsigned long part_length;
extern unsigned char bios_disk_number;
extern Real_Mode_SW_Int real_mode_linear_sw_int;
extern Disk_Address_Packet disk_address_packet;

int main(void) {

	part_start_lba = 33543720;
	part_length = 2*8385930;

   char buf, *addr, checksum;
   int i, count, lba_sector, num_blocks;
 
   printss( "\nMiniOS32 Bootloader v2.0 -> Stage 2\n" );
   printss( "Copyright (C) 2001, 2013\n" );
   printss( "Enter \"?\" for help\n" );
   printss( "Boot ROM date = " );
   printsss( ( char * ) 0xFFFF5, 8 );
   printss( "\nEIP = " );
   printhex( get_eip(), 8 );
   enableA20();
   
   FATInitialize();
   
   printss( "\npart_start_lba=" );
   printhex( part_start_lba, 8 );
   printss( "\npart_length=" );
   printhex( part_length, 8 );
   printss( "\nbios_disk_number=" );
   printhex( bios_disk_number, 2 );
   
   //NEED CHANGE----Switch to 13h
   real_mode_linear_sw_int.al = 0x12;   // select mode 0x12
   real_mode_linear_sw_int.ah = 0x00;   // set video mode
   real_mode_linear_sw_int.opcode = 0xCD;
   real_mode_linear_sw_int.operand = 0x10;
   
   real_mode_sw_int_call(); 
   
   while ( 1 ) {

      printss( "\n>" );

      buf = lowercase( getcc( 1 ) );

      if ( buf == 'b' ) {

         printss( "\nRebooting!!\n" );
         reset();

      }
       else if (buf == 'a') {
		  printss("fat type = ");
		  printhex(fat_type, 8);
		  printss("\n");
		  printss("bootsect.oem_name = ");
		  printss(bootsect.oem_name);
		  printss("\n");
		  printss("bootsect.bytes_per_sector = ");
		  printhex(bootsect.bytes_per_sector, 4);
		  printss("\n");
		  printss("bootsect.sectors_per_cluster = ");
		  printhex(bootsect.sectors_per_cluster, 2);
		  printss("\n");
		  printss("bootsect.reserved_sector_count = ");
		  printhex(bootsect.reserved_sector_count, 4);
		  printss("\n");
		  printss("bootsect.table_count = ");
		  printhex(bootsect.table_count, 2);
		  printss("\n");
		  printss("bootsect.root_entry_count = ");
		  printhex(bootsect.root_entry_count, 4);
		  printss("\n");
		  printss("bootsect.total_sectors_16 = ");
		  printhex(bootsect.total_sectors_16, 4);
		  printss("\n");
		  printss("bootsect.media_type = ");
		  printhex(bootsect.media_type, 2);
		  printss("\n");
		  printss("bootsect.table_size_16 = ");
		  printhex(bootsect.table_size_16, 4);
		  printss("\n");
		  printss("bootsect.sectors_per_track = ");
		  printhex(bootsect.sectors_per_track, 4);
		  printss("\n");
		  printss("bootsect.head_side_count = ");
		  printhex(bootsect.head_side_count, 4);
		  printss("\n");
		  printss("bootsect.hidden_sector_count = ");
		  printhex(bootsect.hidden_sector_count, 8);
		  printss("\n");
		  printss("bootsect.total_sectors_32 = ");
		  printhex(bootsect.total_sectors_32, 8);
		  printss("\n");
		  fat_extBS_32_t* f32 = (fat_extBS_32_t*)bootsect.extended_section;
		  
		  printss("f32->table_size_32 = ");
		  printhex(f32->table_size_32, 8);
		  printss("\n");
		  printss("f32->extended_flags = ");
		  printhex(f32->extended_flags, 4);
		  printss("\n");
		  printss("f32->fat_version = ");
		  printhex(f32->fat_version, 4);
		  printss("\n");
		  printss("f32->root_cluster = ");
		  printhex(f32->root_cluster, 8);
		  printss("\n");
		  printss("f32->fat_info = ");
		  printhex(f32->fat_info, 4);
		  printss("\n");
		  printss("f32->backup_BS_sector = ");
		  printhex(f32->backup_BS_sector, 4);
		  printss("\n");
		  printss("f32->reserved_0 = ");
		  printhex(f32->reserved_0, 3);
		  printss("\n");
		  printss("f32->drive_number = ");
		  printhex(f32->drive_number, 2);
		  printss("\n");
		  printss("f32->reserved_1 = ");
		  printhex(f32->reserved_1, 2);
		  printss("\n");
		  printss("f32->boot_signature = ");
		  printhex(f32->boot_signature, 2);
		  printss("\n");
		  printss("f32->volume_id = ");
		  printhex(f32->volume_id, 8);
		  printss("\n");
		  printss("f32->volume_label = ");
		  printss(f32->volume_label);
		  printss("\n");
		  printsss("f32->fat_type_label = ", 11);
		  printsss(f32->fat_type_label, 8);
		  printss("\n");
		  
		  printhex(FATRead(2), 8);
		  printss("\n");
		  
		  char *file;
		  directory_entry_t entry;
		  //directorySearch("test.txt", ((fat_extBS_32_t*)bootsect.extended_section)->root_cluster, &entry, NULL);
		  printss(entry.file_name);
		  //printss(&entry., );
		  char* test = "C:\\test.txt";
		  printhex(test,8);
		  printss(test);
		  printss("\nTest1 complete...\n\n\n\n");
		  //getFile( 0x12345678, 0x34343434, 0x56565656, 0xabababab);
		  getFile( test, &file, &entry, 1);
		  printss(entry.file_name);
		  printss(file);
		  
		  directoryList(((fat_extBS_32_t*)(bootsect.extended_section))->root_cluster, NULL);
	  }

      else if ( buf == 'c' ) {
      
         if ( gethex( ( void * ) &addr, 8, -1 ) != 0 )
            continue;

         // The line below treats addr as a pointer to a function
         ( ( void ( * ) () ) addr )();

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
         printss( "\na                           - FAT test" );
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

// enable A20 address line using the keyboard controller,
// to allow sensible access to RAM > 1MB.
// Returns 0 on success, non-zero on failure.
// Best ensure that irqs are disabled before calling this function.
int enableA20() {
   
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
         
         return 0;
         
      }
      
   }
   
   printss( "\nAll attempts to enable A20 FAILED!" );
   
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

// Read sector(s) from booted partition to 0x4000:0000
// Returns 0 on success and non-zero on failure
int int13h_read_o(unsigned long sector_offset, unsigned char num_blocks, unsigned long readLocationOffset) {

	// Bounds check; only read sectors on the booted partition
	if (sector_offset + num_blocks > part_length)
		return -1;

	//Size check; only read blocks if the resulting size read will still be between 0x4000 and 0x8000
	if (readLocationOffset + num_blocks * 512 > (0x8000 - 0x4000))
		return -1;

	// Sanity check; BIOS int13h can only read up to 0x7F blocks
	if (num_blocks > 0x7F)
		return -1;

	disk_address_packet.packet_size = 0x10;
	disk_address_packet.resv1 = 0x00;
	disk_address_packet.num_blocks = num_blocks;
	disk_address_packet.resv2 = 0x00;
	disk_address_packet.offset = readLocationOffset;
	disk_address_packet.segment = 0x4000;
	disk_address_packet.LBA_sector = (unsigned long long) part_start_lba + (unsigned long long) sector_offset;

	real_mode_linear_sw_int.ah = 0x42;   // Extended read
	real_mode_linear_sw_int.dl = bios_disk_number;
	real_mode_linear_sw_int.si = 0x0040; // Pointer to dap (in real mode memory...DS:SI)
	real_mode_linear_sw_int.opcode = 0xCD;
	real_mode_linear_sw_int.operand = 0x13;

	real_mode_sw_int_call();

	return real_mode_linear_sw_int.ah;

}

// Write sector(s) to booted partition from 0x4000:0000
// Returns 0 on success and non-zero on failure
int int13h_write( unsigned long sector_offset, unsigned char num_blocks ) {
   
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

// Write sector(s) to booted partition from 0x4000:0000
// Returns 0 on success and non-zero on failure
int int13h_write_o(unsigned long sector_offset, unsigned char num_blocks, unsigned long writeLocationOffset) {

	// Bounds check; only write sectors on the booted partition
	if (sector_offset + num_blocks > part_length)
		return -1;

	//Size check; only write blocks if the resulting size written will still be between 0x4000 and 0x8000
	if (writeLocationOffset + num_blocks * 512 > (0x8000 - 0x4000))
		return -1;

	// Sanity check; BIOS int13h can only write up to 0x7F blocks
	if (num_blocks > 0x7F)
		return -1;

	disk_address_packet.packet_size = 0x10;
	disk_address_packet.resv1 = 0x00;
	disk_address_packet.num_blocks = num_blocks;
	disk_address_packet.resv2 = 0x00;
	disk_address_packet.offset = writeLocationOffset;
	disk_address_packet.segment = 0x4000;
	disk_address_packet.LBA_sector = (unsigned long long) part_start_lba + (unsigned long long) sector_offset;

	real_mode_linear_sw_int.al = 0x00;   // Write with verify off
	real_mode_linear_sw_int.ah = 0x43;   // Extended write
	real_mode_linear_sw_int.dl = bios_disk_number;
	real_mode_linear_sw_int.si = 0x0040; // Pointer to dap (in real mode memory...DS:SI)
	real_mode_linear_sw_int.opcode = 0xCD;
	real_mode_linear_sw_int.operand = 0x13;

	real_mode_sw_int_call();

	return real_mode_linear_sw_int.ah;

}

