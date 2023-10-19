# TMPOUT

https://tmpout.sh/

## Elf Mangling

A study to see how much can we distort an Elf and still be able to execute it.

The result is a small elf with one single `PT_LOAD` segment and no sections.

The byte codes for the executable is stored in an unused part of the header, and the program header is overlapping some bytes with the elf header to shrink it's size.

## Infection methods

### Basic `PT_NOTE` -> `PT_LOAD` on `e_entry`

Basic infection method transforming a `PT_NOTE` segment into a `PT_LOAD` to load our payload into memory.

Change `e_entry` to point to our payload, and add a `jmp` at the end of our payload to redirect to where `e_entry` pointed to.

### EPO with `.init_array`

Using a simple Entry Point Obfuscation, we can use the `.init_array` section instead of the `e_entry` to execute our code.

We hijack the first address of the section to put our payload there.

We still need to `PT_NOTE` -> `PT_LOAD` our payload into memory, and we still need to `jmp` to the old function.

This infection method does not work on PIE executables because the relocation overrides the `.init_array` entries at program start.

### EPO with `SHT_RELA` (works with PIE)

This is nice.

Instead of overriding the `.init_array` section, we override the `.rela` section so that, on program load, it changes the entries on `.init_array` to point to our payload.

Almost as simple as overriding `.init_array` directly, but with another layer of indirection and obfuscation on top. And it works on PIE executables as a bonus.
