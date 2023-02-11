//
// lfs_adapter.cpp
// Provides the configuration/inter-working needed to use the littlefs filesystem
// with the SPI Flash code SPIFBlockDevice
// rev 1.0 - shabaz 11/02/2023
//

#include "lfs_adapter.h"
#include "SPIFBlockDevice.h"

extern SPIFBlockDevice spif;

// configuration of the filesystem is provided by this struct
struct lfs_config filesys_config = {
        .context = NULL,
        // block device operations
        .read  = block_device_read,
        .prog  = block_device_prog,
        .erase = block_device_erase,
        .sync  = block_device_sync,

        // block device configuration
        .read_size = 1,
        .prog_size = 1,
        .block_size = 4096,
        .block_count = 128,
        .block_cycles = 500,
        .cache_size = 16,
        .lookahead_size = 16,
};

int block_device_read(const struct lfs_config * cfg, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) {
    printf("block_device_read: block=%d, off=%d, size=%d\n", block, off, size);
    spif.read(buffer, (cfg->block_size * block)+off, size);
    return LFS_ERR_OK;
}

int block_device_prog(const struct lfs_config * cfg, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) {
    printf("block_device_prog: block=%d, off=%d, size=%d\n", block, off, size);
    //spif.erase(cfg->block_size * block, spif.get_erase_size());
    spif.program(buffer, (cfg->block_size * block)+off, size);
    return LFS_ERR_OK;
}

int block_device_erase(const struct lfs_config * cfg, lfs_block_t block) {
    printf("block_device_erase: block=%d\n", block);
    spif.erase(cfg->block_size * block, spif.get_erase_size());
    return LFS_ERR_OK;
}

int block_device_sync(const struct lfs_config * cfg) {
    return LFS_ERR_OK;
}
