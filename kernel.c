#define SECTOR_SIZE 512
#define MAP_SECTOR 256
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define SECTORS_SECTOR 259
#define ARGS_SECTOR 512
#define DIRS_ENTRY_LENGTH 16
#define FILES_ENTRY_LENGTH 16
#define SECTORS_ENTRY_LENGTH 16
#define MAX_DIRS 32
#define MAX_FILES 32
#define MAX_SECTORS 32
#define EMPTY 0x00
#define USED 0xFF
#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define NOT_FOUND -1
#define ALREADY_EXISTS -2
#define INSUFFICIENT_MEMORY -3
#define NAME_OFFSET 1
#define ENTRY_LENGTH 16
#define MAX_ENTRIES 32
#define MAX_FILENAME 15
#define MAX_BYTE 256
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_SEGMENTS -1
#define INSUFFICIENT_DIR_ENTRIES -1

#define VIDEO_SEGMENT 0xB000
#define BASE 0x8000
#define SCREEN_SIZE 25 * 80 * 2

#define BLACK 0x00
#define BLUE 0x01
#define GREEN 0x02
#define CYAN 0x03
#define RED 0x04
#define MAGENTA 0x05
#define BROWN 0x06
#define LIGHT_GRAY 0x07
#define LIGHT_BLACK 0x08
#define LIGHT_BLUE 0x09
#define LIGHT_GREEN 0x0A
#define LIGHT_CYAN 0x0B
#define LIGHT_RED 0x0C
#define LIGHT_MAGENTA 0x0D
#define YELLOW 0x0E
#define WHITE 0x0F

