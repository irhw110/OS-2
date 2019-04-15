
void main() {
    char curdir;
    char argv[4][16];
    char argc;
    int i;

    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);

    for (i = 0; i<argc; i++) {
		    if(i>0) {
          interrupt(0x21, 0x0, " ", 0, 0);
        }
	      interrupt(0x21, 0x23, i, argv[i], 0);
		    interrupt(0x21, 0x0, argv[i], 0, 0);
	  }

    interrupt(0x21, 0x0, "\r\n", 0, 0);
    interrupt(0x21, 0x07, &i, 0, 0);
}
