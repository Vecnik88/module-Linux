#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define PARPORT_BASE 0x378

void do_io( unsigned long addr ){
	unsigned char zero = 0, readout = 0;
	printf( "\twriting 0x%02x to 0x%lx\n", zero, addr );
	outb( zero, addr );
	usleep( 1000 );
	readout = inb( addr + 1 );
	printf( "\treading: 0x%02x from 0x%lx\n", readout, addr + 1 );
}

void do_read_devport( unsigned long addr ){
	unsigned char zero = 0, readout = 0;
	int fd = 0;
	printf( "/dev/port :\n" );
	fd = open( "/dev/port", O_RDWR );
	if( fd < 0 ){
		perror( "reading /dev/port method failed" );
		return;
	}

	if( addr != lseek( fd, addr, SEEK_SET ) ){
		perror( "lseek failed" );
		close( fd );
		return;
	}

	printf( "\twriting: 0x%02x to 0x%1x\n", zero, addr );
	write( fd, &zero, 1 );
	usleep( 1000 );

	read( fd, &readout, 1 );
	printf( "\treading: 0x%02x from 0x%1x\n", readout, addr + 1 );
	close( fd );
	return;
}

void do_ioperm( unsigned long addr ){
	printf( "ioperm :\n" );
	if( ioperm( addr, 2, 1 ) ){
		perror( "ioperm failed" );
		return;
	}
	do_io( addr );
	if( ioperm( addr, 2, 0 ) )
		perror( "ioperm failed" );
}

int iopl_level = 3;

void do_iopl( unsigned long addr ){
	printf( "iopl :\n" );
	if( iopl( iopl_level ) ){
		perror( "iopl failed" );
		return;
	}
	do_io( addr );
	if( iopl( 0 ) )
		perror( "ioperm failed" );
}

int main(int argc, char** argv){
	unsigned long addr = PARPORT_BASE;
	if( argc > 1 ){
		if( !sscanf( argv[1], "%lx", &addr ) ){
			printf( "illegal addres: %s\n", argv[ 1 ] );
			return EXIT_FAILURE;
		}
	}
	if( argc > 2 )
		iopl_level = atoi( argv[ 2 ] );
	do_read_devport( addr );
	do_ioperm( addr ); 
	do_iopl( addr );

	return errno;
}