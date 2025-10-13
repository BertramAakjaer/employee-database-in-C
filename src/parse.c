#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"


int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {

    dbhdr->count++;
    struct employee_t *new_employees = realloc(*employees, dbhdr->count*(sizeof(struct employee_t)));

    if (new_employees == NULL)
    {
        return STATUS_ERROR;
    }

    *employees = new_employees; 

    char *name = strtok(addstring, ",");
char *addr = strtok(NULL, ",");
char *hours_str = strtok(NULL, ",");

// CHECK 1: Ensure all required tokens were found
if (name == NULL || addr == NULL || hours_str == NULL) {
    // Optionally: realloc to undo the count increment and realloc
    dbhdr->count--;
    // Consider reallocating to the previous size if needed,
    // or just return an error and expect the caller to clean up.
    return STATUS_ERROR; // Malformed input string
}

// Check 2: strncpy and atoi/strtol use the non-NULL pointers
// strncpy is used correctly to prevent overflow, but ensure manual NULL termination:
size_t index = dbhdr->count - 1;

strncpy((*employees)[index].name, name, sizeof((*employees)[index].name) - 1);
(*employees)[index].name[sizeof((*employees)[index].name) - 1] = '\0'; // Manual null-termination

strncpy((*employees)[index].address, addr, sizeof((*employees)[index].address) - 1);
(*employees)[index].address[sizeof((*employees)[index].address) - 1] = '\0'; // Manual null-termination

(*employees)[index].hours = atoi(hours_str); 

return STATUS_SUCCESS;
}



int read_emplotees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int count = dbhdr->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL)
    {
        printf("Malloc failed\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    for (int i = 0; i < count; i++)
    {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}





int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {

    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int realcount = dbhdr->count;

    dbhdr->version = htons(dbhdr->version);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);

    lseek(fd, 0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));

    for (int i = 0; i < realcount; i++)
    {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}	




int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }


    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to a create db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t))
    {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if (header->version != 0x1) {
        printf("Improper header version\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size)
    {
        printf("Corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }


    *headerOut = header;
    return STATUS_SUCCESS;
}




int create_db_header(struct dbheader_t **headerOut) {
    if (headerOut == NULL) {
        return STATUS_ERROR; 
    }

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to a create db header\n");
        return STATUS_ERROR;
    }

    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;
    return STATUS_SUCCESS;
}