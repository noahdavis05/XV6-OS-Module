#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"   // for T_DIR


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
        
        // skip the . and .. entreis
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0){
            continue;
        }

         // Construct the full path manually
        char fullpath[512];
        strcpy(fullpath, dir); // Start with the directory path
        // Append '/' and the directory entry name
        int len = strlen(fullpath);
        fullpath[len] = '/';  // Add '/' to the path
        fullpath[len + 1] = '\0'; // Null-terminate the string
        strcat(fullpath, de.name); // Append the directory entry name
        // stat the entry to see if iss a directory
        if (stat(fullpath, &st) < 0){
            printf("Cannot stat %s\n", fullpath);
            continue;
        }

        // check if the current entry is the target file
        if (st.type == T_FILE && strcmp(de.name,filename) == 0){
            printf("%s\n", fullpath);
        }

        // if the current entry is a directory recurse into ti
        if (st.type == T_DIR){
            find(fullpath, filename);
        }

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

    // now check if the directory exists and is a directory
    struct stat st;
    if (stat(path, &st) < 0){
        printf("Cannot stat %s\n", path);
        return 1;
    }
    if (st.type != T_DIR){
        printf("%s is not a directory\n", path);
        return 1;
    }
    // now we need to search through these directories with a function

    return 0;
}