# CMake build for Turbo Storage, the PS Vita storage plugin (firmware 3.60-3.74,
# taiHEN/HENkaku).
#
# Four artifacts:
#   storageturbo_kernel.skprx - kernel module: the faster storage path (staging),
#                               settings, the HUD, the boot guard, and the
#                               user-callable SceStorageTurbo syscall library.
#   storageturbo.suprx        - user module under *main (SceShell): boot guard,
#                               HUD registration, developer command server.
#   storageturbo_settings.suprx - the Turbo Storage page, under *NPXS10015 (the
#                               Settings app).
#   storageturbo_qm.suprx     - optional quick-menu row (a QuickMenuReborn
#                               client), dropped into ur0:QuickMenuReborn/.
#
# BUILD REQUIREMENTS: the VitaSDK toolchain (https://vitasdk.org) and taiHEN's
# headers/stubs (taihen):
#
#   export VITASDK=/usr/local/vitasdk
#   cmake -S . -B build \
#         -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
#   cmake --build build
#
# Order matters: the kernel module's stub library (SceStorageTurbo_stub) is
# generated from storageturbo_kernel.yml and must exist before the user module
# links.

cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
        CACHE PATH "VitaSDK toolchain")
  else()
    message(FATAL_ERROR "Set VITASDK or pass -DCMAKE_TOOLCHAIN_FILE")
  endif()
endif()

project(storageturbo C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra -Wno-unused-parameter -O2)

# Build id, logged next to the version so a user's log pins the exact build:
# the git commit (-dirty with uncommitted changes). The Makefile passes it in;
# otherwise ask git here, or say "unknown".
if(NOT TS_BUILD_ID)
  execute_process(COMMAND git describe --always --dirty --abbrev=7 --exclude "*"
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE TS_BUILD_ID OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_QUIET)
endif()
if(NOT TS_BUILD_ID)
  set(TS_BUILD_ID unknown)
endif()
add_compile_definitions(TS_BUILD_ID="${TS_BUILD_ID}")

# --------------------------------------------------------------------------
# Kernel module: storageturbo_kernel.skprx
# --------------------------------------------------------------------------
add_executable(storageturbo_kernel
  kernel/main.c
  kernel/klog.c
  kernel/settings.c
  kernel/stage.c
  kernel/hud.c
  kernel/hud_i18n.c
  kernel/diag.c
  kernel/fg.c
)

target_link_libraries(storageturbo_kernel
  taihenForKernel_stub
  taihenModuleUtils_stub        # module_get_export_func (registry: HUD language)
  SceSysmemForDriver_stub       # memblocks, kernel->user copies
  SceSysclibForDriver_stub      # memcpy/memset/memcmp/snprintf for kernel
  SceThreadmgrForDriver_stub
  SceDebugForDriver_stub        # ksceDebugPrintf (kernel logger)
  SceIofilemgrForDriver_stub    # ksceIo* (log, settings, boot guard)
  SceSdifForDriver_stub         # ksceSdifGetSdContextGlobal (which port a call is for)
)
# taiHEN modules enter via module_start, not crt0's main - drop startup files.
target_link_options(storageturbo_kernel PRIVATE -nostartfiles)

vita_create_self(storageturbo_kernel.skprx storageturbo_kernel
  CONFIG ${CMAKE_SOURCE_DIR}/storageturbo_kernel.yml
  UNSAFE)
vita_create_stubs(storageturbo-stubs storageturbo_kernel
  ${CMAKE_SOURCE_DIR}/storageturbo_kernel.yml KERNEL)

# --------------------------------------------------------------------------
# User module: storageturbo.suprx (SceShell, *main)
# --------------------------------------------------------------------------
#
# LINK RECIPE (why weak stubs, as in Wi-Fi Turbo): a STRONG import of a user
# library that is not loaded in the host process makes taiHEN fail the whole
# module load SILENTLY - module_start never runs. Every standard system library
# is therefore imported as its *_stub_weak variant: a weak import that cannot
# be resolved is skipped, and when the library IS present it resolves normally.
# Only taihen and our own kernel syscall library stay STRONG: without them the
# module must not load at all.
#
# Kept small: SceShell memory is shared by the Home screen, LiveArea, the
# downloader and PSN sign-in. The Settings page is its own module (below).
add_executable(storageturbo
  user/main.c
  user/dispatch.c                       # developer command server
  settings_app/tslog.c                  # the shared user.log
)

add_dependencies(storageturbo storageturbo-stubs)

target_link_libraries(storageturbo
  # STRONG - must be present, fail loudly if not:
  taihen_stub
  ${CMAKE_BINARY_DIR}/libstorageturbo_kernel_stub.a   # SceStorageTurbo syscalls
  # WEAK - host-resident libs:
  SceNet_stub_weak              # sceNet* sockets (dispatch.c)
  SceLibKernel_stub_weak        # sceKernel* / sceIo* helpers, module loading
  SceKernelThreadMgr_stub_weak  # sceKernelDelayThread, threads
  SceIofilemgr_stub_weak        # sceIoOpen/Write/Close (the log file)
  SceProcessmgr_stub_weak       # sceKernelGetProcessTimeWide (log stamps)
)

