#define NUM_ARGS 2
#define FILE_IDX 1
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>

void validate_args(int argc, char * argv[]);
FILE *open_file(char *name);
char *uint32_to_str(uint32_t i);
void *checked_malloc (size_t size);
void remove_trailing(char *str);
char *remove_leading(char *str);
void get_args(char *str, char *args[]);
void check_dir(char *argv[]);
void c_r_vinodes();
void arguments(char *aargs[]);
uint32_t f_cd(uint32_t cwd, char *arg, char *ichars);
int is_dir(int inode);
void f_ls(uint32_t cwd);
void f_mkdir(uint32_t cwd, char *arg);
void f_touch(uint32_t cwd, char *arg);
void print_inodes();
uint32_t open_i();
int checkdir(uint32_t cwd, char *dir);
int checkfile(uint32_t cwd, char *file);

int main(int argc, char * argv[]) {
    char *str = NULL;
    validate_args(argc, argv);
    // char *dir_name = NULL;
    
    get_args(str, argv);
    DIR* dir = opendir(argv[1]);
    if (dir) {
        chdir(argv[1]);
    }
    else {
        perror("directory does not exist");
        exit(1);
    }
    // ^ requirement 1 works
    // ^ requirement 2 works

    arguments(argv);

    // closedir(dir);
    // fclose(in);
    return 0;
}

void arguments(char *aargs[]) {
    // char *cwd;
    FILE *in = NULL;
    size_t n;
    char *str = NULL;
    char *cd = "cd";
    char *ls = "ls";
    char *mkdir = "mkdir";
    char *touch = "touch";
    char *exit = "exit";
    char ichars[] = "";
    uint32_t cwd = 0;


    c_r_vinodes(&ichars);
    // printf("ichars: %s \n", ichars);

    in = stdin;
    // *prev = cwd;
    while (strcmp(aargs[0], exit) != 0) {
        in = stdin;
        getline(&str, &n, in);
        str = remove_leading(str);
        remove_trailing(str);
        get_args(str, aargs);

        if(strcmp(aargs[0], cd) == 0) {
            cwd = f_cd(cwd, aargs[1], ichars);
            // pwd = cwd;
        }

        else if(strcmp(aargs[0], ls) == 0) {
            // printf("the CWD is: %s \n", cwd);
            f_ls(cwd);
        }

        else if(strcmp(aargs[0], mkdir) == 0) {
            if (strlen(aargs[1]) > 32){
                printf("name too long. try again \n");
            }
            else if (checkdir(cwd, aargs[1]) == 1) {
                printf("directory already exists \n");
            }
            else {
                f_mkdir(cwd, aargs[1]);
            }
        }

        else if(strcmp(aargs[0], touch) == 0) {
            if (strlen(aargs[1]) > 32){
                printf("name too long. try again \n");
            }
            else if (checkfile(cwd, aargs[1]) == 1) {
                printf("file already exists \n");
            }
            else {
                f_touch(cwd, aargs[1]);
            }
        }

        // printf("cwd: %s \n", cwd);
    }

    if(strcmp(aargs[0], exit) == 0) {
        fclose(in);
        return;
    }
}

void print_inodes(){
    FILE *iptr = NULL;
    int cidx = 0;
    int c;
    uint32_t ibuf[1024];
    char cbuf[1024];

    iptr = fopen("inodes_list", "r");

    while (fread(&ibuf, sizeof(uint32_t), 1, iptr) > 0) {
        printf("uint: %lu \n", (unsigned long) (ibuf[0]));
        fread(&cbuf, sizeof(char), 1, iptr);
        c = cbuf[cidx];
        printf("char: %c \n", c);
    }

    fclose(iptr);
}

int is_dir(int inode) {
    FILE *iptr = NULL;
    char c;
    int cidx = 0;
    uint32_t ibuf[1024];
    char cbuf[1024];
    int count = 0;

    iptr = fopen("inodes_list", "r");

    while (fread(&ibuf, sizeof(uint32_t), 1, iptr) > 0) {
        fread(&cbuf, sizeof(char), 1, iptr);
        c = cbuf[cidx];
        if (inode == count) {
            if (c != 'd'){
                return 1;
            }
        }
        count ++;
    }
    
    fclose(iptr);
    return 0;
}

