Multi-platform C lib for peeking/poking/handling virtual memory

    #define CHARACTER_HEALTH 0xDEADBEEF
    vas_t *proc = vas_open(pid, 0);
    uint32_t health;
    vas_read(proc, CHARACTER_HEALTH, &health sizeof(uint32_t));
    printf("Character health is %" PRIu32 "\n", health);

## Features

- Multi-Platform: Runs on Windows, Linux, BSD, MacOS, GNU Hurd and a couple other systems
- Read/Write/Remap other processes' memory
- Allocate those nifty virtual-memory-mirrored ring buffers
- Duplicate memory with Copy-on-Write semantics
- Portably map files

## Install

If Perl is available, you can run:

    cpan Alien::libvas
    # TODO: change to hardlink
    cp -R $(perl -MAlien::libvas -e 'print Alien::libvas->dist_dir,"/share"') /usr/local/

If not consider installing it, as it not only fetches CMake if unavailable and uses it to build the library but also runs the much more extensive perl-based test suite. See [Alien::libvas] for details.

Afterwards you can use `pkg-config --libs libvas` and `pkg-config --cflags libvas` in your build script.

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
[Alien::libvas]: https://github.com/athreef/Alien::libvas
[Stijus' MemoryScanner]: https://github.com/Stiju/MemoryScanner
