// dma_test_app.c
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEVICE_NAME "\\\\.\\DmaDemoDriver"
#define IOCTL_DMA_GET_BUFFER_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMA_PERFORM_TRANSFER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMA_FILL_PATTERN CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _DMA_BUFFER_INFO {
    ULONGLONG PhysicalAddress;
    ULONGLONG VirtualAddress;
    ULONGLONG Size;
    ULONGLONG PatternValue;
} DMA_BUFFER_INFO, *PDMA_BUFFER_INFO;

void PrintError(const char* operation, DWORD errorCode) {
    printf("ERROR: %s failed with code: %lu (0x%lX)\n", operation, errorCode, errorCode);
}

int main() {
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    BOOL success;
    DWORD bytesReturned;
    
    printf("=== Windows DMA Demonstration Application ===\n");
    printf("Testing DMA driver functionality...\n\n");
    
    // Step 1: Open device
    printf("1. Opening DMA device...\n");
    hDevice = CreateFileA(DEVICE_NAME,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
    
    if (hDevice == INVALID_HANDLE_VALUE) {
        PrintError("CreateFile", GetLastError());
        return 1;
    }
    printf("   Device opened successfully!\n\n");
    
    // Step 2: Get DMA buffer information
    printf("2. Getting DMA buffer information...\n");
    DMA_BUFFER_INFO bufferInfo = {0};
    success = DeviceIoControl(hDevice,
                             IOCTL_DMA_GET_BUFFER_INFO,
                             NULL, 0,
                             &bufferInfo, sizeof(bufferInfo),
                             &bytesReturned,
                             NULL);
    
    if (success && bytesReturned == sizeof(bufferInfo)) {
        printf("   DMA Buffer Information:\n");
        printf("     Physical Address: 0x%016llX\n", bufferInfo.PhysicalAddress);
        printf("     Virtual Address:  0x%016llX\n", bufferInfo.VirtualAddress);
        printf("     Size:             %llu bytes\n", bufferInfo.Size);
        printf("     Pattern:          0x%016llX\n", bufferInfo.PatternValue);
    } else {
        PrintError("IOCTL_DMA_GET_BUFFER_INFO", GetLastError());
    }
    printf("\n");
    
    // Step 3: Write data to DMA buffer
    printf("3. Writing data to DMA buffer...\n");
    const char* testData = "Hello Windows DMA World! This is a test of DMA buffer operations.";
    DWORD bytesWritten;
    
    success = WriteFile(hDevice, testData, (DWORD)strlen(testData) + 1, &bytesWritten, NULL);
    if (success) {
        printf("   Write successful: %lu bytes written\n", bytesWritten);
        printf("   Data: '%s'\n", testData);
    } else {
        PrintError("WriteFile", GetLastError());
    }
    printf("\n");
    
    // Step 4: Read data from DMA buffer
    printf("4. Reading data from DMA buffer...\n");
    char readBuffer[256] = {0};
    DWORD bytesRead;
    
    success = ReadFile(hDevice, readBuffer, sizeof(readBuffer) - 1, &bytesRead, NULL);
    if (success) {
        printf("   Read successful: %lu bytes read\n", bytesRead);
        printf("   Data: '%s'\n", readBuffer);
    } else {
        PrintError("ReadFile", GetLastError());
    }
    printf("\n");
    
    // Step 5: Fill buffer with pattern
    printf("5. Filling DMA buffer with test pattern...\n");
    success = DeviceIoControl(hDevice,
                             IOCTL_DMA_FILL_PATTERN,
                             NULL, 0,
                             NULL, 0,
                             &bytesReturned,
                             NULL);
    
    if (success) {
        printf("   Pattern fill completed successfully\n");
    } else {
        PrintError("IOCTL_DMA_FILL_PATTERN", GetLastError());
    }
    printf("\n");
    
    // Step 6: Perform DMA transfer test
    printf("6. Performing DMA transfer test...\n");
    ULONGLONG transferTime = 0;
    success = DeviceIoControl(hDevice,
                             IOCTL_DMA_PERFORM_TRANSFER,
                             NULL, 0,
                             &transferTime, sizeof(transferTime),
                             &bytesReturned,
                             NULL);
    
    if (success && bytesReturned == sizeof(transferTime)) {
        printf("   DMA transfer test completed\n");
        printf("   Transfer time: %llu nanoseconds\n", transferTime);
        
        double throughput = (1024.0 * 1024.0) / (transferTime / 1e9); // MB/s
        printf("   Estimated throughput: %.2f MB/s\n", throughput / (1024 * 1024));
    } else {
        PrintError("IOCTL_DMA_PERFORM_TRANSFER", GetLastError());
    }
    printf("\n");
    
    // Cleanup
    printf("7. Cleaning up...\n");
    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
        printf("   Device handle closed\n");
    }
    
    printf("\n=== DMA Test Completed ===\n");
    printf("All operations finished. Check kernel debug output for detailed driver messages.\n");
    
    return 0;
}
