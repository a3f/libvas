#ifndef LIBVAS_VAS_H_
#define LIBVAS_VAS_H_

#include <pid.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

/**
 * References a virtual address space
 */
typedef struct vas_t vas_t;
/**
 * Unsigned data type.
 * References a _data_ address in a virtual address space
 */
typedef uintptr_t vas_addr_t;

/**
 * Opens a handle to process id pid's address space
 */
vas_t *vas_open(pid_t, int flags);

/**
 * Closes a handle opened by vas_open and frees resources
 */
void vas_close(vas_t *);

/**
 * Reads size_t bytes from vas_t at vas_addr_t and
 * saves them to void*
 */
ssize_t vas_read(vas_t*, const vas_addr_t, void*, size_t);

/**
 * Write size_t to from vas_t at vas_addr_t from
 * void*
 */
ssize_t vas_write(vas_t*, vas_addr_t, const void*, size_t);

/**
 * Effecient memory searching
 */
int vas_search(vas_t*, const vas_addr_t, const void*, size_t);

/**
 * atomically reading multiple values
 * Is this even viable?
 */
struct vas_addrvec {
    vas_addr_t remote;
    void *local;
    size_t size;
};

ssize_t vas_readv(vas_t*, const struct vas_addrvec[], int vec_len);

ssize_t vas_writev(vas_t*, const struct vas_addrvec[], int vec_len);

/**
 */
typedef struct vas_poll_t vas_poll_t;

vas_poll_t *vas_poll_new(vas_t*, vas_addr_t, size_t, int);
int vas_poll(vas_poll_t*, void*);
void vas_poll_del(vas_poll_t*);


void *vas_fmap(const char *);

/**
 * Duplicate
 */
vas_addr_t vas_dup(vas_t*, const vas_addr_t, size_t);

/**
 * Copy on write specified memory block(s)
 */
void *vas_cow(vas_t*, vas_addr_t, const vas_addr_t, size_t);


#endif