void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int segment, int *result, char parentIndex);
void terminateProgram(int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void putArgs(char curdir, char argc, char **argv);
void getCurdir(char *curdir);
void getArgc(char *argc);
void getArgv(char index, char *argv);

int strcmp(char *s1, char *s2);
void printInt(int i);
int searchUnusedSector(char *map);
int searchUnusedEntry(char *entries);
char searchPath(char *path, char parentIndex);
int searchFile(char *filename, char dir_index);

int main()
{
    char sec[10];

    makeInterrupt21();

    interrupt(0x21, 0xFF << 8 | 0x6, "shell", 0x2000, sec);

    while (1)
        ;
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
    char AL, AH;
    AL = (char)(AX);
    AH = (char)(AX >> 8);
    switch (AL)
    {
    case 0x00:
        printString(BX);
        break;
    case 0x01:
        readString(BX);
        break;
    case 0x02:
        readSector(BX, CX);
        break;
    case 0x03:
        writeSector(BX, CX);
        break;
    case 0x04:
        readFile(BX, CX, DX, AH);
        break;
    case 0x05:
        writeFile(BX, CX, DX, AH);
        break;
    case 0x06:
        executeProgram(BX, CX, DX, AH);
        break;
    case 0x07:
        terminateProgram(BX);
        break;
    case 0x08:
        makeDirectory(BX, CX, AH);
        break;
    case 0x09:
        deleteFile(BX, CX, AH);
        break;
    case 0x0A:
        deleteDirectory(BX, CX, AH);
        break;
    case 0x20:
        putArgs(BX, CX, DX);
        break;
    case 0x21:
        getCurdir(BX);
        break;
    case 0x22:
        getArgc(BX);
        break;
    case 0X23:
        getArgv(BX, CX);
        break;
    default:
        printString("Invalid interrupt");
    }
}

void printString(char *string)
{
    int string_iterator = 0;
    while (string[string_iterator] != '\0')
    {
        interrupt(0x10, 0xE00 + string[string_iterator], 0, 0, 0);
        string_iterator++;
    }
}
void readString(char *string)
{
    int string_iterator = 0;
    char c;
    do
    {
        c = interrupt(0x16, 0, 0, 0, 0);
        if (c == '\b')
        {
            interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
            interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
            interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
            string_iterator--;
        }
        else if (c != '\r')
        {
            string[string_iterator] = c;
            interrupt(0x10, 0xE00 + c, 0, 0, 0);
            string_iterator++;
        }
    } while (c != '\r');

    string[string_iterator] = '\0';
    interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
    interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
}
int mod(int a, int b)
{
    while (a >= b)
    {
        a = a - b;
    }
    return a;
}
int div(int a, int b)
{
    int q = 0;
    while (q * b <= a)
    {
        q = q + 1;
    }
    return q - 1;
}
void readSector(char *buffer, int sector)
{
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}
void writeSector(char *buffer, int sector)
{
    interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}
void readFile(char *buffer, char *path, int *result, char parentIndex)
{
    char sectors[SECTOR_SIZE];
    int dirs_iterator = 0;
    int dirsname_iterator = 0;
    int filename_idx = 0;
    int files_iterator = 0;
    int sectors_iterator = 0;

    readSector(sectors, SECTORS_SECTOR);
    while (path[dirsname_iterator] != '\0')
    {
        if (path[dirsname_iterator] == '/')
            filename_idx = dirsname_iterator + 1;
        dirsname_iterator++;
    }
    dirsname_iterator = 0;

    if (filename_idx != 0)
    {
        path[filename_idx - 1] = '\0';
        dirs_iterator = searchPath(path, parentIndex);
        if (dirs_iterator == 0xFE)
        {
            *result = NOT_FOUND;
            return;
        }
    }
    else
    {

        dirs_iterator = parentIndex;
    }

    files_iterator = searchFile(path + filename_idx, dirs_iterator);
    if (files_iterator == MAX_FILES)
    {
        *result = NOT_FOUND;
        return;
    }

    while (sectors[files_iterator * SECTORS_ENTRY_LENGTH + sectors_iterator] != '\0' && sectors_iterator < SECTORS_ENTRY_LENGTH)
    {
        readSector(buffer + sectors_iterator * SECTOR_SIZE, sectors[files_iterator * SECTORS_ENTRY_LENGTH + sectors_iterator]);
        sectors_iterator++;
    }
    *result = files_iterator;
}
void clear(char *buffer, int length)
{
    int i;
    for (i = 0; i < length; ++i)
    {
        buffer[i] = EMPTY;
    }
}
void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
    char map[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE];
    int map_iterator = 0;
    int dirs_iterator = 0;
    int dirsname_iterator = 0;
    int files_iterator = 0;
    int filesname_iterator = 0;
    int empty_files_index = 0;
    int sectors_iterator = 0;
    int empty_sector = 0;
    int filename_idx = 0;

    readSector(sectors, SECTORS_SECTOR);
    readSector(files, FILES_SECTOR);
    readSector(map, MAP_SECTOR);
    map_iterator = searchUnusedSector(map);

    if (map_iterator == NOT_FOUND)
    {
        *sectors = INSUFFICIENT_MEMORY;
        return;
    }

    for (empty_files_index = 0; empty_files_index < MAX_FILES; ++empty_files_index)
    {
        if (files[empty_files_index * DIRS_ENTRY_LENGTH + 1] == '\0')
            break;
    }

    if (files_iterator == MAX_FILES)
    {
        *sectors = INSUFFICIENT_MEMORY;
        return;
    }

    while (path[dirsname_iterator] != '\0')
    {
        if (path[dirsname_iterator] == '/')
            filename_idx = dirsname_iterator + 1;
        dirsname_iterator++;
    }
    dirsname_iterator = 0;

    if (filename_idx != 0)
    {
        path[filename_idx - 1] = '\0';
        dirs_iterator = searchPath(path, parentIndex);
        if (dirs_iterator == 0xFE)
        {
            *sectors = NOT_FOUND;
            return;
        }
    }
    else
    {

        dirs_iterator = parentIndex;
    }

    files_iterator = searchFile(path + filename_idx, dirs_iterator);
    if (files_iterator != -1)
    {
        *sectors = ALREADY_EXISTS;
        return;
    }

    files[empty_files_index * FILES_ENTRY_LENGTH] = (char)dirs_iterator;
    dirsname_iterator = 0;
    while (path[filename_idx + dirsname_iterator] != '\0')
    {
        files[empty_files_index * FILES_ENTRY_LENGTH + dirsname_iterator + 1] = path[filename_idx + dirsname_iterator];
        ++dirsname_iterator;
    }

    while (buffer[sectors_iterator * SECTOR_SIZE] != '\0')
    {
        empty_sector = searchUnusedSector(map);
        writeSector(buffer + sectors_iterator * SECTOR_SIZE, empty_sector);
        sectors[empty_files_index * SECTORS_ENTRY_LENGTH + sectors_iterator] = empty_sector;
        map[empty_sector] = 0xFF;
        sectors_iterator++;
    }

    writeSector(map, MAP_SECTOR);
    writeSector(files, FILES_SECTOR);
    writeSector(sectors, SECTORS_SECTOR);
    *sectors = SUCCESS;
}

void executeProgram(char *path, int segment, int *result, char parentIndex)
{
    char buffer[SECTOR_SIZE * MAX_SECTORS];
    int exe_iterator;

    readFile(buffer, path, result, parentIndex);

    if (*result)
    {
        for (exe_iterator = 0; exe_iterator < (MAX_SECTORS * SECTOR_SIZE); exe_iterator++)
        {
            putInMemory(segment, exe_iterator, buffer[exe_iterator]);
        }
        launchProgram(segment);
    }
}
void terminateProgram(int *result)
{
    char shell[6];
    shell[0] = 's';
    shell[1] = 'h';
    shell[2] = 'e';
    shell[3] = 'l';
    shell[4] = 'l';
    shell[5] = '\0';
    executeProgram(shell, 0x2000, result, 0xFF);
}

