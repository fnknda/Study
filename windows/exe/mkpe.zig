const std = @import("std");
const sout = std.io.getStdOut().writer();
const dosCode = @embedFile("dos_code");
const peCode = @embedFile("pe_code");

const pe = @import("pe.zig");

inline fn splitFromStruct(data: anytype) []const u8 {
    return @as([*]const u8, @ptrCast(&data))[0..@sizeOf(@TypeOf(data))];
}

pub fn main() !void {
    const dosCodeBase = @sizeOf(pe.DosHeader);

    const peHeaderBase = dosCodeBase + dosCode.len;
    const optionalHeaderBase = peHeaderBase + @sizeOf(pe.CoffHeader);
    const peCodeBase = optionalHeaderBase + @sizeOf(pe.OptionalHeaderPE32plus);

    const dosHeader = pe.DosHeader{
        .signature = std.mem.readInt(u16, "MZ", std.builtin.Endian.Little),
        .bytes_in_last_block = 0x90,
        .blocks_in_file = 1,
        .num_relocs = 0,
        .header_paragraphs = 4,
        .min_extra_paragraphs = 0,
        .max_extra_paragraphs = 0xFFFF,
        .ss = 0,
        .sp = 0xB8,
        .checksum = 0,
        .ip = 0,
        .cs = 0,
        .reloc_table_offset = 0x40,
        .overlay_number = 0,
        .overlay_info = [_]u8{0} ** 8,
        .oem_identifier = 0,
        .oem_info = 0,
        .reserved = [_]u8{0} ** 20,
        .pe_header_start = peHeaderBase,
    };

    const peHeader = pe.CoffHeader{
        .signature = std.mem.readInt(u32, "PE\x00\x00", std.builtin.Endian.Little),
        .machine = 0x8664,
        .section_nums = 0,
        .timedate_stamp = 0,
        .symbol_table_start = 0,
        .symbol_table_nums = 0,
        .optional_header_sz = @sizeOf(pe.OptionalHeaderPE32plus),
        .characteristics = 0x202,
    };

    const optionalHeader = pe.OptionalHeaderPE32plus{
        .magic = 0x10b,
        .linker_version_major = 1,
        .linker_version_minor = 0,
        .code_sz = dosCode.len,
        .init_data_sz = 0,
        .uninit_data_sz = 0,
        .entry = peCodeBase,
        .base_of_code = peCodeBase,
    };

    var file = try std.fs.cwd().createFile("dos.exe", .{ .mode = 0o755 });
    defer file.close();

    var writer = file.writer();

    _ = try writer.write(splitFromStruct(dosHeader));
    _ = try writer.write(dosCode);
    _ = try writer.write(splitFromStruct(peHeader));
    _ = try writer.write(splitFromStruct(optionalHeader));
    _ = try writer.write(peCode);
}
