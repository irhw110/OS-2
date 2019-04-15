#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define MAX_DIRS 32
#define MAX_FILES 32
#define ENTRY_LENGTH 16
#define SECTOR_SIZE 512
#define MAX_FILENAME 15

void main() {
    char files[SECTOR_SIZE]; char buffer[MAX_FILENAME + 1]; char dirs[SECTOR_SIZE];
    char argv[4][16];
    int* result;
    int i, j;
    char curdir;
    int succ;

    interrupt(0x21, 0x02, files, FILES_SECTOR);
    interrupt(0x21, 0x02, dirs, DIRS_SECTOR);

    //baca parent dir
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x0, "Daftar file:\r\n", 0, 0);
    for (i = 0; i < MAX_FILES; i++) {
        if ((files[i * ENTRY_LENGTH + 1] != '\0') && (files[i * ENTRY_LENGTH] == curdir)) {
            j = 0;
            while ((j < MAX_FILENAME) && (files[i * ENTRY_LENGTH + 1 + j] != '\0')) {
                buffer[j] = files[i * ENTRY_LENGTH + 1 + j];
                j++;
            }
            buffer[j] = '\0';
            interrupt(0x21, 0x0, buffer, 0, 0);
            interrupt(0x21, 0x0, "\r\n", 0, 0);
        }
    }

    interrupt(0x21, 0x0, "Daftar dir:\r\n", 0, 0);
    for (i = 0; i < MAX_DIRS; i++) {
        if ((dirs[i * ENTRY_LENGTH + 1] != '\0') && (dirs[i * ENTRY_LENGTH] == curdir)) {
            j = 0;
            while ((j < MAX_FILENAME) && (dirs[i * ENTRY_LENGTH + 1 + j] != '\0')) {
                buffer[j] = dirs[i * ENTRY_LENGTH + 1 + j];
                j++;
            }
            buffer[j] = '\0';
            interrupt(0x21, 0x0, buffer, 0, 0);
            interrupt(0x21, 0x0, "\r\n", 0, 0);
        }
    }    
    interrupt(0x21, 0x07, result, 0, 0);
    //return 0;
}