void makeDirectory(char *path, int *result, char parentIndex)
{
    char dirs[SECTOR_SIZE];
    char sectors[SECTOR_SIZE];
    int i;
    int dirs_iterator = 0;
    int dirsNameOffset = 0;
    int target_dir_idx = 0;
    int unusedEntry;
    int toBeCreatedOffset = 0;

    readSector(dirs, DIRS_SECTOR);

    unusedEntry = searchUnusedEntry(dirs);
    if (unusedEntry == NOT_FOUND)
    {
        *result = -3;
        return;
    }

    while (path[dirsNameOffset] != '\0')
    {
        if (path[dirsNameOffset] == '/')
        {
            target_dir_idx = dirsNameOffset + 1;
        }
        dirsNameOffset += 1;
    }

    if (target_dir_idx != 0)
    {
        path[target_dir_idx - 1] = '\0';
        dirs_iterator = searchPath(path, parentIndex);
        if (dirs_iterator == 0xFE)
        {
            *result = NOT_FOUND;
            return;
        }
    }
    else
    {

        dirs_iterator = parentIndex;
    }

    toBeCreatedOffset = searchPath(path + target_dir_idx, dirs_iterator);
    if (toBeCreatedOffset != 0xFE)
    {
        *result = ALREADY_EXISTS;
        return;
    }

    dirs[unusedEntry * DIRS_ENTRY_LENGTH] = dirs_iterator;
    i = 1;
    for (dirsNameOffset = target_dir_idx; path[dirsNameOffset] != '\0'; ++dirsNameOffset)
    {
        dirs[unusedEntry * DIRS_ENTRY_LENGTH + i] = path[dirsNameOffset];
        i++;
    }
    writeSector(dirs, DIRS_SECTOR);
    *result = SUCCESS;
}
void deleteFile(char *path, int *result, char parentIndex)
{
    char map[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE];
    int dirs_iterator = 0, dirsname_iterator = 0, filename_idx = 0;
    int files_iterator = 0;
    int sectors_iterator = 0;

    readSector(map, MAP_SECTOR);
    readSector(sectors, SECTORS_SECTOR);
    readSector(files, FILES_SECTOR);

    while (path[dirsname_iterator] != '\0')
    {
        if (path[dirsname_iterator] == '/')
            filename_idx = dirsname_iterator + 1;
        dirsname_iterator++;
    }

    if (filename_idx != 0)
    {
        path[filename_idx - 1] = '\0';
        dirs_iterator = searchPath(path, parentIndex);
        if (dirs_iterator == 0xFE)
        {
            *result = NOT_FOUND;
            return;
        }
    }
    else
    {
        dirs_iterator = parentIndex;
    }

    files_iterator = searchFile(path + filename_idx, dirs_iterator);
    if (files_iterator == MAX_FILES)
    {
        *result = NOT_FOUND;
        return;
    }

    sectors_iterator = 0;
    while (sectors[files_iterator * SECTORS_ENTRY_LENGTH + sectors_iterator] != '\0')
    {
        map[sectors[files_iterator * SECTORS_ENTRY_LENGTH + sectors_iterator]] = 0x00;
        sectors_iterator++;
    }

    files[files_iterator * FILES_ENTRY_LENGTH + 1] = '\0';

    writeSector(map, MAP_SECTOR);
    writeSector(files, FILES_SECTOR);
    writeSector(sectors, SECTORS_SECTOR);
    *result = SUCCESS;
}
void deleteDirectory(char *path, int *success, char parentIndex)
{
    char dirs[SECTOR_SIZE], files[SECTOR_SIZE];
    int dirdel_iterator = 0;
    int dirs_iterator = 0;
    int files_iterator = 0;
    char *delresult;

    readSector(dirs, DIRS_SECTOR);
    readSector(files, FILES_SECTOR);

    dirs_iterator = searchPath(path, parentIndex);
    if (dirs_iterator == 0xFE)
    {
        *success = NOT_FOUND;
        return;
    }

    for (files_iterator = 0; files_iterator < MAX_FILES; ++files_iterator)
    {
        if (files[files_iterator * FILES_ENTRY_LENGTH] == dirs_iterator && files[files_iterator * FILES_ENTRY_LENGTH + 1] != '\0')
        {
            deleteFile(files + files_iterator * FILES_ENTRY_LENGTH + 1, delresult, dirs_iterator);
        }
    }

    for (dirdel_iterator = 0; dirdel_iterator < MAX_FILES; ++dirdel_iterator)
    {
        if (dirs[dirdel_iterator * DIRS_ENTRY_LENGTH] == dirs_iterator && dirs[dirdel_iterator * DIRS_ENTRY_LENGTH + 1] != '\0')
        {
            deleteDirectory(dirs + dirdel_iterator * DIRS_ENTRY_LENGTH + 1, delresult, dirs_iterator);
            readSector(dirs, DIRS_SECTOR);
        }
    }

    dirs[dirs_iterator * DIRS_ENTRY_LENGTH + 1] = '\0';
    writeSector(dirs, DIRS_SECTOR);
    *success = SUCCESS;
}

