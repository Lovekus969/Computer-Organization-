// dma_driver.c
#include "dma_driver.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;

    KdPrint(("=== DMA Demonstration Driver Loading ===\n"));
    KdPrint(("Built: %s %s\n", __DATE__, __TIME__));

    WDF_DRIVER_CONFIG_INIT(&config, DmaEvtDeviceAdd);
    config.DriverPoolTag = 'AMDD'; // DMA Demo tag

    status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to create WDF driver: 0x%X\n", status));
    }

    return status;
}

NTSTATUS DmaEvtDeviceAdd(WDFDRIVER Driver, PWDFDEVICE_INIT DeviceInit)
{
    WDFDEVICE device;
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_IO_QUEUE_CONFIG queueConfig;
    NTSTATUS status;
    UNREFERENCED_PARAMETER(Driver);

    KdPrint(("Creating DMA device...\n"));

    // Setup device name
    status = WdfDeviceInitAssignName(DeviceInit, DEVICE_NAME);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to assign device name: 0x%X\n", status));
        return status;
    }

    // Setup device context
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DMA_DEVICE_CONTEXT);
    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to create device: 0x%X\n", status));
        return status;
    }

    // Create symbolic link
    status = WdfDeviceCreateSymbolicLink(device, SYMBOLIC_NAME);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to create symbolic link: 0x%X\n", status));
        return status;
    }

    // Setup I/O queue
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);
    queueConfig.EvtIoRead = DmaEvtIoRead;
    queueConfig.EvtIoWrite = DmaEvtIoWrite;
    queueConfig.EvtIoDeviceControl = DmaEvtIoDeviceControl;

    status = WdfIoQueueCreate(device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to create I/O queue: 0x%X\n", status));
        return status;
    }

    KdPrint(("DMA device created successfully\n"));
    return STATUS_SUCCESS;
}

NTSTATUS DmaEvtPrepareHardware(WDFDEVICE Device, WDFCMRESLIST ResourcesRaw, WDFCMRESLIST ResourcesTranslated)
{
    PDMA_DEVICE_CONTEXT context = GetDeviceContext(Device);
    NTSTATUS status;
    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    KdPrint(("Preparing DMA hardware...\n"));

    status = InitializeDma(context);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to initialize DMA: 0x%X\n", status));
        return status;
    }

    KdPrint(("DMA hardware prepared successfully\n"));
    return STATUS_SUCCESS;
}

NTSTATUS DmaEvtReleaseHardware(WDFDEVICE Device, WDFCMRESLIST ResourcesTranslated)
{
    PDMA_DEVICE_CONTEXT context = GetDeviceContext(Device);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    KdPrint(("Releasing DMA hardware...\n"));
    CleanupDma(context);
    KdPrint(("DMA hardware released\n"));

    return STATUS_SUCCESS;
}

NTSTATUS InitializeDma(PDMA_DEVICE_CONTEXT context)
{
    WDF_DMA_ENABLER_CONFIG dmaConfig;
    NTSTATUS status;
    WDFDEVICE device = WdfObjectContextGetObject(context);

    KdPrint(("Initializing DMA subsystem...\n"));

    // Initialize spin lock
    KeInitializeSpinLock(&context->BufferLock);

    // Create DMA enabler
    WDF_DMA_ENABLER_CONFIG_INIT(&dmaConfig, WdfDmaProfileScatterGather64, BUFFER_SIZE);
    status = WdfDmaEnablerCreate(device, &dmaConfig, WDF_NO_OBJECT_ATTRIBUTES, &context->DmaEnabler);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to create DMA enabler: 0x%X\n", status));
        return status;
    }

    // Allocate common buffer
    status = WdfCommonBufferCreate(context->DmaEnabler, BUFFER_SIZE, WDF_NO_OBJECT_ATTRIBUTES, &context->CommonBuffer);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to allocate common buffer: 0x%X\n", status));
        return status;
    }

    // Get addresses
    context->VirtualAddress = WdfCommonBufferGetAlignedVirtualAddress(context->CommonBuffer);
    context->PhysicalAddress = WdfCommonBufferGetAlignedLogicalAddress(context->CommonBuffer);
    context->BufferSize = BUFFER_SIZE;

    // Initialize buffer with zeros
    RtlZeroMemory(context->VirtualAddress, context->BufferSize);

    KdPrint(("DMA Initialization Complete:\n"));
    KdPrint(("  Virtual Address:  0x%p\n", context->VirtualAddress));
    KdPrint(("  Physical Address: 0x%I64X\n", context->PhysicalAddress.QuadPart));
    KdPrint(("  Buffer Size:      %lu bytes\n", context->BufferSize));

    return STATUS_SUCCESS;
}

