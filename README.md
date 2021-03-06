Cross-Platform C library for peeking/poking/handling virtual memory

    #define CHARACTER_HEALTH_ADDR 0xDEADBEEF /* In foreign address space */
    unsigned health;
    vas_t *vas = vas_open(process_id, 0);
    vas_read(vas, CHARACTER_HEALTH_ADDR, &health, sizeof (unsigned));
    printf("Character health is %u\n", health);

[![Build Status](https://travis-ci.org/a3f/libvas.svg?branch=master)](https://travis-ci.org/a3f/libvas)
[![Build status](https://ci.appveyor.com/api/projects/status/q68mvjmksaide04c/branch/master?svg=true)](https://ci.appveyor.com/project/a3f/libvas/branch/master)
[![CPAN testers](https://img.shields.io/badge/CPAN%20Testers-Alien::libvas-orange.svg)](http://www.cpantesters.org/distro/A/Alien-libvas.html)
[![Perl Wrapper](https://img.shields.io/badge/Perl%20Wrapper-Proc%3A%3AMemory-blue.svg)](https://metacpan.org/pod/Proc::Memory)


## Features

- Cross-Platform: Runs on Windows, Linux, BSD, MacOS, SunOS and a probably a couple more systems
- Read/Write/Remap other processes' memory
- Allocate those nifty virtual-memory-mirrored ring buffers
- Duplicate memory with Copy-on-Write semantics
- Portably map files

## Install

Binary releases for Windows, macOS and Linux are available at the [Github releases page](https://github.com/a3f/libvas/releases).

Otherwise, with Perl (and enough disk space for package dependencies) available, you can run:

    cpan Alien::libvas
    ln $(perl -MAlien::libvas -e 'print Alien::libvas->dist_dir')/share/pkgconfig/libvas.pc /usr/local/share/pkgconfig/

This fetches CMake if unavailable, uses it to build the library, installs the Perl wrapper and runs both the C and Perl test suite. See [Alien::libvas] for details.

Afterwards you can use `pkg-config --libs libvas` and `pkg-config --cflags libvas` in your build scripts.

### Manual install

    git clone https://github.com/a3f/libvas.git && cd libvas
    mkdir build && cd build
    cmake ..
    make
    ctest
    make install

Some manual tests/examples are available in `test/manual/`. Applicable tests in `test/` are run automatically on `ctest`.

## Platforms

Multiple backends are available, each corresponding to a directory in the source hierarchy:

    • win32      - Windows API's {Read,Write}ProcessMemory
    • mach       - Mach Virtual Memory API (vm_copy) - macOS and possibly GNU Hurd
    • process_vm - process_vm_{readv, writev} on Linux 3.2+
    • procfs     - /proc/$pid/mem on Linux and some BSDs, /proc/$pid/as on SunOS
    • ptrace     - ptrace(2), available on many Unices
    • memcpy     - Trivial implementation that doesn't supports foreign address spaces

The appropriate backend is selected by CMake at configuration time. You can override the selection by specifying e.g. `cmake -DBACKEND=ptrace`.

libvas has no external non-OS dependencies and is written in C++-compatible C89 and should compilable by any standard C or C++ compiler.

## TODO

- [X] Add support for `ptrace(PT_IO, ...)` on BSD
- [ ] CheatEngine's dbk32.sys driver as another Win32 Backend
- [ ] Change page access permissions
- [X] Better error reporting
- [ ] Report errors on Windows
- [ ] Make global errors thread-local
- [ ] run `onchild_fork` test on macOS without super user privileges
- [ ] Make Backends selectable at run time
- [ ] Partition backends: ringbuffer, CoW, memory peek/poke?
- [ ] Trapping access violations
- [ ] Guard page allocation?
- [ ] Wrappers for scripting languages besides Perl
- [ ] Provide all configurations in the release package: universal libraries on macOS, Both 32 bit and 64 bit on Linux and *.lib as well as *.a on Windows

## License

libvas is released under the LGPL. It contains code from [Stijus' MemoryScanner], released under the MIT License.

[Proc::Memory]: https://metacpan.org/pod/Proc::Memory
[libpid]: https://github.com/a3f/libpid
[Alien::libvas]: https://github.com/athreef/Alien-libvas
[Stijus' MemoryScanner]: https://github.com/Stiju/MemoryScanner
