const std = @import("std");

const print = std.debug.print;

const elf = std.elf;
const fmt = std.fmt;
const fs = std.fs;
const io = std.io;
const linux = std.os.linux;
const mem = std.mem;
const process = std.process;

const palloc = std.heap.page_allocator;

var buffered_sout = io.bufferedWriter(io.getStdOut().writer());
const sout = buffered_sout.writer();

const prot = struct {
    const PROT_READ: u32 = 0x1;
    const PROT_WRITE: u32 = 0x2;
    const PROT_EXEC: u32 = 0x4;
    const PROT_NONE: u32 = 0x0;
};

fn unmap(org_path: []u8) !void {
    sout.print("Unmapping\n", .{}) catch unreachable;

    var arena = std.heap.ArenaAllocator.init(palloc);
    defer arena.deinit();
    const alloc = arena.allocator();

    const maps = fs.openFileAbsolute("/proc/self/maps", .{}) catch unreachable;
    defer maps.close();
    var maps_reader = io.bufferedReader(maps.reader());
    const reader = maps_reader.reader();

    var line = std.ArrayList(u8).init(alloc);
    defer line.deinit();

    const writer = line.writer();
    while (reader.streamUntilDelimiter(writer, '\n', null)) {
        defer line.clearRetainingCapacity();

        var info = mem.splitSequence(u8, line.items, " ");
        var addrs = mem.splitSequence(u8, info.first(), "-");
        const start = fmt.parseInt(u64, addrs.first(), 16) catch unreachable;
        const end = fmt.parseInt(u64, addrs.next().?, 16) catch unreachable;

        var path = info.next().?;
        while (true) {
            if (info.peek() == null)
                break;

            path = info.next().?;
        }

        if (path.len > 0 and mem.containsAtLeast(u8, path, 1, org_path)) {
            sout.print("Unmapping {s} ({x}, {x})\n", .{ path, start, end - start }) catch unreachable;
            _ = linux.munmap(@ptrFromInt(start), end - start);
        }
    } else |err| switch (err) {
        error.EndOfStream => {},
        else => return err,
    }

    sout.print("Unmapped\n", .{}) catch unreachable;
}

fn load(args: [][]u8) !void {
    sout.print("Loading {s}\n", .{args[0]}) catch unreachable;

    const f = fs.cwd().openFile(args[0], .{}) catch unreachable;
    defer f.close();
    const reader = f.reader();

    const ehdr = reader.readStruct(elf.Elf64_Ehdr) catch unreachable;
    f.seekTo(ehdr.e_phoff) catch unreachable;

    for (0..ehdr.e_phnum) |_| {
        const phdr = reader.readStruct(elf.Elf64_Phdr) catch unreachable;
        if (phdr.p_type == elf.PT_LOAD and (phdr.p_flags & elf.PF_X) != 0) {
            sout.print("Loading offset {x} into address {x}\n", .{ phdr.p_offset, phdr.p_vaddr }) catch unreachable;

            const vaddr = phdr.p_vaddr & 0xfffffffffffff000;
            const memsz = phdr.p_memsz + phdr.p_vaddr - vaddr;
            const addr = linux.mmap( //FIXME: Erroring with EINVAL
                @ptrFromInt(vaddr),
                memsz,
                prot.PROT_READ | prot.PROT_EXEC,
                linux.MAP.PRIVATE,
                f.handle,
                @intCast(phdr.p_offset),
            );

            if (addr != phdr.p_vaddr) {
                sout.print("  Could not map to right address ({x})->({x})\n", .{ phdr.p_vaddr, addr }) catch unreachable;
            }
        }
    }

    sout.print("Loaded\n", .{}) catch unreachable;
}

fn set_env(args: [][]u8, env: [][]u8) !void {
    sout.print("Setting environment {s} {*}\n", .{ args[0], env }) catch unreachable;

    sout.print("Environment set\n", .{}) catch unreachable;
}

fn execute() !void {
    sout.print("Executing\n", .{}) catch unreachable;
}

extern fn syscall_exit(code: u32) noreturn;

comptime {
    asm (
        \\ .global syscall_exit;
        \\ .type syscall_exit, @function;
        \\ syscall_exit:
        \\ mov $60, %rax
        \\ syscall
    );
}

export fn ulexec(c_org_path: [*c]u8, c_args: [*:0][*c]u8, c_env: [*:0][*c]u8) noreturn {
    var arena = std.heap.ArenaAllocator.init(palloc);
    defer arena.deinit();
    const alloc = arena.allocator();

    const org_path: []u8 = alloc.dupe(u8, mem.span(c_org_path)) catch unreachable;

    var args_arr = std.ArrayList([]u8).init(alloc);
    defer args_arr.deinit();
    var args_n: u64 = 0;
    while (c_args[args_n] != 0) : (args_n += 1) {
        const value: []u8 = alloc.dupe(u8, mem.span(c_args[args_n])) catch unreachable;
        args_arr.append(value) catch unreachable;
    }
    const args: [][]u8 = args_arr.items;

    var env_arr = std.ArrayList([]u8).init(alloc);
    defer env_arr.deinit();
    var env_n: u64 = 0;
    while (c_env[env_n] != 0) : (env_n += 1) {
        const value: []u8 = alloc.dupe(u8, mem.span(c_env[env_n])) catch unreachable;
        env_arr.append(value) catch unreachable;
    }
    const env: [][]u8 = env_arr.items;

    unmap(org_path) catch unreachable;
    load(args) catch unreachable;
    set_env(args, env) catch unreachable;
    execute() catch unreachable;

    syscall_exit(0);
}