VOID CleanupDma(PDMA_DEVICE_CONTEXT context)
{
    KdPrint(("Cleaning up DMA resources...\n"));

    if (context->CommonBuffer) {
        WdfObjectDelete(context->CommonBuffer);
        context->CommonBuffer = NULL;
    }

    KdPrint(("DMA cleanup completed\n");
}

VOID DmaEvtIoRead(WDFQUEUE Queue, WDFREQUEST Request, size_t Length)
{
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    PDMA_DEVICE_CONTEXT context = GetDeviceContext(device);
    NTSTATUS status;
    PVOID outputBuffer;
    size_t bytesCopied;
    KIRQL oldIrql;

    KdPrint(("Processing Read Request: %zu bytes\n", Length));

    // Validate request
    if (Length == 0) {
        WdfRequestCompleteWithInformation(Request, STATUS_INVALID_PARAMETER, 0);
        return;
    }

    // Get output buffer
    status = WdfRequestRetrieveOutputBuffer(Request, Length, &outputBuffer, &bytesCopied);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to get output buffer: 0x%X\n", status));
        WdfRequestCompleteWithInformation(Request, status, 0);
        return;
    }

    // Lock buffer for thread-safe access
    KeAcquireSpinLock(&context->BufferLock, &oldIrql);

    // Copy data from DMA buffer
    size_t copySize = min(Length, context->BufferSize);
    RtlCopyMemory(outputBuffer, context->VirtualAddress, copySize);

    KeReleaseSpinLock(&context->BufferLock, oldIrql);

    WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, copySize);
    KdPrint(("Read completed: %zu bytes transferred\n", copySize));
}

VOID DmaEvtIoWrite(WDFQUEUE Queue, WDFREQUEST Request, size_t Length)
{
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    PDMA_DEVICE_CONTEXT context = GetDeviceContext(device);
    NTSTATUS status;
    PVOID inputBuffer;
    size_t bytesRead;
    KIRQL oldIrql;

    KdPrint(("Processing Write Request: %zu bytes\n", Length));

    // Validate request
    if (Length == 0 || Length > context->BufferSize) {
        WdfRequestCompleteWithInformation(Request, STATUS_INVALID_BUFFER_SIZE, 0);
        return;
    }

    // Get input buffer
    status = WdfRequestRetrieveInputBuffer(Request, Length, &inputBuffer, &bytesRead);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ERROR: Failed to get input buffer: 0x%X\n", status));
        WdfRequestCompleteWithInformation(Request, status, 0);
        return;
    }

    // Lock buffer for thread-safe access
    KeAcquireSpinLock(&context->BufferLock, &oldIrql);

    // Copy data to DMA buffer
    RtlCopyMemory(context->VirtualAddress, inputBuffer, Length);

    KeReleaseSpinLock(&context->BufferLock, oldIrql);

    WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, Length);
    KdPrint(("Write completed: %zu bytes transferred\n", Length));
}

VOID DmaEvtIoDeviceControl(WDFQUEUE Queue, WDFREQUEST Request, size_t OutputBufferLength, 
                          size_t InputBufferLength, ULONG IoControlCode)
{
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    PDMA_DEVICE_CONTEXT context = GetDeviceContext(device);
    NTSTATUS status = STATUS_SUCCESS;
    size_t bytesReturned = 0;
    KIRQL oldIrql;

    KdPrint(("Processing IOCTL: 0x%X\n", IoControlCode));

    switch (IoControlCode) {
        case IOCTL_DMA_GET_BUFFER_INFO: {
            PDMA_BUFFER_INFO bufferInfo;
            
            status = WdfRequestRetrieveOutputBuffer(Request, sizeof(DMA_BUFFER_INFO), 
                                                   (PVOID*)&bufferInfo, &bytesReturned);
            if (NT_SUCCESS(status)) {
                bufferInfo->PhysicalAddress = context->PhysicalAddress.QuadPart;
                bufferInfo->VirtualAddress = (ULONGLONG)context->VirtualAddress;
                bufferInfo->Size = context->BufferSize;
                bufferInfo->PatternValue = 0xDEADBEEF;
                bytesReturned = sizeof(DMA_BUFFER_INFO);
                
                KdPrint(("Returning DMA buffer info\n"));
            }
            break;
        }

        case IOCTL_DMA_FILL_PATTERN: {
            KeAcquireSpinLock(&context->BufferLock, &oldIrql);
            
            // Fill buffer with test pattern
            ULONG *buffer = (ULONG*)context->VirtualAddress;
            ULONG pattern = 0x12345678;
            for (ULONG i = 0; i < context->BufferSize / sizeof(ULONG); i++) {
                buffer[i] = pattern + i;
            }
            
            KeReleaseSpinLock(&context->BufferLock, oldIrql);
            
            KdPrint(("Buffer filled with test pattern\n"));
            bytesReturned = sizeof(ULONG);
            break;
        }

        case IOCTL_DMA_PERFORM_TRANSFER: {
            LARGE_INTEGER startTime, endTime;
            ULONG transferSize = 1024 * 1024; // 1MB transfer test
            
            KeQueryPerformanceCounter(&startTime);
            
            KeAcquireSpinLock(&context->BufferLock, &oldIrql);
            
            // Simulate DMA transfer by memory operations
            ULONG *src = (ULONG*)context->VirtualAddress;
            ULONG *dst = (ULONG*)((PUCHAR)context->VirtualAddress + transferSize);
            
            for (ULONG i = 0; i < transferSize / sizeof(ULONG); i++) {
                dst[i] = src[i];
            }
            
            KeReleaseSpinLock(&context->BufferLock, oldIrql);
            
            KeQueryPerformanceCounter(&endTime);
            
            ULONGLONG timeNs = (endTime.QuadPart - startTime.QuadPart) * 1000000000ULL / 
                              KeQueryPerformanceFrequency().QuadPart;
            
            KdPrint(("DMA transfer simulation completed in %llu ns\n", timeNs));
            bytesReturned = sizeof(ULONGLONG);
            break;
        }

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            KdPrint(("ERROR: Unknown IOCTL: 0x%X\n", IoControlCode));
            break;
    }

    WdfRequestCompleteWithInformation(Request, status, bytesReturned);
}