# -Tdata: vita-elf-create appends ~1.5 KB of module info to the end of the text
# segment; a fixed data address leaves room regardless (see Wi-Fi Turbo).
target_link_options(storageturbo PRIVATE -nostartfiles -Wl,-Tdata=0x81100000)
target_compile_options(storageturbo PRIVATE -Os)

vita_create_self(storageturbo.suprx storageturbo
  CONFIG ${CMAKE_SOURCE_DIR}/storageturbo.yml
  UNSAFE)

# --------------------------------------------------------------------------
# Settings page: storageturbo_settings.suprx, loaded into the Settings app only
# --------------------------------------------------------------------------
#
# Listed under *NPXS10015 in config.txt. Kept out of storageturbo.suprx so its
# code, translations and page buffers cost SceShell nothing. A config.txt from
# before v0.4.0 lists storageturbo.suprx under *NPXS10015 instead; that module
# then loads this one (user/main.c). Same import rules as above.
add_executable(storageturbo_settings
  settings_app/settings_main.c
  settings_app/storageturbo_settings.c  # the page: Settings-app hooks
  settings_app/storageturbo_i18n.c      # its text in 20 languages
  settings_app/tslog.c                  # the shared user.log
)
add_dependencies(storageturbo_settings storageturbo-stubs)
target_link_libraries(storageturbo_settings
  taihen_stub
  ${CMAKE_BINARY_DIR}/libstorageturbo_kernel_stub.a
  SceLibKernel_stub_weak        # sceIo* helpers
  SceKernelModulemgr_stub_weak  # sceKernelGetModuleIdByAddr (second-copy check)
  SceKernelThreadMgr_stub_weak
  SceIofilemgr_stub_weak        # the log file
  SceProcessmgr_stub_weak       # log stamps
  SceRegistryMgr_stub_weak      # sceRegMgrGetKeyInt (system language)
  SceAppUtil_stub_weak          # sceAppUtilSystemParamGetInt (language fallback)
)
# Size-optimised: page UI, not timing-sensitive.
target_compile_options(storageturbo_settings PRIVATE -Os)
target_link_options(storageturbo_settings PRIVATE -nostartfiles -Wl,-Tdata=0x81100000)

vita_create_self(storageturbo_settings.suprx storageturbo_settings
  CONFIG ${CMAKE_SOURCE_DIR}/storageturbo_settings.yml
  UNSAFE)

# --------------------------------------------------------------------------
# Optional quick-menu plugin: storageturbo_qm.suprx (a QuickMenuReborn client)
# --------------------------------------------------------------------------
#
# Dropped into ur0:QuickMenuReborn/ (not config.txt): QuickMenuReborn loads it
# into SceShell and builds its row each time the quick menu opens. The
# QuickMenuReborn calls it makes are declared in quickmenu/qmr.h, and their NIDs
# listed in quickmenu/qmr_imports.yml, from which the link stub is generated.

set(QMR_STUB_DIR ${CMAKE_BINARY_DIR}/qmr_stubs)
add_custom_command(
  OUTPUT ${QMR_STUB_DIR}/libQuickMenuReborn_stub.a
  COMMAND ${CMAKE_COMMAND} -E make_directory ${QMR_STUB_DIR}
  COMMAND vita-libs-gen-2 -yml=${CMAKE_SOURCE_DIR}/quickmenu/qmr_imports.yml -output=${QMR_STUB_DIR}
  COMMAND make -s -C ${QMR_STUB_DIR}
  DEPENDS ${CMAKE_SOURCE_DIR}/quickmenu/qmr_imports.yml
  COMMENT "Generating the QuickMenuReborn link stub")
add_custom_target(qmr-stubs DEPENDS ${QMR_STUB_DIR}/libQuickMenuReborn_stub.a)

add_executable(storageturbo_qm
  quickmenu/storageturbo_qm.c          # its one label needs no translations
)
add_dependencies(storageturbo_qm storageturbo-stubs qmr-stubs)
target_include_directories(storageturbo_qm PRIVATE ${CMAKE_SOURCE_DIR}/quickmenu)
target_link_libraries(storageturbo_qm
  # STRONG: no kernel module or no QuickMenuReborn -> the plugin does not load.
  ${CMAKE_BINARY_DIR}/libstorageturbo_kernel_stub.a
  ${QMR_STUB_DIR}/libQuickMenuReborn_stub.a
  SceIofilemgr_stub_weak        # writes the row icon to ur0:data/turbostorage/
  SceLibKernel_stub_weak
)
target_link_options(storageturbo_qm PRIVATE -nostartfiles)
target_compile_options(storageturbo_qm PRIVATE -Os)

vita_create_self(storageturbo_qm.suprx storageturbo_qm
  CONFIG ${CMAKE_SOURCE_DIR}/storageturbo_qm.yml
  UNSAFE)
