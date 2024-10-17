#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_RECORDS 500
#define BUF_SIZE 512

int count_lines_in_history(){
    char buf[BUF_SIZE];
    int fd, n, i, line_count = 0;

    // open file for reading
    fd = open("command_history.txt", O_RDONLY);
    if (fd < 0){
        printf("Error opening file command_history.txt\n");
        return -1;
    }

    // read the file and count lines
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                line_count++;  // Count the newlines as lines
            }
        }
    }

    // check if there was an error whilst reading
    if (n < 0) {
        printf("Error reading file\n");
        close(fd);
        return -1;
    }

    close(fd);
    return line_count;
}

int main(int argc, char *argv[]){
    // check the command line arguments
    if (argc > 2 ){
        printf("Too many arguments, only 1 expected!\n");
        exit(1);
    } else if (argc == 2){
        // get the value of argv[1] as this is the number of history to show
        int records = atoi(argv[1]);
        printf("Records to display: %d\n", records);

        // make an array of 500
        char record_array[MAX_RECORDS][BUF_SIZE];  // 500 records, each can store up to 512 characters
        int fd, n, i, line_count = 0;
        char buf[BUF_SIZE];

        // open file for reading
        fd = open("command_history.txt", O_RDONLY);
        if (fd < 0){
            printf("Error opening file command_history.txt\n");
            return -1;
        }

        // read the file and add lines to the record_array
        int record_index = 0;
        int start = 0;  // starting index for each line
        while ((n = read(fd, buf, sizeof(buf))) > 0 && record_index < MAX_RECORDS) {
            for (i = 0; i < n; i++) {
                if (buf[i] == '\n') {
                    // Copy the line into the record array
                    int line_length = i - start + 1;
                    if (line_count % 2 == 0 && record_index < MAX_RECORDS) {  // Skip every second line
                        // Copy characters manually
                        for (int j = 0; j < line_length; j++) {
                            record_array[record_index] = buf;
                        }
                        record_array[record_index][line_length] = '\0';  // Null-terminate the string
                        printf("Record %d: %s", record_index + 1, record_array[record_index]);
                        record_index++;
                    }
                    line_count++;
                    start = i + 1;  // Move start to the next character after newline
                }
            }
        }

        close(fd);

        

 

    } else {
        // If no arguments, display all records
        printf("Displaying all records\n");
        char buf[BUF_SIZE];
        int fd, n, i, line_count = 0;

        // open file for reading
        fd = open("command_history.txt", O_RDONLY);
        if (fd < 0){
            printf("Error opening file command_history.txt\n");
            return -1;
        }

        // read the file and count lines
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (line_count % 2 == 0){
                printf("%s", buf);
            }
            for (i = 0; i < n; i++) {
                if (buf[i] == '\n') {
                    line_count++;  // Count the newlines as lines
                }
            }
        }

        close(fd);
    }

    return 0;
}
