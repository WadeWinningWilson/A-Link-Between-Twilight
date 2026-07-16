#include "dusk/bmd_endian_restore.hpp"

#if TARGET_PC

#include "dusk/endian.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace dusk::bmd_export {
namespace {

constexpr u32 kGxVaNull = 0xFF;

struct VtxFmt {
    u32 attr;
    u32 cnt;
    u32 typ;
    u8 frac;
};

static u32 read_be32(const u8* p) {
    return (u32(p[0]) << 24) | (u32(p[1]) << 16) | (u32(p[2]) << 8) | u32(p[3]);
}

static u32 read_le32(const u8* p) {
    return u32(p[0]) | (u32(p[1]) << 8) | (u32(p[2]) << 16) | (u32(p[3]) << 24);
}

static void write_be32(u8* p, u32 v) {
    p[0] = u8(v >> 24);
    p[1] = u8(v >> 16);
    p[2] = u8(v >> 8);
    p[3] = u8(v);
}

static u16 read_be16(const u8* p) {
    return u16((u16(p[0]) << 8) | u16(p[1]));
}

static void write_be16(u8* p, u16 v) {
    p[0] = u8(v >> 8);
    p[1] = u8(v);
}

static void host_u32_to_be(u8* p) {
    write_be32(p, read_le32(p));
}

static void host_u16_to_be(u8* p) {
    write_be16(p, u16((u16(p[0]) | (u16(p[1]) << 8))));
}

static bool is_superbmd_attr(u32 attr) {
    return attr >= 9 && attr <= 20;
}

static u32 comp_stride(const char* name, u32 typ) {
    if (name[0] == 'c') {
        return (typ == 0 || typ == 5) ? 2 : 1;
    }
    if (typ == 4) {
        return 4;
    }
    if (typ == 2 || typ == 3) {
        return 2;
    }
    return 1;
}

static u32 attr_id_for_name(const char* name) {
    if (std::strcmp(name, "pos") == 0) {
        return 9;
    }
    if (std::strcmp(name, "nrm") == 0) {
        return 10;
    }
    if (std::strcmp(name, "nbt") == 0) {
        return 25;
    }
    if (std::strcmp(name, "c0") == 0) {
        return 11;
    }
    if (std::strcmp(name, "c1") == 0) {
        return 12;
    }
    if (name[0] == 't' && name[1] >= '0' && name[1] <= '7' && name[2] == '\0') {
        return 13 + u32(name[1] - '0');
    }
    return 0;
}

struct AttrPtr {
    const char* name;
    u32 rel;
};

static constexpr AttrPtr kAttrOrder[] = {
    {"pos", 0x0C}, {"nrm", 0x10}, {"nbt", 0x14}, {"c0", 0x18}, {"c1", 0x1C},
    {"t0", 0x20},  {"t1", 0x24}, {"t2", 0x28}, {"t3", 0x2C}, {"t4", 0x30},
    {"t5", 0x34},  {"t6", 0x38}, {"t7", 0x3C},
};

static void unfix_array(u8* buf, u32 start, u32 end, u32 stride) {
    if (start == 0 || end <= start) {
        return;
    }
    if (stride == 1) {
        return;
    }
    if (stride == 2) {
        for (u32 pos = start; pos + 2 <= end; pos += 2) {
            host_u16_to_be(buf + pos);
        }
        return;
    }
    if (stride == 3) {
        for (u32 pos = start; pos + 3 <= end; pos += 3) {
            u8 b0 = buf[pos];
            u8 b1 = buf[pos + 1];
            u8 b2 = buf[pos + 2];
            buf[pos] = b2;
            buf[pos + 1] = b1;
            buf[pos + 2] = b0;
        }
        return;
    }
    for (u32 pos = start; pos + 4 <= end; pos += 4) {
        host_u32_to_be(buf + pos);
    }
}

static std::vector<VtxFmt> rebuild_vtx_attr_fmt_list(u8* buf, u32 vtx_off, u32 list_off) {
    std::vector<VtxFmt> parsed;
    u32 pos = list_off;
    while (pos + 16 <= vtx_off + read_be32(buf + vtx_off + 4)) {
        const u32 array_type = read_le32(buf + pos);
        if (array_type == 0 || array_type == kGxVaNull) {
            break;
        }
        if (!is_superbmd_attr(array_type)) {
            break;
        }
        parsed.push_back({array_type, read_le32(buf + pos + 4), read_le32(buf + pos + 8), buf[pos + 12]});
        pos += 16;
    }

    u32 min_ptr = 0;
    bool have_ptr = false;
    for (const AttrPtr& ap : kAttrOrder) {
        const u32 rel = read_be32(buf + vtx_off + ap.rel);
        if (rel == 0) {
            continue;
        }
        if (!have_ptr || rel < min_ptr) {
            min_ptr = rel;
            have_ptr = true;
        }
    }

    u32 table_bytes = have_ptr ? (min_ptr - (list_off - vtx_off)) : 0x40;
    if (table_bytes == 0) {
        table_bytes = 0x40;
    }

    std::vector<u8> out(table_bytes, 0);
    u32 write_pos = 0;
    for (const VtxFmt& fmt : parsed) {
        write_be32(out.data() + write_pos, fmt.attr);
        write_be32(out.data() + write_pos + 4, fmt.cnt);
        write_be32(out.data() + write_pos + 8, fmt.typ);
        out[write_pos + 12] = fmt.frac;
        out[write_pos + 13] = 0xFF;
        out[write_pos + 14] = 0xFF;
        out[write_pos + 15] = 0xFF;
        write_pos += 16;
    }
    if (write_pos + 4 <= out.size()) {
        write_be32(out.data() + write_pos, kGxVaNull);
    }
    std::memcpy(buf + list_off, out.data(), out.size());
    return parsed;
}

static void unfix_vtx1(u8* buf, u32 off) {
    const u32 fmt_rel = read_be32(buf + off + 8);
    const u32 list_off = off + fmt_rel;
    const std::vector<VtxFmt> formats = rebuild_vtx_attr_fmt_list(buf, off, list_off);

    struct NamedPtr {
        const char* name;
        u32 abs;
    };
    std::vector<NamedPtr> ordered;
    for (const AttrPtr& ap : kAttrOrder) {
        const u32 rel = read_be32(buf + off + ap.rel);
        if (rel != 0) {
            ordered.push_back({ap.name, off + rel});
        }
    }

    for (std::size_t i = 0; i < ordered.size(); i++) {
        const u32 aid = attr_id_for_name(ordered[i].name);
        const VtxFmt* fmt = nullptr;
        for (const VtxFmt& f : formats) {
            if (f.attr == aid) {
                fmt = &f;
                break;
            }
        }
        if (fmt == nullptr) {
            continue;
        }
        const u32 start = ordered[i].abs;
        const u32 end = (i + 1 < ordered.size()) ? ordered[i + 1].abs : off + read_be32(buf + off + 4);
        unfix_array(buf, start, end, comp_stride(ordered[i].name, fmt->typ));
    }
}

static void unfix_jnt1(u8* buf, u32 off) {
    const u16 joint_num = read_be16(buf + off + 8);
    const u32 init_base = off + read_be32(buf + off + 0x0C);
    const u32 idx_base = off + read_be32(buf + off + 0x10);
    for (u16 i = 0; i < joint_num; i++) {
        const u32 idx_off = idx_base + u32(i) * 2;
        host_u16_to_be(buf + idx_off);
        const u16 idx = read_be16(buf + idx_off);
        const u32 joff = init_base + u32(idx) * 0x30;
        host_u16_to_be(buf + joff);
        for (int f = 0; f < 3; f++) {
            host_u32_to_be(buf + joff + 0x04 + u32(f) * 4);
        }
        for (int f = 0; f < 3; f++) {
            host_u16_to_be(buf + joff + 0x10 + u32(f) * 2);
        }
        for (int f = 0; f < 3; f++) {
            host_u32_to_be(buf + joff + 0x18 + u32(f) * 4);
        }
        host_u32_to_be(buf + joff + 0x24);
        for (int f = 0; f < 3; f++) {
            host_u32_to_be(buf + joff + 0x28 + u32(f) * 4);
        }
    }
}

static void unfix_vtx_desc_list(u8* buf, u32 list_off, u32 region_end) {
    u32 pos = list_off;
    while (pos + 8 <= region_end) {
        host_u32_to_be(buf + pos);
        host_u32_to_be(buf + pos + 4);
        if (read_be32(buf + pos) == kGxVaNull) {
            break;
        }
        pos += 8;
    }
}

static void unfix_shp1(u8* buf, u32 off) {
    const u16 shape_num = read_be16(buf + off + 8);
    const u32 init_rel = read_be32(buf + off + 0x0C);
    const u32 idx_rel = read_be32(buf + off + 0x10);
    const u32 vtx_rel = read_be32(buf + off + 0x18);
    const u32 mtx_rel = read_be32(buf + off + 0x1C);
    const u32 vtx_base = off + vtx_rel;
    const u32 vtx_region_end = (mtx_rel > vtx_rel) ? off + mtx_rel : vtx_base + 0x80;

    std::vector<u32> list_starts;
    if (init_rel && idx_rel && shape_num) {
        const u32 init_base = off + init_rel;
        const u32 idx_base = off + idx_rel;
        for (u16 i = 0; i < shape_num; i++) {
            const u16 shape_idx = read_be16(buf + idx_base + u32(i) * 2);
            const u32 shape_off = init_base + u32(shape_idx) * 0x28;
            if (shape_off + 6 <= off + read_be32(buf + off + 4)) {
                const u16 desc_idx = read_be16(buf + shape_off + 4);
                list_starts.push_back(vtx_base + desc_idx);
            }
        }
    }
    if (list_starts.empty()) {
        list_starts.push_back(vtx_base);
    }
    std::sort(list_starts.begin(), list_starts.end());
    list_starts.erase(std::unique(list_starts.begin(), list_starts.end()), list_starts.end());
    for (u32 list_off : list_starts) {
        if (list_off < vtx_base || list_off >= vtx_region_end) {
            continue;
        }
        unfix_vtx_desc_list(buf, list_off, vtx_region_end);
    }
}

static void restore_blocks(u8* buf, std::size_t size) {
    if (size < 0x28 || std::memcmp(buf, "J3D2", 4) != 0) {
        return;
    }
    u32 off = 0x20;
    while (off + 8 <= size) {
        const u32 bsize = read_be32(buf + off + 4);
        if (bsize < 8 || off + bsize > size) {
            break;
        }
        if (std::memcmp(buf + off, "VTX1", 4) == 0) {
            unfix_vtx1(buf, off);
        } else if (std::memcmp(buf + off, "JNT1", 4) == 0) {
            unfix_jnt1(buf, off);
        } else if (std::memcmp(buf + off, "SHP1", 4) == 0) {
            unfix_shp1(buf, off);
        }
        off += bsize;
    }
}

}  // namespace

void restore_pc_j3d2_for_export(void* data, std::size_t size) {
    if (data == nullptr || size == 0) {
        return;
    }
    restore_blocks(static_cast<u8*>(data), size);
}

}  // namespace dusk::bmd_export

#else  // !TARGET_PC

namespace dusk::bmd_export {

void restore_pc_j3d2_for_export(void*, std::size_t) {}

}  // namespace dusk::bmd_export

#endif  // TARGET_PC
