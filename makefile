# HailOS Makefile (C++ & C version)

# --- Toolchain Configuration ---
TARGET_ARCH = x86_64
TARGET_TRIPLET = $(TARGET_ARCH)-elf
CXX = $(TARGET_TRIPLET)-g++
CC  = $(TARGET_TRIPLET)-gcc
AS = $(TARGET_TRIPLET)-as
LD = $(TARGET_TRIPLET)-ld
OBJCOPY = $(TARGET_TRIPLET)-objcopy

# --- Build Directories ---
BUILD_DIR = Build
OBJ_DIR = $(BUILD_DIR)/obj
KERNEL_ELF = $(BUILD_DIR)/kernel.elf

# --- Source Directories ---
KERNEL_DIR = Kernel
KERNEL_BOOT_DIR = $(KERNEL_DIR)/Boot
COMMON_DIR = Common
LIBRARY_DIR = Library
API_FILEIO_DIR = API/FileIO
UI_BITMAP_DIR = UserInterface/BitmapImage
UI_CURSOR_DIR = UserInterface/Cursor

LIB_ACPI_DIR = $(LIBRARY_DIR)/ACPI
LIB_CONSOLE_DIR = $(LIBRARY_DIR)/Console
LIB_FILESYSTEM_DIR = $(LIBRARY_DIR)/Filesystem/FAT32
LIB_HAL_DISK_DIR = $(LIBRARY_DIR)/HAL/Disk
LIB_HAL_KBD_DIR  = $(LIBRARY_DIR)/HAL/Keyboard
LIB_IO_COMMON_DIR = $(LIBRARY_DIR)/IO/Common
LIB_IO_DISK_ATA_DIR = $(LIBRARY_DIR)/IO/Disk/ATA
LIB_IO_DISK_AHCI_DIR = $(LIBRARY_DIR)/IO/Disk/AHCI
LIB_IO_PCI_DIR = $(LIBRARY_DIR)/IO/PCI
LIB_IO_PIC_DIR = $(LIBRARY_DIR)/IO/PIC
LIB_IO_PS2_DIR = $(LIBRARY_DIR)/IO/PS2
LIB_IO_PS2_KBD_DIR = $(LIBRARY_DIR)/IO/PS2/Keyboard
LIB_IO_PS2_MOUSE_DIR = $(LIBRARY_DIR)/IO/PS2/Mouse
LIB_IO_TSC_DIR = $(LIBRARY_DIR)/IO/TSC
LIB_MEMMGR_DIR = $(LIBRARY_DIR)/MemoryManager
LIB_STDLIB_DIR = $(LIBRARY_DIR)/StdLib
LIB_TIME_DIR = $(LIBRARY_DIR)/Time
LIB_VGA_DIR = $(LIBRARY_DIR)/VGA
LIB_IO_USB_XHCI_DIR = $(LIBRARY_DIR)/IO/USB/xHCI

# --- Compiler and Linker Flags ---
CXXFLAGS_COMMON = -g -ffreestanding -Wall -Wextra -mno-red-zone -mcmodel=kernel -fno-pie -fno-stack-protector -O0 -std=c++20 -fno-exceptions -fno-rtti
CXXFLAGS = $(CXXFLAGS_COMMON)
CFLAGS   = -g -ffreestanding -Wall -Wextra -mno-red-zone -mcmodel=kernel -fno-pie -fno-stack-protector -O0 -std=c11
ASFLAGS = -g

# Specific flags for interrupt.cpp
CXXFLAGS_INTERRUPT = $(CXXFLAGS_COMMON) -mgeneral-regs-only

# --- Includes ---
INCLUDES = \
    -I$(COMMON_DIR) \
    -I$(KERNEL_DIR) \
    -I$(KERNEL_BOOT_DIR) \
    -I$(LIB_CONSOLE_DIR) \
    -I$(LIB_FILESYSTEM_DIR) \
    -I$(LIB_HAL_DISK_DIR) \
    -I$(LIB_HAL_KBD_DIR) \
    -I$(LIB_IO_COMMON_DIR) \
    -I$(LIB_IO_DISK_ATA_DIR) \
    -I$(LIB_IO_DISK_AHCI_DIR) \
    -I$(LIB_IO_PCI_DIR) \
    -I$(LIB_IO_PIC_DIR) \
    -I$(LIB_IO_PS2_DIR) \
    -I$(LIB_IO_PS2_KBD_DIR) \
    -I$(LIB_IO_PS2_MOUSE_DIR) \
    -I$(LIB_IO_TSC_DIR) \
    -I$(LIB_MEMMGR_DIR) \
    -I$(LIB_STDLIB_DIR) \
    -I$(LIB_TIME_DIR) \
    -I$(LIB_VGA_DIR) \
    -I$(API_FILEIO_DIR) \
    -I$(UI_BITMAP_DIR) \
    -I$(UI_CURSOR_DIR) \
    -I$(LIB_ACPI_DIR) \
    -I$(LIB_IO_USB_XHCI_DIR)

