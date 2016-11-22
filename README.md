Multi-platform C lib for peeking/poking~~/searching~~ memory

    #define CHARACTER_HEALTH 0xDEADBEEF
    vas_t *proc = vas_open(pid, 0);
    uint32_t health;
    vas_read(proc, CHARACTER_HEALTH, &health sizeof(uint32_t));
    printf("Character health is %" PRIu32 "\n", health);

## Install

<!--
If Perl is available, you can install the library and dependencies by running

    cpan Proc::Memory

-->

### Manual install

    git clone https://github.com/a3f/libvas.git && cd libvas
    mkdir build && cd build
    cmake ..
    make install

## Wrappers

See [Proc::Memory] for a Perl interface to the library.

## License

libvas is released under the LGPL. It contains code from [Stijus' MemoryScanner], released under the MIT License.

[Proc::Memory]: https://metacpan.org/pod/Proc::Memory
[libpid]: https://github.com/a3f/libpid
[Stijus' MemoryScanner]: https://github.com/Stiju/MemoryScanner
