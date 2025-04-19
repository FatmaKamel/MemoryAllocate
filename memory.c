#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For using the 'bool' type

// Structure to represent a block of memory
typedef struct MemoryBlock {
    int startAddress;
    int size;
    char processName[10];
    bool isAllocated; // Using boolean type for allocation status
    struct MemoryBlock* next;
} MemoryBlock;

// Global pointer to the head of the memory block list
MemoryBlock* memoryHead = NULL;
// Global variable to store the total size of the memory
int totalMemorySize = 0;

// Function prototypes for memory management operations
void initializeMemory(int size);
void allocateMemory(char processName[], int size, char allocationStrategy);
void releaseMemory(char processName[]);
void compactMemory();
void printMemoryStatus();

// Main function - Entry point of the program
int main() {
    int memorySize;

    printf("Enter the initial memory size: ");
    if (scanf("%d", &memorySize) != 1) {
        fprintf(stderr, "Invalid input for memory size.\n");
        return EXIT_FAILURE;
    }
    totalMemorySize = memorySize;
    initializeMemory(totalMemorySize);

    char command[5];
    char processName[10];
    int size;
    char strategy; // User input for allocation strategy

    printf("\nMemory Allocator Commands:\n");
    printf("RQ <processName> <size> <strategy(F/B/W)>: Request memory allocation\n");
    printf("RL <processName>: Release allocated memory\n");
    printf("C: Compact memory\n");
    printf("ST: Print memory status\n");
    printf("EX: Exit the program\n");

    while (true) { // Using 'true' for an infinite loop until exit
        printf("allocator> ");
        if (scanf("%s", command) != 1) {
            fprintf(stderr, "Error reading command.\n");
            break;
        }

        if (strcmp(command, "RQ") == 0) {
            printf("Enter: processName size strategy (F/B/W)\n");
            if (scanf("%s %d %c", processName, &size, &strategy) != 3) {
                fprintf(stderr, "Invalid input for request command.\n");
                // Clear input buffer to prevent issues in the next iteration
                while (getchar() != '\n');
                continue;
            }
            allocateMemory(processName, size, strategy);
        } else if (strcmp(command, "RL") == 0) {
            printf("Enter: processName\n");
            if (scanf("%s", processName) != 1) {
                fprintf(stderr, "Invalid input for release command.\n");
                while (getchar() != '\n');
                continue;
            }
            releaseMemory(processName);
        } else if (strcmp(command, "C") == 0) {
            compactMemory();
        } else if (strcmp(command, "STAT") == 0) {
            printMemoryStatus();
        } else if (strcmp(command, "X") == 0) {
            break; // Exit the loop, ending the program
        } else {
            printf("Invalid command. Please use one of the listed commands.\n");
        }
    }

    // Cleanup: Free all allocated memory blocks before exiting
    MemoryBlock* current = memoryHead;
    while (current != NULL) {
        MemoryBlock* nextBlock = current->next;
        free(current);
        current = nextBlock;
    }
    memoryHead = NULL; // Reset the head pointer

    return EXIT_SUCCESS; // Indicate successful program execution
}

// Function to initialize the memory with a single free block
void initializeMemory(int size) {
    memoryHead = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (memoryHead == NULL) {
        perror("Failed to allocate memory for initialization");
        exit(EXIT_FAILURE);
    }
    memoryHead->startAddress = 0;
    memoryHead->size = size;
    strcpy(memoryHead->processName, "");
    memoryHead->isAllocated = false; // Initialize as not allocated
    memoryHead->next = NULL;
}

// Function to allocate memory to a process using the specified strategy
void allocateMemory(char processName[], int size, char allocationStrategy) {
    MemoryBlock* currentBlock = memoryHead;
    MemoryBlock* bestFitBlock = NULL;
    MemoryBlock* worstFitBlock = NULL;
    int bestFitSize = totalMemorySize + 1;
    int worstFitSize = 0;

    // Iterate through the memory blocks to find a suitable free block
    while (currentBlock != NULL) {
        if (!currentBlock->isAllocated && currentBlock->size >= size) {
            if (allocationStrategy == 'F') {
                break; // First-fit: Found the first suitable block, so stop searching
            } else if (allocationStrategy == 'B' && currentBlock->size < bestFitSize) {
                // Best-fit: Update if the current block is smaller and suitable
                bestFitSize = currentBlock->size;
                bestFitBlock = currentBlock;
            } else if (allocationStrategy == 'W' && currentBlock->size > worstFitSize) {
                // Worst-fit: Update if the current block is larger and suitable
                worstFitSize = currentBlock->size;
                worstFitBlock = currentBlock;
            }
        }
        currentBlock = currentBlock->next;
    }

    // Select the block based on the chosen allocation strategy
    MemoryBlock* selectedBlock = NULL;
    if (allocationStrategy == 'B' && bestFitBlock != NULL) {
        selectedBlock = bestFitBlock;
    } else if (allocationStrategy == 'W' && worstFitBlock != NULL) {
        selectedBlock = worstFitBlock;
    } else if (allocationStrategy == 'F' && currentBlock != NULL) {
        selectedBlock = currentBlock;
    }

    // If a suitable block is found, allocate the memory
    if (selectedBlock != NULL) {
        if (selectedBlock->size == size) {
            // Exact fit: Allocate the entire block
            selectedBlock->isAllocated = true;
            strcpy(selectedBlock->processName, processName);
        } else {
            // Split the block: Create a new allocated block and update the existing free block
            MemoryBlock* newAllocatedBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
            if (newAllocatedBlock == NULL) {
                perror("Failed to allocate memory for the new process block");
                return;
            }
            newAllocatedBlock->startAddress = selectedBlock->startAddress;
            newAllocatedBlock->size = size;
            strcpy(newAllocatedBlock->processName, processName);
            newAllocatedBlock->isAllocated = true;
            newAllocatedBlock->next = selectedBlock;

            selectedBlock->startAddress += size;
            selectedBlock->size -= size;

            // Update the linked list to insert the new allocated block
            if (selectedBlock == memoryHead) {
                memoryHead = newAllocatedBlock;
            } else {
                MemoryBlock* temp = memoryHead;
                while (temp->next != selectedBlock) {
                    temp = temp->next;
                }
                temp->next = newAllocatedBlock;
            }
        }
        printf("Allocated %d bytes to process '%s' using %c strategy\n", size, processName, allocationStrategy);
    } else {
        printf("Error: Not enough memory available for process '%s'\n", processName);
    }
}

