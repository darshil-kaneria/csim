#include "cache.h"

namespace csim {
    Cache::Cache(uint32_t set_assoc, uint32_t block_size, uint32_t tag_size, uint32_t addr_len)
    : set_assoc_(set_assoc),
    block_size_(block_size), 
    tag_size_(tag_size),
    addr_len_(addr_len) {
        // Todo...
    }
}