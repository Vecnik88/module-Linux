//---------------------------------------------

static char *sj( void ) {    // метка времени
   static char s[ 40 ];
   sprintf( s, "%08ld : ", jiffies );
   return s;
}

static char *st( int lvl ) { // метка потока
   static char s[ 40 ];
   sprintf( s, "%skthread [%05d:%d]", sj(), current->pid, lvl );
   return s;
}

//---------------------------------------------
