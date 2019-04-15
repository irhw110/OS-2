#define SECTOR_SIZE 512
#define MAX_SECTORS 16

int strlen(char* s);
int div(int a, int b);

int main() {
	char curdir, argc, res;
	char argv[16];
	char buff[SECTOR_SIZE * MAX_SECTORS];
	char isifile[128];
	int i,j,len;

	interrupt(0x21, 0x21, &curdir, 0, 0);
	// membaca banyak argumen
	interrupt(0x21, 0x22, &argc, 0, 0);
	// membaca nama file
	interrupt(0x21, 0x23, 0, argv, 0);


	if (argc == 2) {	// menuliskan isi file
		// baca input user

		interrupt(0x21, 0x00, "Masukan teks: ", 0, 0);
		interrupt(0x21, 0x01, isifile, 0, 0);
		for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++) {
				buff[i] = 0;
		}
		i = 0;
		j = 0;
		while (isifile[i] != '\0') {
				buff[j] = isifile[i];
				j++;
				i++;
		}
		buff[j] = 0;
		j++;
		len= div(j, SECTOR_SIZE) + 1;
		interrupt(0x21, curdir << 8 | 0x05, isifile, argv, &len);

    /*
		if (res == 1) {
			interrupt(0x21, 0x07, &res, 0, 0);
			//interrupt(0x21, 0x00, "File berhasil ditulis\r\n", 0, 0);
		}
		else interrupt(0x21, 0x00, "Gagal menulis file\r\n", 0, 0); */
	}
	else
	if (argc == 1) {
		//baca
		interrupt(0x21, (curdir << 8) | 0x04, isifile, argv, &res);
		// print isi ke layar
		if (res == 0) {
			interrupt(0x21, 0x00, isifile, 0, 0);
			interrupt(0x21, 0x00, "\r\n", 0, 0);
		}
		else interrupt(0x21, 0x00, "Gagal membaca file\r\n", 0, 0);
	}
	interrupt(0x21, 0x07, &res, 0, 0);

	return 0;
}

int div(int a, int b){
  int q = 0;
   while(q*b <= a) {
      q = q+1;
   }
   return q-1;
}

int strlen(char* s){
	int i = 0;
	while (s[i] != 0){
		i++;
	}
	return i;
}
