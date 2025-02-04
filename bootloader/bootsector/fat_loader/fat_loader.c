#include <FunnyOS/QuickFat/QuickFat.h>

#include <stdnoreturn.h>
#include <config.h>

/**
 * Size of a sector in bytes.
 */
#define SECTOR_SIZE 0x200

/**
 * Name of the file to boot
 */
#define FILE_NAME "/boot/bootload32"

#define VIDEO_MEMORY ((uint8_t(*))0xB8000)
#define SCREEN_WIDTH 80

// Boot parameters
extern uint8_t g_boot_partition;

// Read buffer
uint8_t g_read_buffer[SECTOR_SIZE];

/**
 * Loads one sector from the boot drive starting from logical sector address [lba].
 * Result in stored in [read_buffer]
 *
 * @return 0 if success, a non-zero error code if fail
 */
__attribute__((cdecl)) extern int fl_load_from_disk(uint32_t lb);

/**
 * Jumps back to real mode, loads boot parameters and jumps to address at [bootloader_location]
 */
noreturn extern void fl_jump_to_bootloader(void* bootloader_location);

/**
 * Hangs the CPU indefinitely.
 */
noreturn extern void fl_hang(void);

void* memcpy(void* dest, const void* src, size_t count) {
    for (size_t i = 0; i < count; i++) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    if (dest < src) {
        for (size_t i = 0; i < count; i++) {
            ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
        }
    } else if (dest > src) {
        for (size_t i = count; i > 0; i--) {
            ((uint8_t*)dest)[i - 1] = ((uint8_t*)src)[i - 1];
        }
    }

    return dest;
}

void* memset(void* dest, int value, size_t count) {
    for (size_t i = 0; i < count; i++) {
        ((uint8_t*)dest)[i] = value;
    }

    return dest;
}

extern void fl_print(const char* str) {
    static unsigned int c_cursor_base = 0;

    for (; *str; str++) {
        switch (*str) {
            case '\n':
                c_cursor_base += SCREEN_WIDTH;
                break;
            case '\r':
                c_cursor_base -= c_cursor_base % SCREEN_WIDTH;
                break;
            case 0:
                return;
            default:
                *(VIDEO_MEMORY + c_cursor_base * 2 + 0) = *str;
                *(VIDEO_MEMORY + c_cursor_base * 2 + 1) = 0x0F;
                c_cursor_base++;
                break;
        }
    }
}

/**
 * Prints an integer onto the screen.
 */
void fl_print_int(unsigned int number) {
    const int count = sizeof(int) * 2;
    char buffer[count + 1];

    for (int i = 0; i < count; i++) {
        const unsigned char part = (char)(number >> ((count - i - 1) * 4)) & 0x0F;
        buffer[i] = "0123456789ABCDEF"[part];
    }
    buffer[count] = 0;

    fl_print(buffer);
}

/**
 * Prints an error code and hangs the CPU indefinitely.
 */
#define CHECK_ERROR(code, error) \
    if ((error_code & (code)) == (code)) fl_print((error))

noreturn void fl_error(int error_code) {
    CHECK_ERROR(QUICKFAT_ERROR_FAILED_LOAD_MBR, " ** Failed to load the MBR into memory");
    CHECK_ERROR(QUICKFAT_ERROR_FAILED_LOAD_MBR_SIGNATURE_MISMATCH, " ** The MBR signature is invalid");
    CHECK_ERROR(QUICKFAT_ERROR_FAILED_LOAD_BPB, " ** Failed to load the BPB into memory");
    CHECK_ERROR(QUICKFAT_ERROR_FAILED_LOAD_BPB_SIGNATURE_MISMATCH, " ** The BPB signature is invalid");
    CHECK_ERROR(QUICKFAT_ERROR_FAILED_TO_LOAD_CLUSTER_FROM_MAP, " ** Failed to load cluster info from FAT");
    CHECK_ERROR(QUICKFAT_ERROR_FAILED_TO_LOAD_FAT_CLUSTER, " ** Failed to load cluster from disk");
    CHECK_ERROR(QUICKFAT_ERROR_FAILED_TO_FIND_FILE, " ** " FILE_NAME " not found.");

    fl_print("\r\n ** Loading error: 0x");
    fl_print_int(error_code);
    fl_print("\r\n");
    fl_hang();
}
#undef CHECK_ERROR

/**
 * Wrapper for fl_load_from_disk for QuickFat
 */
extern int fl_load_from_disk_wrapper(void* unused, uint32_t lba, uint32_t count, uint8_t* out) {
    (void)unused;

    int error;

    for (uint32_t i = 0; i < count; i++) {
        if ((error = fl_load_from_disk(lba + i)) != 0) {
            return error;
        }

        memcpy(out + (i * SECTOR_SIZE), g_read_buffer, SECTOR_SIZE);
    }

    return 0;
}

/**
 * Main function
 */
noreturn void fat_loader(void) {
    fl_print("FunnyOS FAT32 Loader\r\n");

    // Init context
    fl_print(" * Initializing the QuickFat library\r\n");
    QuickFat_Context context;
    QuickFat_initialization_data init;
    init.partition_entry = g_boot_partition;
    init.read_function = fl_load_from_disk_wrapper;

    int error;

    if ((error = quickfat_init_context(&context, &init)) != 0) {
        fl_error(error);
    }

    // Open file
    fl_print(" * Searching for " FILE_NAME "\r\n");

    QuickFat_File file;
    if ((error = quickfat_open_file(&context, &file, FILE_NAME)) != 0) {
        fl_error(error);
    }

    // Read file
    fl_print(" * File found. Loading... \n\r");
    if ((error = quickfat_read_file(&context, &file, (void*)F_BOOTLOADER_MEMORY_LOCATION)) != 0) {
        fl_error(error);
    }

    // Verify
    fl_print(" * File loaded. Verifying... \n\r");
    uint32_t* magic_location = (uint32_t*)(((uint8_t*)F_BOOTLOADER_MEMORY_LOCATION) + file.size - 4);

    // Die, horribly
    if (*magic_location != F_BOOTLOADER_MAGIC) {
        fl_print(" ** Verification failed. \n\r");
        fl_error(1);
    }

    fl_jump_to_bootloader((void*)F_BOOTLOADER_MEMORY_LOCATION);
}