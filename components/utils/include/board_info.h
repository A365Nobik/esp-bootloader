#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif

    const char *get_chip_name(void);
    uint32_t get_chip_freq_hz(void);
    uint32_t get_flash_size_bytes(void);
    size_t get_sram_size_bytes(void);
    size_t get_psram_size_bytes(void);

#ifdef __cplusplus
}
#endif