# --- Linker ---
LDSCRIPT = linker.ld
LDFLAGS = -T $(LDSCRIPT) -nostdlib -n

# --- Source Files ---
ASM_SOURCES = \
    $(KERNEL_BOOT_DIR)/boot.S

CPP_SOURCE_INTERRUPT = $(KERNEL_BOOT_DIR)/interrupt.cpp

CPP_SOURCES = \
    $(COMMON_DIR)/common.cpp \
    $(COMMON_DIR)/status.cpp \
    $(KERNEL_DIR)/hailos.cpp \
    $(KERNEL_DIR)/kernellib.cpp \
    $(KERNEL_BOOT_DIR)/init.cpp \
    $(LIB_CONSOLE_DIR)/console.cpp \
    $(LIB_FILESYSTEM_DIR)/fat32.cpp \
    $(LIB_HAL_DISK_DIR)/hal_disk.cpp \
    $(LIB_HAL_KBD_DIR)/hal_kbd.cpp \
    $(LIB_IO_COMMON_DIR)/io.cpp \
    $(LIB_IO_DISK_ATA_DIR)/ata.cpp \
    $(LIB_IO_DISK_AHCI_DIR)/ahci.cpp \
    $(LIB_IO_PCI_DIR)/pci.cpp \
    $(LIB_IO_PIC_DIR)/pic.cpp \
    $(LIB_IO_PS2_KBD_DIR)/ps2kbd.cpp \
    $(LIB_IO_PS2_MOUSE_DIR)/ps2mouse.cpp \
    $(LIB_IO_TSC_DIR)/tsc.cpp \
    $(LIB_MEMMGR_DIR)/memmgr.cpp \
    $(LIB_MEMMGR_DIR)/memutil.cpp \
    $(LIB_STDLIB_DIR)/cstring.cpp \
    $(LIB_TIME_DIR)/time.cpp \
    $(LIB_VGA_DIR)/vga.cpp \
    $(LIB_ACPI_DIR)/acpi.cpp \
    $(API_FILEIO_DIR)/fileio.cpp \
    $(UI_BITMAP_DIR)/bitmap.cpp \
    $(UI_CURSOR_DIR)/cursor.cpp \
    $(LIB_IO_USB_XHCI_DIR)/xhci.cpp

# C sources (compiled with gcc)
C_SOURCES = \
    $(LIB_CONSOLE_DIR)/basic_font.c \
    $(LIB_IO_PS2_KBD_DIR)/keycode.c

# --- Object Files ---
OBJ_FILES_ASM = $(patsubst %.S,$(OBJ_DIR)/%.o,$(ASM_SOURCES))
OBJ_FILES_CPP = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter-out $(CPP_SOURCE_INTERRUPT),$(CPP_SOURCES)))
OBJ_FILE_INTERRUPT = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SOURCE_INTERRUPT))
OBJ_FILES_C = $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SOURCES))

ALL_OBJS = $(OBJ_FILES_ASM) $(OBJ_FILES_CPP) $(OBJ_FILE_INTERRUPT) $(OBJ_FILES_C)

# --- Build Rules ---
all: $(KERNEL_ELF)

$(KERNEL_ELF): $(ALL_OBJS) $(LDSCRIPT)
	@echo "LD   $@"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(ALL_OBJS) -lgcc

$(OBJ_DIR)/%.o: %.cpp
	@echo "CXX  $<"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_FILE_INTERRUPT): $(CPP_SOURCE_INTERRUPT)
	@echo "CXX-INT $<"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_INTERRUPT) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@echo "CC   $<"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.S
	@echo "AS   $<"
	@mkdir -p $(@D)
	$(CC) $(ASFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: all clean

clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)