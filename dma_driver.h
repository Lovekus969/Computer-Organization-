// dma_driver.h
#pragma once

#include <ntddk.h>
#include <wdf.h>

#define DEVICE_NAME L"\\Device\\DmaDemoDriver"
#define SYMBOLIC_NAME L"\\DosDevices\\DmaDemoDriver"
#define BUFFER_SIZE (2 * 1024 * 1024)  // 2MB

// IOCTL codes
#define IOCTL_DMA_GET_BUFFER_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMA_PERFORM_TRANSFER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMA_FILL_PATTERN CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// DMA buffer information structure
typedef struct _DMA_BUFFER_INFO {
    ULONGLONG PhysicalAddress;
    ULONGLONG VirtualAddress;
    ULONGLONG Size;
    ULONGLONG PatternValue;
} DMA_BUFFER_INFO, *PDMA_BUFFER_INFO;

// Device context structure
typedef struct _DMA_DEVICE_CONTEXT {
    WDFDMAENABLER DmaEnabler;
    WDFCOMMONBUFFER CommonBuffer;
    PVOID VirtualAddress;
    PHYSICAL_ADDRESS PhysicalAddress;
    ULONG BufferSize;
    KSPIN_LOCK BufferLock;
} DMA_DEVICE_CONTEXT, *PDMA_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DMA_DEVICE_CONTEXT, GetDeviceContext)

// Function declarations
DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD DmaEvtDeviceAdd;
EVT_WDF_DEVICE_PREPARE_HARDWARE DmaEvtPrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE DmaEvtReleaseHardware;
EVT_WDF_IO_QUEUE_IO_READ DmaEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE DmaEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL DmaEvtIoDeviceControl;

NTSTATUS InitializeDma(PDMA_DEVICE_CONTEXT context);
VOID CleanupDma(PDMA_DEVICE_CONTEXT context);
VOID FillBufferWithPattern(PDMA_DEVICE_CONTEXT context, ULONG pattern);
VOID VerifyBufferPattern(PDMA_DEVICE_CONTEXT context, ULONG pattern);
