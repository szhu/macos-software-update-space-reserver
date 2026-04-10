#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// Converts a size string (e.g., "20g", "500m", "1024") into bytes.
off_t parse_size(const char *sizeStr) {
    char *end;
    // Use strtoull to get the numeric part
    unsigned long long size = strtoull(sizeStr, &end, 10);
    if (size == 0 && end == sizeStr) {
        fprintf(stderr, "Invalid size: %s\n", sizeStr);
        exit(EXIT_FAILURE);
    }

    // Check for suffix letters
    if (*end) {
        switch (*end) {
            case 'g': case 'G':
                size *= 1024ULL * 1024ULL * 1024ULL;
                break;
            case 'm': case 'M':
                size *= 1024ULL * 1024ULL;
                break;
            case 'k': case 'K':
                size *= 1024ULL;
                break;
            // Optionally support bytes (or ignore other letters)
            default:
                fprintf(stderr, "Unknown size suffix: %c\n", *end);
                exit(EXIT_FAILURE);
        }
    }
    return (off_t) size;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <size> <filename>\nExample: %s 20g \"Reserved Space\"\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    off_t size = parse_size(argv[1]);
    const char *filename = argv[2];

    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open failed");
        return EXIT_FAILURE;
    }

    // Prepare preallocation structure for macOS
    fstore_t prealloc;
    prealloc.fst_flags = F_ALLOCATEALL; // Request allocation of all requested space
    prealloc.fst_posmode = F_PEOFPOSMODE;
    prealloc.fst_offset = 0;
    prealloc.fst_length = size;
    prealloc.fst_bytesalloc = 0; // not used here

    if (fcntl(fd, F_PREALLOCATE, &prealloc) == -1) {
        if (errno == ENOSPC) {
            fprintf(stderr, "fcntl F_PREALLOCATE failed: No space left on device\n");
            unlink(filename); // Remove the file if no space left on device
        } else {
            perror("fcntl F_PREALLOCATE failed");
        }
        close(fd);
        return EXIT_FAILURE;
    }

    if (ftruncate(fd, size) == -1) {
        perror("ftruncate failed");
        close(fd);
        return EXIT_FAILURE;
    }

    // printf("Successfully allocated %lld bytes in file '%s'\n", (long long)size, filename);
    close(fd);
    return EXIT_SUCCESS;
}
