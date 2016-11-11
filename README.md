Multi-platform C lib for peeking/poking~~/searching~~ memory

    #define CHARACTER_HEALTH 0xDEADBEEF
    vas_t *proc = vas_open(pid, 0);
    uint32_t health;
    vas_read(proc, CHARACTER_HEALTH, &health sizeof(uint32_t));
    printf("Character health is %" PRIu32 "\n", health);

Works on OS X, Windows and Linux. Requires [libpid]. Install with:

    cmake .
    make install

# License
libvas is released under the LGPL. It contains code from [Stijus' MemoryScanner], released under the MIT License.

[libpid]: https://github.com/a3f/libpid
[Stijus' MemoryScanner]: https://github.com/Stiju/MemoryScanner
