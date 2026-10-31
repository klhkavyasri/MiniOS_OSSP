#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

void display_menu() {
    printf("\n=== ForgeShell Memory-Mapped File Manager ===\n");
    printf("1. View File Content (Memory Read via Pointer)\n");
    printf("2. Modify Byte at Offset (Memory Write)\n");
    printf("3. Flush Modifications to Disk (msync)\n");
    printf("4. Unmap and Exit (munmap)\n");
    printf("Enter choice: ");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filepath = argv[1];
    int fd = open(filepath, O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Error getting file size");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (sb.st_size == 0) {
        fprintf(stderr, "Error: Cannot map an empty file.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    size_t length = sb.st_size;
    
    char *addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    int choice;
    size_t offset;
    char new_char;

    while (1) {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Terminating.\n");
            break;
        }

        switch (choice) {
            case 1:
                printf("\n--- Mapped Region Content (Length: %zu bytes) ---\n", length);
                fwrite(addr, sizeof(char), length, stdout);
                printf("\n------------------------------------------------\n");
                break;

            case 2:
                printf("Enter byte offset (0 to %zu): ", length - 1);
                if (scanf("%zu", &offset) != 1 || offset >= length) {
                    printf("Error: Offset exceeds allocated boundary.\n");
                    break;
                }
                printf("Enter replacement character: ");
                scanf(" %c", &new_char);

                addr[offset] = new_char;
                printf("Offset %zu successfully modified to '%c'.\n", offset, new_char);
                break;

            case 3:
                if (msync(addr, length, MS_SYNC) == -1) {
                    perror("Error synchronizing file to disk");
                } else {
                    printf("Success: Memory regions flushed to persistent disk (MS_SYNC).\n");
                }
                break;

            case 4:
                if (munmap(addr, length) == -1) {
                    perror("Error unmapping memory");
                    exit(EXIT_FAILURE);
                }
                printf("Resource released via munmap(). Process terminating gracefully.\n");
                exit(EXIT_SUCCESS);

            default:
                printf("Invalid option. Please try again.\n");
        }
    }

    munmap(addr, length);
    return 0;
}
