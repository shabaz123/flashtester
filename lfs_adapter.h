//
// Created by shabaz on 11/02/2023.
//

#ifndef FLASHTESTER_LFS_ADAPTER_H
#define FLASHTESTER_LFS_ADAPTER_H

#include "littlefs/lfs.h"


int block_device_read(const struct lfs_config * cfg, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);
int block_device_prog(const struct lfs_config * cfg, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);
int block_device_erase(const struct lfs_config * cfg, lfs_block_t block);
int block_device_sync(const struct lfs_config * cfg);


#endif //FLASHTESTER_LFS_ADAPTER_H
