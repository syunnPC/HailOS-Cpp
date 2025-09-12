#include "status.hpp"

namespace HailOS
{
    const char* statusToString(Status status)
    {
        switch (status)
        {
            default:
                return "unknown Status";
            case Status::STATUS_SUCCESS:
                return "STATUS_SUCCESS";
            case Status::STATUS_ERROR:
                return "STATUS_ERROR";
            case Status::STATUS_UNREACHABLE:
                return "STATUS_UNREACHABLE";
            case Status::STATUS_NOT_INITIALIZED:
                return "STATUS_NOT_INITIALIZED";
            case Status::STATUS_INVALID_PARAMETER:
                return "STATUS_INVALID_PARAMETER";
            case Status::STATUS_FILE_NOT_FOUND:
                return "STATUS_FILE_NOT_FOUND";
            case Status::STATUS_NOT_IMPLEMENTED:
                return "STATUS_NOT_IMPLEMENTED";
            case Status::STATUS_DISK_IO_ERROR:
                return "STATUS_DISK_IO_ERROR";
            case Status::STATUS_DEVICE_NOT_FOUND:
                return "STATUS_DEVICE_NOT_FOUND";
            case Status::STATUS_MEMORY_ALLOCATION_FAILED:
                return "STATUS_MEMORY_ALLOCATION_FAILED";
            case Status::STATUS_FAT32_FILESYSTEM:
                return "STATUS_FAT32_FILESYSTEM";
            case Status::STATUS_DIVIDE_ERROR:
                return "STATUS_DIVIDE_ERROR";
            case Status::STATUS_DEBUG_EXCEPTION:
                return "STATUS_DEBUG_EXCEPTION";
            case Status::STATUS_NON_MASKABLE_INTERRUPT:
                return "STATUS_NON_MASKABLE_INTERRUPT";
            case Status::STATUS_BREAKPOINT_HIT:
                return "STATUS_BREAKPOINT_HIT";
            case Status::STATUS_OVERFLOW:
                return "STATUS_OVERFLOW";
            case Status::STATUS_BOUND_RANGE_EXCEEDED:
                return "STATUS_BOUND_RANGE_EXCEEDED";
            case Status::STATUS_INVALID_OPCODE:
                return "STATUS_INVALID_OPCODE";
            case Status::STATUS_FPU_MMX_DEVICE_NOT_AVAILABLE:
                return "STATUS_FPU_MMX_DEVICE_NOT_AVAILABLE";
            case Status::STATUS_DOUBLE_FAULT:
                return "STATUS_DOUBLE_FAULT";
            case Status::STATUS_COPROCESSOR_SEGMENT_OVERRUN:
                return "STATUS_COPROCESSOR_SEGMENT_OVERRUN";
            case Status::STATUS_INVALID_TSS:
                return "STATUS_INVALID_TSS";
            case Status::STATUS_SEGMENT_NOT_PRESENT:
                return "STATUS_SEGMENT_NOT_PRESENT";
            case Status::STATUS_STACK_SEGMENT_FAULT:
                return "STATUS_STACK_SEGMENT_FAULT";
            case Status::STATUS_GENERAL_PROTECTION_FAULT:
                return "STATUS_GENERAL_PROTECTION_FAULT";
            case Status::STATUS_PAGE_FAULT:
                return "STATUS_PAGE_FAULT";
            case Status::STATUS_FPU_FP_EXCEPTION:
                return "STATUS_FPU_FP_EXCEPTION";
            case Status::STATUS_ALIGNMENT_CHECK:
                return "STATUS_ALIGNMENT_CHECK";
            case Status::STATUS_MACHINE_CHECK_EXCEPTION:
                return "STATUS_MACHINE_CHECK_EXCEPTION";
            case Status::STATUS_SIMD_FP_EXCEPTION:
                return "STATUS_SIMD_FP_EXCEPTION";
            case Status::STATUS_VIRTUALIZATION_EXCEPTION:
                return "STATUS_VIRTUALIZATION_EXCEPTION";
            case Status::STATUS_CONTROL_PROTECTION_EXCEPTION:
                return "STATUS_CONTROL_PROTECTION_EXCEPTION";
            case Status::STATUS_HYPERVISOR_INJECTION_EXCEPTION:
                return "STATUS_HYPERVISOR_INJECTION_EXCEPTION";
            case Status::STATUS_VMM_COMMUNICATION_EXCEPTION:
                return "STATUS_VMM_COMMUNICATION_EXCEPTION";
            case Status::STATUS_SECURITY_EXCEPTION:
                return "STATUS_SECURITY_EXCEPTION";
            case Status::STATUS_ACPI_ERROR:
                return "STATUS_ACPI_ERROR";
        }
    }
}