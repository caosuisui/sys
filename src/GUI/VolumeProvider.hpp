//
// Created by wyz on 2022/4/28.
//

#ifndef SYS_VOLUMEPROVIDER_HPP
#define SYS_VOLUMEPROVIDER_HPP
#include <vector>
#include <string>
#include "VoxelCompression/voxel_compress/VoxelCmpDS.h"
using namespace sv;
class VolumeProvider{
public:
    struct BlockIndex{
        int x,y,z;
    };

    VolumeProvider(const std::string& filename);

    bool readBlock(const BlockIndex& index,void* buf,size_t size);

    struct VolumeInfo{
        uint32_t block_length;
        uint32_t padding;
        uint32_t raw_x;
        uint32_t raw_y;
        uint32_t raw_z;
    };
    VolumeInfo getVolumeInfo();

    std::unique_ptr<Reader> reader;

};


#endif //SYS_VOLUMEPROVIDER_HPP
