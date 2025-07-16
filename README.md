# myos

## Description
**myos** is a simple OS that runs on a Raspberry Pi 3B+ (3B should work too). I created it because I wanted to learn more about operating systems and low-level programming. I test it on my Raspberry Pi and also in QEMU.

## Status
- libc implementation is still a work in progress
  - Can run the Lua interpreter with very few issues
  - Vim only kind-of works
- For processes, moved memory mapping of heap and stack to be further apart
  - This prevents the stack from overflowing into the heap without causing a page fault
- Started implementing signals
- Filesystem is no longer RAM-only
  - A FAT32 filesystem can be mounted as read-only
  - Next step is to allow files to be synced to the disk
- Shell now uses GNU readline
  - This allows some autocompletion and command history
- Future ideas:
  - Add pthreads
  - Add dynamic linking
  - Make the shell a proper POSIX shell, i.e. an interpreter for the shell command language
