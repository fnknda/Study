pub const DosHeader = extern struct {
    signature: u16,
    bytes_in_last_block: u16,
    blocks_in_file: u16,
    num_relocs: u16,
    header_paragraphs: u16,
    min_extra_paragraphs: u16,
    max_extra_paragraphs: u16,
    ss: u16,
    sp: u16,
    checksum: u16,
    ip: u16,
    cs: u16,
    reloc_table_offset: u16,
    overlay_number: u16,
    overlay_info: [8]u8,
    oem_identifier: u16,
    oem_info: u16,
    reserved: [20]u8,
    pe_header_start: u32,
};

pub const Reloc = extern struct {
    offset: u16,
    segment: u16,
};

pub const CoffHeader = extern struct {
    signature: u32,
    machine: u16,
    section_nums: u16,
    timedate_stamp: u32,
    symbol_table_start: u32,
    symbol_table_nums: u32,
    optional_header_sz: u16,
    characteristics: u16,
};

pub const OptionalHeaderPE32 = extern struct {
    magic: u16,
    linker_version_major: u8,
    linker_version_minor: u8,
    code_sz: u32,
    init_data_sz: u32,
    uninit_data_sz: u32,
    entry: u32,
    base_of_code: u32,
    base_of_data: u32,
};

pub const OptionalHeaderPE32plus = extern struct {
    magic: u16,
    linker_version_major: u8,
    linker_version_minor: u8,
    code_sz: u32,
    init_data_sz: u32,
    uninit_data_sz: u32,
    entry: u32,
    base_of_code: u32,
};