uint32_t f_cd(uint32_t cwd, char *arg, char *ichars) {
    FILE *fileptr;
    char *string = NULL;
    // char *c;
    int iidx = 0;
    uint32_t ibuf[1024];
    char cbuf[1024];
    int eF = 1;
    int iD = 1;
    // int count = 0;
    uint32_t state;
    char *scwd;
    uint32_t x;
    char *item;

    scwd = uint32_to_str(cwd);

    fileptr = fopen(scwd, "r");
    state = cwd;
    // cwd = arg;
    // printf("cwd: %s \n", cwd);

    while (fread(&ibuf, sizeof(uint32_t), 1, fileptr) > 0) {
        x = ibuf[iidx];
        string = uint32_to_str(ibuf[iidx]);
        // printf("%s ", string);

        fread(&cbuf, sizeof(char), 32, fileptr);
        item = cbuf;
        // printf("c: %s\n", item);
        if ((strcmp(item, arg)) == 0) {
            eF = 0; // file exists!
            scwd = string; // set current working directory to string
            cwd = x;
        }
        // printf("%s \n", c);
        // count ++;
    }

    // printf("char: %s \n", ichars);
    if(eF == 0) {
        // printf("success \n");
        iD = is_dir(atoi(scwd));
        if(iD == 0){
            // printf("success \n");
        }
        else{
            cwd = state;
            printf("cannot change into non directory item \n");
        }
    }
    else {
        printf("directory doesn't exist \n");
        cwd = state; // restore the cwd after it changed into an invalid directory
    }

    fclose(fileptr);
    return cwd;
}  

void f_ls(uint32_t cwd) { 
    FILE *fileptr;
    char *string = NULL;
    char *c;
    int iidx = 0;
    uint32_t ibuf[1024];
    char cbuf[1024];
    char *scwd;

    scwd = uint32_to_str(cwd);

    // printf("cwd: %s \n", cwd);
    fileptr = fopen(scwd, "r");

    while (fread(&ibuf, sizeof(uint32_t), 1, fileptr) > 0) {
        string = uint32_to_str(ibuf[iidx]);
        printf("%s ", string);

        fread(&cbuf, sizeof(char), 32, fileptr);
        c = cbuf;
        printf("%s \n", c);
    }
    fclose(fileptr);
}

uint32_t open_i(){
    FILE *ptr;
    uint32_t ibuf[1024];
    char cbuf[1024];
    uint32_t count = 0;

    ptr = fopen("inodes_list", "r");
    // printf("FILE OPENED");

    while ((fread(&ibuf, sizeof(uint32_t), 1, ptr)) > 0) {
        fread(&cbuf, sizeof(char), 1, ptr);
        count ++;
    }
    fclose(ptr);
    return count;
}

int checkdir(uint32_t cwd, char *dir){
    FILE *ptr;
    uint32_t ibuf[1024];
    char cbuf[1024];
    char *scwd;

    scwd = uint32_to_str(cwd);

    ptr = fopen(scwd, "r");
    // printf("directory: %s \n", dir);

    while (fread(&ibuf, sizeof(uint32_t), 1, ptr) > 0) {
        // printf("%s ", string);

        fread(&cbuf, sizeof(char), 32, ptr);
        // c = cbuf;
        // printf("STRING: %s \n", cbuf);
        if ((strcmp(cbuf, dir)) == 0) {
            return 1;
        }
        // printf("%s \n", c);
        // count ++;
    }

    fclose(ptr);
    return 0;
}

int checkfile(uint32_t cwd, char *file) {
    FILE *ptr;
    uint32_t ibuf[1024];
    char cbuf[1024];
    char *scwd;

    scwd = uint32_to_str(cwd);

    ptr = fopen(scwd, "r");
    // printf("directory: %s \n", dir);

    while (fread(&ibuf, sizeof(uint32_t), 1, ptr) > 0) {
        // printf("%s ", string);

        fread(&cbuf, sizeof(char), 32, ptr);
        // c = cbuf;
        // printf("STRING: %s \n", cbuf);
        if ((strcmp(cbuf, file)) == 0) {
            return 1;
        }
        // printf("%s \n", c);
        // count ++;
    }

    fclose(ptr);
    return 0;
}

void f_touch(uint32_t cwd, char *arg) {
    FILE *iptr = NULL;
    uint32_t count;
    // char *stri;
    FILE *cp = NULL;
    char *scwd;
    FILE *new = NULL;
    char *stri;

    char f[] = "f";

    scwd = uint32_to_str(cwd);
    count = open_i();
    stri = uint32_to_str(count);

    iptr = fopen("inodes_list", "a");
    fwrite(&count, 1, sizeof(uint32_t), iptr);
    fwrite(f, 1, sizeof(char), iptr);
    fclose(iptr);

    cp = fopen(scwd, "a"); 
    fwrite(&count, 1, sizeof(uint32_t), cp);
    fwrite(arg, 32, sizeof(char), cp);
    fclose(cp);
    
    new = fopen(stri, "w");
    fwrite(arg, 32, sizeof(char), cp);
    fclose(new);
}

