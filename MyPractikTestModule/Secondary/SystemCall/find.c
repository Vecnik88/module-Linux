static void* find_sym( const char* sym ){
	static unsigned long faddr = 0;
	
	/* вложенная функция - расширение GCC */
	int symb_fn( void* data, const char* sym, struct module* mod, unsigned long addr ){
		if( strcmp( ( char* )data, sym ) == 0 ){
			faddr = addr;
			return 1;
		} else {
			return 0;
		}
	}

	kallsyms_on_each_symbol( symb_fn, ( void* )sym );
	return ( void* )faddr;
}
