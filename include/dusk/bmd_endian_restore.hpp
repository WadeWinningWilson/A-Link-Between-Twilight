#ifndef DUSK_BMD_ENDIAN_RESTORE_HPP
#define DUSK_BMD_ENDIAN_RESTORE_HPP

#include <cstddef>

namespace dusk::bmd_export {

// Reverse PC J3D loader in-place fixups on mpRawData so SuperBMD sees on-disk BE layout.
void restore_pc_j3d2_for_export(void* data, std::size_t size);

}  // namespace dusk::bmd_export

#endif  // DUSK_BMD_ENDIAN_RESTORE_HPP
