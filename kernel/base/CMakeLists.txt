configure_file(
        Config.hpp.in
        "${CMAKE_CURRENT_BINARY_DIR}/config/FunnyOS/Kernel/Config.hpp"
        @ONLY
)

add_library(FunnyOS_Kernel_Base STATIC
        src/GFX/ScreenManager.cpp
        src/MM/PhysicalMemoryManager.cpp
        src/MM/VirtualMemoryManager.cpp
        src/GDT.cpp
        src/KABI.cpp
        src/Kernel.cpp
        src/LogManager.cpp
        src/PlatformK64.cpp
)

target_include_directories(FunnyOS_Kernel_Base
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/headers/"
            "${CMAKE_CURRENT_BINARY_DIR}/config/"
)

target_link_libraries(FunnyOS_Kernel_Base
        PUBLIC
            FunnyOS_Stdlib_Base64_Kernel
            FunnyOS_Misc_Bootparams
            FunnyOS_Misc_Hardware64
            FunnyOS_Misc_MemoryAllocator64
            FunnyOS_Misc_TerminalManager64
)