void putArgs (char curdir, char argc, char **argv) {
   char args[SECTOR_SIZE];
   int i, j, p;

   clear(args, SECTOR_SIZE);

   args[0] = curdir;
   args[1] = argc;
   i = 0;
   j = 0;
   for (p = 2; p < ARGS_SECTOR && i < argc; ++p) {
      args[p] = argv[i][j];
      if (argv[i][j] == '\0') {
         ++i;
         j = 0;
      }
      else {
         ++j;
      }
   }

   writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
   char args[SECTOR_SIZE];
   readSector(args, ARGS_SECTOR);
   *curdir = args[0];
}

void getArgc (char *argc) {
   char args[SECTOR_SIZE];
   readSector(args, ARGS_SECTOR);
   *argc = args[1];
}

void getArgv (char index, char *argv) {
   char args[SECTOR_SIZE];
   int i, j, p;
   readSector(args, ARGS_SECTOR);

   i = 0;
   j = 0;
   for (p = 2; p < ARGS_SECTOR; ++p) {
      if (i == index) {
         argv[j] = args[p];
         ++j;
      }

      if (args[p] == '\0') {
         if (i == index) {
            break;
         }
         else {
         ++i;
         }
      }
   }
} 



//STRING

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

//FILE
int searchUnusedSector(char *map)
{
    int i;
    for (i = 0; i < MAX_BYTE; ++i)
    {
        if (map[i] == 0x00)
        {
            return i;
        }
    }
    return NOT_FOUND;
}

int searchUnusedEntry(char *entries)
{
    int i;
    for (i = 0; i < MAX_ENTRIES; ++i)
    {
        if (entries[i * ENTRY_LENGTH + NAME_OFFSET] == '\0')
        {
            return i;
        }
    }
    return NOT_FOUND;
}

char searchPath(char *path, char parentIndex)
{
    char dirs[SECTOR_SIZE], cur_parent;
    int dirs_iterator = 0, dirsname_iterator = 0, dirsname_iterator_chkp = 0, found = FALSE;
    interrupt(0x21, 0x2, dirs, DIRS_SECTOR, 0);
    dirsname_iterator = 0;
    cur_parent = parentIndex;
    do
    {
        found = FALSE;

        do
        {

            if (dirs[dirs_iterator * DIRS_ENTRY_LENGTH] == cur_parent)
            {

                found = TRUE;
                for (dirsname_iterator = 0; dirsname_iterator <= MAX_FILES && path[dirsname_iterator_chkp + dirsname_iterator] != '/' && path[dirsname_iterator_chkp + dirsname_iterator] != '\0'; dirsname_iterator++)
                {
                    if (dirs[(dirs_iterator * DIRS_ENTRY_LENGTH) + dirsname_iterator + 1] != path[dirsname_iterator_chkp + dirsname_iterator])
                    {
                        found = FALSE;
                        ++dirs_iterator;
                        break;
                    }
                }
            }
            else
            {
                dirs_iterator++;
            }
        } while (!found && dirs_iterator < MAX_ENTRIES);

        if (!found)
        {
            return 0xFE;
        }
        dirsname_iterator_chkp += dirsname_iterator + 1;
        cur_parent = dirs_iterator;
    } while (path[dirsname_iterator_chkp - 1] != '\0');
    return cur_parent;
}

int searchFile(char *filename, char dir_index)
{
    char files[SECTOR_SIZE];
    int files_iterator = 0, filesname_iterator = 0;
    interrupt(0x21, 0x2, files, FILES_SECTOR, 0);
    do
    {

        if (files[files_iterator * FILES_ENTRY_LENGTH] == dir_index)
        {

            if (strcmp(files + files_iterator * FILES_ENTRY_LENGTH + 1, filename))

                break;
        }
        files_iterator++;
    } while (files_iterator < MAX_FILES);
    return files_iterator == MAX_FILES ? -1 : files_iterator;
}
