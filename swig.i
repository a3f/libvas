%module libvas
%{
#include <vas.h>
%}

extern vas_t *vas_open(pid_t, int);
extern int vas_read(vas_t *, const vas_addr_t, void *, size_t);
extern int vas_write(vas_t *, vas_addr_t, const void *, size_t);
extern void vas_close(vas_t *);
