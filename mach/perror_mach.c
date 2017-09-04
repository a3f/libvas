/* probably reinventing the wheel */

#include <mach/kern_return.h>
#include <stdio.h>
#include <string.h>

#include "vas-internal.h"

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

#define INT_MIN_STR TOSTR(INT_MIN)

static const char *tostr(kern_return_t errnum);

/* TODO use snprintf with C99 */
int
vas_mach_strerror_r(kern_return_t errnum, char *strerrbuf, size_t buflen)
{
    const char *errmsg, *prefix;
    size_t len;

    if ((errmsg = tostr(errnum))) {
        len = strlen(errmsg);
        memcpy(strerrbuf, errmsg, MIN(buflen, len));
    } else {
        if (errnum > KERN_RETURN_MAX /* 100 */)
            prefix = "Return value exceeds maximum allowable: ";
        else
            prefix = "Illegal error code: ";

        len = strlen(prefix);
        memcpy(strerrbuf, prefix, MIN(buflen, len));
        len += sizeof INT_MIN_STR;
        if (len  <= buflen)
            sprintf(strerrbuf, "%d", errnum);
    }

    return len <= buflen ? 0 : -1;
}

const char *
vas_mach_strerror(kern_return_t errnum)
{
    const char *errmsg;
    static char errbuf[576];

    if ((errmsg = tostr(errnum)))
        return errmsg;

    vas_mach_strerror_r(errnum, errbuf, sizeof errbuf);
    return errbuf;
}

void
vas_mach_perror(kern_return_t errnum, const char *usermsg)
{
    const char *errmsg;
    if (usermsg && *usermsg)
        fprintf(stderr, "%s: ", usermsg);

    if ((errmsg = tostr(errnum)))
        fprintf(stderr, "%s\n", errmsg);
    else if (errnum > KERN_RETURN_MAX /* 100 */)
        fprintf(stderr, "Return value exceeds maximum allowable: %d\n", (int)errnum);
    else
        fprintf(stderr, "Illegal error code: %d\n", (int)errnum);
}