void f_mkdir(uint32_t cwd, char *arg) {
    FILE *iptr = NULL;
    uint32_t count;
    char *stri;
    FILE *new = NULL;
    FILE *cp = NULL;
    char *scwd;

    scwd = uint32_to_str(cwd);

    // prev = prev;

    char d[] = "d";
    // printf("string: %s \n", arg);

    count = open_i();
    stri = uint32_to_str(count);
    // printf("open inode: %s \n", stri);

    iptr = fopen("inodes_list", "a");
    // fseek(iptr, numbytes, SEEK_CUR);
    // printf("OPENED \n");

    // FIX STRI, SHOULD NOT BE A STRING, SHOULD BE AN UINT32_T INSTEAD
    
    fwrite(&count, 1, sizeof(uint32_t), iptr);
    fwrite(d, 1, sizeof(char), iptr);
    fclose(iptr);
    // print_inodes();
    
    // printf("CWD: %s \n", cwd);

    // writes the newly created directory into the file of the current directory
    cp = fopen(scwd, "a");
    fwrite(&count, 1, sizeof(uint32_t), cp);
    fwrite(arg, 32, sizeof(char), cp);
    fclose(cp);

    // populates the new directory
    // printf("STRI: %s \n", stri);
    new = fopen(stri, "w");
    // printf("here now \n");
    fwrite(&count, 1, sizeof(uint32_t), new);
    fwrite(".", 32, sizeof(char), new);

    fwrite(&cwd, 1, sizeof(uint32_t), new);
    fwrite("..", 32, sizeof(char), new);
    fclose(new);
}

void c_r_vinodes(char *ichars) {  // change and read and verify inode file
    FILE *fileptr;
    char *string = NULL;
    char c;
    int iidx = 0;
    int cidx = 0;
    uint32_t ibuf[1024];
    char cbuf[1024];

    fileptr = fopen("inodes_list", "r");


    while (fread(&ibuf, sizeof(uint32_t), 1, fileptr) > 0) {
        string = uint32_to_str(ibuf[iidx]);
        if ((atoi(string) > 1024) | (atoi(string) < 0)){
            printf("invalid inode \n");
        }
        else {
            // printf("uint: %s \n", string);
        }

        fread(&cbuf, sizeof(char), 1, fileptr);
        c = cbuf[cidx];
        // printf("char: %c \n", c);
        strncat(ichars, &c, 1);
        if((c != 'f') & (c != 'd')) {
            printf("invalid indicator \n");
        }
    }
}

void check_dir(char *argv[]) { // checks if the directory exists
    DIR* dir = opendir(argv[1]);
    if (dir) {
        chdir(argv[1]);
    }
    else {
        perror("directory does not exist");
        exit(1);
    }
}

void get_args(char *str, char *args[]) {  // get the name of the directory
    char *token;
    int x = 0;
    while ((token = strsep(&str, " ")) != NULL) {
        args[x] = token;
        x ++;
    }
}

void remove_trailing(char *str) { // remove trailing whitespaces
    ssize_t len = strlen(str);

    for(ssize_t idx = len - 1; idx >= 0 && isspace(str[idx]); idx --) {
        str[idx] = '\0';
    }
}

char *remove_leading(char *str) { // remove leading whitespaces
    int idx = 0;
    while((isspace(str[idx])) && (str[idx] != '\0')) {
        idx++;
    }
    return &str[idx];
}

char *uint32_to_str(uint32_t i) {
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);       // pretend to print to a string to get length
   char *str = checked_malloc(length + 1);                        // allocate space for the actual string
   snprintf(str, length + 1, "%lu", (unsigned long)i);            // print to string

   return str;
}

void *checked_malloc (size_t size) {
    int *p;
    int len = 100;
    p = malloc(sizeof(uint32_t) * len);
    if (p == NULL) {
        perror("malloc");
        exit(1);
    }
    return p;
}

void validate_args(int argc, char * argv[]) { // validate args
    if (argc > NUM_ARGS) {
        fprintf(stderr, "usage: %s file \n", argv[0]);
        exit(1);
    }
}

FILE *open_file(char *name) { // open file
    FILE *f = fopen(name, "r");
    if (f == NULL) {
        perror(name);
        exit(1);
    }
    return f;
}
