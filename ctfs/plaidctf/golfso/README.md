# PlaidCTF 2020 - golf.so

The challenge was to create a shared object in less than 1024 bytes that could run `/bin/sh` through the command `LD_PRELOAD=<file> /bin/true`.

To create a shared object is a little troublesome. You need a `PT_DYN` segment and at least two `Elf64_Dyn` entries: `DT_STRTAB` and `DT_SYMTAB`. They can just be set to zero though.

We then need to be able to execute the program. For that, we use the `DT_INIT` entry. It points to a code that will be executed as a "library initialization function" after being loaded into memory.
