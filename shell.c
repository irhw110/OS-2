#define MAX_ENTRIES 32
#define DIRS_SECTOR 0x101
#define SECTOR_SIZE 512

int getParentIndex(char *path,char parentIndex,int sector);

int strlen(char* s);

int strcmp(char* s1, char* s2) ;


int main() {
	char input [100];
	char cmd[10];
	int dir = 0xFF; //inisialisasi parent
	//char argv[4][16];
	char**argv;
	char temp[16];
	int i,j;
	int argc;
	int temppath = dir;
	int result;
	while (1) {
		//Baca input pengguna
		for (i = 0; i<10;i++) cmd[i] = 0;
		for (i = 0; i<100;i++) input[i] = 0;
		for (i = 0; i<4 ;i++) for (j = 0; j<16 ;j++) argv[i][j] = 0;
		for (i = 0; i<16 ;i++) temp[i] = 0;
		do {
            interrupt(0x21, 0x00, "$ ", 0, 0);
            interrupt(0x21, 0x01, input, 0, 0);
 		}
 		while (strcmp(input,""));

		//masukkan ke dalam cmd
		i = 0;
		while (input[i] != 0 && input[i] != ' ') {
			cmd[i] = input[i];
			i++; //iterate
		}
		cmd[i] = 0; //terminate

		//masukkan ke dalam argv dan argc
		argc = 0;
		j = 0;
		while (input[i] != '\0') {
            if (input[i] ==' ') {
				          if (argc>0) {
                    argv[argc-1][j] = 0;
                  } //terminate
                  argc++;
                  j = 0;
            }
            else {
				          argv[argc-1][j] = input[i];
				          j++;
			      }
			      i++;
    }
    argv[argc-1][j] = 0; //terminate

		temppath = dir;
		if (strcmp(cmd,"cd")) { //Search Dir
			i = 0; j = 0;
			while (argv[0][i] != '\0') {
				if (argv[0][i] == '/') {
					temp[j] = 0; //terminate string
					j = 0;
					//Instruksi path
					temppath = getParentIndex(temp, temppath,DIRS_SECTOR);
					if (temppath == -999){ //tidak ketemu
						interrupt(0x21,0,"Path tidak ditemukan\n\r",0,0);
					}
				}
				else {
					temp[j] =  argv[0][i];
					j++;
				}
				i++;
			}//
			temp[j] = 0;
			temppath = getParentIndex(temp, temppath,DIRS_SECTOR);
			if (temppath == -999){ //tidak ketemu
				interrupt(0x21,0,"Path tidak ditemukan\r\n",0,0);
			}
			else {
				dir = temppath;
				//interrupt(0x21, 0x20, dir, argc, argv);
			}
		}
		// Baca dari program
		else {
			//geser
			if (cmd[0] == '.' && cmd[1] == '/') {
				i = 2;
				while (cmd[i] != 0) {
					cmd[i-2] = cmd;
					i++;
				}
				cmd[i] = 0;
			}
			interrupt(0x21, 0x20, dir, argc, argv);
      interrupt(0x21, (0xFF << 8) | 0x06, cmd, 0x2000, &result); //jalankan program
      if (result == -1) {
        interrupt(0x21, 0, "Program tidak ditemukan\r\n", 0, 0);
      }  // Not found
      else if  (result != 0) {
        interrupt(0x21, 0, "Program gagal dijalankan\r\n", 0, 0);
      }// Gagal
    }
	}
}


int getParentIndex(char *path,char parentIndex,int sector){
	char dirs[SECTOR_SIZE];
	int found = 0;
	int tdksama;
	int i = 0;
	int j;

	interrupt(0x21, 0x2, dirs, sector, 0);

	while (i<MAX_ENTRIES && !found) {
		//printString("\r\n");
		if (dirs[i*0x10] == parentIndex) { //ketemu
			tdksama = 0;
			j = 0;
			while (!tdksama && path[j] != '\0') {
				if (path[j] != dirs[i*0x10+j+1]) tdksama = 1;
				else j++;
			}
			if (!tdksama) found = 1;
			else i++;
		}
		else {
			i++;
		}
	}
	if (found) {
    return i;
  }
	else {
    return -999; 
  }//tidak ketemu
}

int strlen(char* s){
	int i = 0;
	while (s[i] != 0){
		i++;
	}
	return i;
}

int strcmp(char *s1, char *s2)
{
    int i = 0;
    while (!(s1[i] == '\0' && s2[i] == '\0'))
    {
        if (s1[i] != s2[i])
            return 0;
        i++;
    }
    return 1;
}