// Function to release memory allocated to a process
void releaseMemory(char processName[]) {
    MemoryBlock* currentBlock = memoryHead;

    // Find the memory block associated with the given process name
    while (currentBlock != NULL) {
        if (currentBlock->isAllocated && strcmp(currentBlock->processName, processName) == 0) {
            currentBlock->isAllocated = false;
            strcpy(currentBlock->processName, "");

            MemoryBlock* previousBlock = NULL;
            MemoryBlock* nextBlock = currentBlock->next;
            MemoryBlock* temp = memoryHead;

            // Find the previous block in the linked list
            if (currentBlock != memoryHead) {
                while (temp->next != currentBlock) {
                    temp = temp->next;
                }
                previousBlock = temp;
            }

            // Coalesce with the next free block if it exists
            if (nextBlock != NULL && !nextBlock->isAllocated) {
                currentBlock->size += nextBlock->size;
                currentBlock->next = nextBlock->next;
                free(nextBlock);
                nextBlock = currentBlock->next; // Update next block after freeing
            }

            // Coalesce with the previous free block if it exists
            if (previousBlock != NULL && !previousBlock->isAllocated) {
                previousBlock->size += currentBlock->size;
                previousBlock->next = currentBlock->next;
                free(currentBlock);
                currentBlock = previousBlock; // Move current block pointer for the next iteration if needed
            } else if (previousBlock == NULL) {
                currentBlock->startAddress = 0; // Reset start address if it's the first block
            }

            printf("Successfully released memory for process '%s'\n", processName);
            return;
        }
        currentBlock = currentBlock->next;
    }

    printf("Error: Process '%s' not found in allocated memory.\n", processName);
}

// Function to compact the memory by moving all allocated blocks to the beginning
void compactMemory() {
    MemoryBlock* currentBlock = memoryHead;
    MemoryBlock* newHead = NULL;
    MemoryBlock* newTailUsed = NULL;
    int currentAddress = 0;
    int totalFreeSize = 0;

    // First pass: Iterate through the current memory blocks
    while (currentBlock != NULL) {
        if (currentBlock->isAllocated) {
            // Create a new block for the allocated segment
            MemoryBlock* newAllocatedBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
            if (newAllocatedBlock == NULL) {
                perror("Failed to allocate memory during compaction");
                // Basic cleanup of the new list if allocation fails
                MemoryBlock* temp = newHead;
                while (temp != NULL) {
                    MemoryBlock* next = temp->next;
                    free(temp);
                    temp = next;
                }
                memoryHead = NULL;
                return;
            }
            newAllocatedBlock->startAddress = currentAddress;
            newAllocatedBlock->size = currentBlock->size;
            strcpy(newAllocatedBlock->processName, currentBlock->processName);
            newAllocatedBlock->isAllocated = true;
            newAllocatedBlock->next = NULL;

            // Add the new allocated block to the new list
            if (newHead == NULL) {
                newHead = newAllocatedBlock;
                newTailUsed = newAllocatedBlock;
            } else {
                newTailUsed->next = newAllocatedBlock;
                newTailUsed = newAllocatedBlock;
            }
            currentAddress += currentBlock->size;
        } else {
            totalFreeSize += currentBlock->size;
        }
        MemoryBlock* temp = currentBlock;
        currentBlock = currentBlock->next;
        free(temp); // Free the original block
    }

    // Create a single free block at the end if there is any free space
    if (totalFreeSize > 0) {
        MemoryBlock* freeBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        if (freeBlock == NULL) {
            perror("Failed to allocate memory for the free block after compaction");
            // Basic cleanup of the new list if allocation fails
            MemoryBlock* temp = newHead;
            while (temp != NULL) {
                MemoryBlock* next = temp->next;
                free(temp);
                temp = next;
            }
            memoryHead = NULL;
            return;
        }
        freeBlock->startAddress = currentAddress;
        freeBlock->size = totalFreeSize;
        strcpy(freeBlock->processName, "");
        freeBlock->isAllocated = false;
        freeBlock->next = NULL;

        // Add the free block to the end of the new list
        if (newHead == NULL) {
            newHead = freeBlock;
        } else {
            newTailUsed->next = freeBlock;
        }
    }

    // Update the head of the memory block list to the new compacted list
    memoryHead = newHead;
    printf("Memory compaction completed.\n");
}

// Function to print the current status of the memory blocks
void printMemoryStatus() {
    MemoryBlock* currentBlock = memoryHead;
    printf("\nMemory Status:\n");
    while (currentBlock != NULL) {
        printf("Addresses [%d:%d] ", currentBlock->startAddress, currentBlock->startAddress + currentBlock->size - 1);
        if (currentBlock->isAllocated) {
            printf("Process '%s'\n", currentBlock->processName);
        } else {
            printf("Unused\n");
        }
        currentBlock = currentBlock->next;
    }
    printf("Total memory size: %d bytes\n\n", totalMemorySize);
}