static const char *
tostr(kern_return_t errnum)
{
    switch (errnum) {
        case KERN_SUCCESS: /* 0 */
            return "All is well";

        case KERN_INVALID_ADDRESS: /* 1 */
            return "Specified address is not currently valid.";

        case KERN_PROTECTION_FAILURE: /* 2 */
            return "Specified memory is valid, but does not permit the required forms of access.";

        case KERN_NO_SPACE : /* 3 */
            return "The address range specified is already in use, or "
                   "no address range of the size specified could be "
                   "found";

        case KERN_INVALID_ARGUMENT : /* 4 */
            return "The function requested was not applicable to this "
                   "type of argument, or an argument is invalid";

        case KERN_FAILURE : /* 5 */
            return "The function could not be performed. A catch-all";

        case KERN_RESOURCE_SHORTAGE : /* 6 */
            return "A system resource could not be allocated to fulfill "
                   "this request. This failure may not be permanent";

        case KERN_NOT_RECEIVER : /* 7 */
            return "The task in question does not hold receive rights "
                   "for the port argument";

        case KERN_NO_ACCESS : /* 8 */
            return "Bogus access restriction";

        case KERN_MEMORY_FAILURE : /* 9 */
            return "During a page fault, the target address refers to a "
                   "memory object that has been destroyed. This "
                   "failure is permanent";

        case KERN_MEMORY_ERROR : /* 10 */
            return "During a page fault, the memory object indicated "
                   "that the data could not be returned. This failure "
                   "may be temporary; future attempts to access this "
                   "same data may succeed, as defined by the memory "
                   "object";

        case KERN_ALREADY_IN_SET : /* 11 */
            return "The receive right is already a member of the portset";

        case KERN_NOT_IN_SET : /* 12 */
            return "The receive right is not a member of a port set";

        case KERN_NAME_EXISTS : /* 13 */
            return "The name already denotes a right in the task";

        case KERN_ABORTED : /* 14 */
            return "The operation was aborted. Ipc code will "
                   "catch this and reflect it as a message error";

        case KERN_INVALID_NAME : /* 15 */
            return "The name doesn't denote a right in the task";

        case KERN_INVALID_TASK : /* 16 */
            return "Target task isn't an active task";

        case KERN_INVALID_RIGHT : /* 17 */
            return "The name denotes a right, but not an appropriate right";

        case KERN_INVALID_VALUE : /* 18 */
            return "A blatant range error";

        case KERN_UREFS_OVERFLOW : /* 19 */
            return "Operation would overflow limit on user-references";

        case KERN_INVALID_CAPABILITY : /* 20 */
            return "The supplied (port) capability is improper";

        case KERN_RIGHT_EXISTS : /* 21 */
            return "The task already has send or receive rights "
                   "for the port under another name";

        case KERN_INVALID_HOST : /* 22 */
            return "Target host isn't actually a host";

        case KERN_MEMORY_PRESENT : /* 23 */
            return "An attempt was made to supply \"precious\" data "
                   "for memory that is already present in a "
                   "memory object";

        case KERN_MEMORY_DATA_MOVED : /* 24 */
            return "A page was requested of a memory manager via "
                   "memory_object_data_request for an object using "
                   "a MEMORY_OBJECT_COPY_CALL strategy, with the "
                   "VM_PROT_WANTS_COPY flag being used to specify "
                   "that the page desired is for a copy of the "
                   "object, and the memory manager has detected "
                   "the page was pushed into a copy of the object "
                   "while the kernel was walking the shadow chain "
                   "from the copy to the object. This error code "
                   "is delivered via memory_object_data_error "
                   "and is handled by the kernel";
                    /* (it forces the kernel to restart the fault).
                     * It will not be seen by users
                     */

        case KERN_MEMORY_RESTART_COPY : /* 25 */
            return "A strategic copy was attempted of an object "
                   "upon which a quicker copy is now possible. "
                   "The caller should retry the copy using "
                   "vm_object_copy_quickly. This error code "
                   "is seen only by the kernel";

        case KERN_INVALID_PROCESSOR_SET : /* 26 */
            return "An argument applied to assert processor set privilege "
                   "was not a processor set control port";

        case KERN_POLICY_LIMIT : /* 27 */
            return "The specified scheduling attributes exceed the thread's "
                   "limits";

        case KERN_INVALID_POLICY : /* 28 */
            return "The specified scheduling policy is not currently "
                   "enabled for the processor set";

        case KERN_INVALID_OBJECT : /* 29 */
            return "The external memory manager failed to initialize the "
                   "memory object";

        case KERN_ALREADY_WAITING : /* 30 */
            return "A thread is attempting to wait for an event for which "
                   "there is already a waiting thread";

        case KERN_DEFAULT_SET : /* 31 */
            return "An attempt was made to destroy the default processor "
                   "set";

        case KERN_EXCEPTION_PROTECTED : /* 32 */
            return "An attempt was made to fetch an exception port that is "
                   "protected, or to abort a thread while processing a "
                   "protected exception";

        case KERN_INVALID_LEDGER : /* 33 */
            return "A ledger was required but not supplied";

        case KERN_INVALID_MEMORY_CONTROL : /* 34 */
            return "The port was not a memory cache control port";

        case KERN_INVALID_SECURITY : /* 35 */
            return "An argument supplied to assert security privilege "
                   "was not a host security port";

        case KERN_NOT_DEPRESSED : /* 36 */
            return "thread_depress_abort was called on a thread which "
                   "was not currently depressed";

        case KERN_TERMINATED : /* 37 */
            return "Object has been terminated and is no longer available";

        case KERN_LOCK_SET_DESTROYED : /* 38 */
            return "Lock set has been destroyed and is no longer available";

        case KERN_LOCK_UNSTABLE : /* 39 */
            return "The thread holding the lock terminated before releasing "
                   "the lock";

        case KERN_LOCK_OWNED : /* 40 */
            return "The lock is already owned by another thread";

        case KERN_LOCK_OWNED_SELF : /* 41 */
            return "The lock is already owned by the calling thread";

        case KERN_SEMAPHORE_DESTROYED : /* 42 */
            return "Semaphore has been destroyed and is no longer available";

        case KERN_RPC_SERVER_TERMINATED : /* 43 */
            return "Return from RPC indicating the target server was "
                   "terminated before it successfully replied";

        case KERN_RPC_TERMINATE_ORPHAN : /* 44 */
            return "Terminate an orphaned activation";

        case KERN_RPC_CONTINUE_ORPHAN : /* 45 */
            return "Allow an orphaned activation to continue executing";

        case KERN_NOT_SUPPORTED : /* 46 */
            return "Empty thread activation (No thread linked to it)";

        case KERN_NODE_DOWN : /* 47 */
            return "Remote node down or inaccessible";

        case KERN_NOT_WAITING : /* 48 */
            return "A signalled thread was not actually waiting";

        case KERN_OPERATION_TIMED_OUT : /* 49 */
            return "Some thread-oriented operation (semaphore_wait) timed out";

        case KERN_CODESIGN_ERROR : /* 50 */
            return "During a page fault, indicates that the page was rejected "
                   "as a result of a signature check";

        case KERN_POLICY_STATIC : /* 51 */
            return "The requested property cannot be changed at this time";

        case KERN_INSUFFICIENT_BUFFER_SIZE : /* 52 */
            return "The provided buffer is of insufficient size for the requested data";

        default:
            return NULL;
    }
}
