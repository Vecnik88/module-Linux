#include <sys/stat.h>
#include <errno.h>
#include "common.h"

/* asmlinkage long sys_init_module					<---. системный вызов sys_init_module
				( void __user* umod, unsigned long len, const char __user* uargs ) */

int main( int argc, char const *argv[] )
{
	char parms[ BUF_MES ] = "";
	char file[ BUF_MES ] = SLAVE_FILE;
	void* buff = NULL;
	int fd, res;
	off_t fsize;						// <---. общий размер в байтах

	if( argc > 2 ){
		int i;
		for( i = 2; i < argc; ++i ){
			strcat( parms, argv[ i ] );
			strcat( parms, " " );
		}
	}

	printf("Download module: %s %s\n", file, parms );
	fd = open( file, O_RDONLY );
	if( fd < 0 ){
		printf("ошибка open: %m\n" );
		
		return errno;
	}
	{ struct stat fst;
		if( fstat( fd, &fst ) < 0 ){
			printf("Error stat: %m\n" );
			close( fd );
			return errno;
		}

	if( !S_ISREG( fst.st_mode ) ){
		printf("Error %s = no file\n", file );
		close( fd );
		return EXIT_FAILURE;
	}
	fsize = fst.st_size;
}

printf("Size module %s = %ls byte\n", file, fsize );
buff = malloc( fsize );

if( buff == NULL ){
	printf("Error malloc: %m\n" );
	close( fd );
	return errno;
}
	if( fsize != read( fd, buff, fsize ) ){
		printf("Error read^ %m\n");
		free( buff );
		close( fd );
		return errno;
	}

	close( fd );
	res = syscall( __NR_init_module, buff, fsize, parms );
	free( buff );
	if( res < 0 )
		printf("Error download module %m\n" );
	else
		printf("Module %s download!\n", file );

	return res;
}





