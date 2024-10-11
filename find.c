#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"   // for T_DIR

// function to concatonate 2 strings
void concatenate(char *dest, const char *src1, const char *src2) {
    // Copy src1 to dest
    while (*src1 != '\0') {
        *dest++ = *src1++;
    }
    // Copy src2 to dest
    while (*src2 != '\0') {
        *dest++ = *src2++;
    }
    // Null terminate the destination string
    *dest = '\0';
}


// my find function
void find(const char *dir, const char *filename){
    // open the directory
    int fd = open(dir,0);
    if (fd < 0) {
        printf("Cannot open directory %s\n", dir);
        return;
    }
    // use fstat to confirm it was a directory
    struct stat st;
    if (fstat(fd, &st) < 0){
        printf("Cannot stat %s\n", dir);
        close(fd);
        return;
    }

    // check its a directory
    if (st.type != T_DIR){
        printf("%s is not a directory\n", dir);
        close(fd);
        return;
    }

    // read the entries
    struct dirent de; // de is directory entry
    // iterate through the items in the directory
    while (read(fd, &de, sizeof(de)) == sizeof(de)){
        if (de.inum == 0)   continue;

         // Skip the . and .. entries
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
            continue;
        }

        // need to check if the directory entry is a file
        //get the full path
        char tempPath[300];
        concatenate(tempPath, dir, "/");
        char fullPath[500];
        concatenate(fullPath,tempPath, de.name );
        printf("Full Path %s\n", fullPath);
        // now i have the full path check if it is a directory
        struct stat st;
        if (stat(fullPath, &st) < 0){
            printf("Cannot stat %s\n", fullPath);
            continue;
        }

        // Check the type field in the stat structure
        if (st.type == T_DIR) {
            printf("Directory\n");
            printf("Recursion!!!\n");
            find(fullPath, filename); // Recursive call
            
            
        } 
        
        
        // skip the . and .. entreis
        if (strcmp(de.name, filename) == 0){
            printf("%s/%s\n",dir,de.name);
        }

        printf("%s/%s inum=%d\n",dir,de.name, de.inum);
        

    }
    close(fd);

}


int main(int argc, char *argv[]){
    // check argc and argv
    // should be the directory / path and the target filename
    if (argc != 3) {
        printf("Directory path and target filename are required!\n");
        return 1;
    }
    char *path = argv[1];       // Use pointer to string
    char *filename = argv[2];   // Use pointer to string

    printf("Path %s filename %s \n", path, filename);

    find(path, filename);

    return 0;
}