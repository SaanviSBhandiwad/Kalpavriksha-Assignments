#include <stdio.h>
#include <stdlib.h>

struct Block
{
    int start;
    int size;
    int id;      // 0 when free
    int isFree;  // 1 = free, 0 = allocated
};

char *pool;
struct Block *blocks;
int count = 1, capacity = 4;
int poolSize, nextId = 1, strategy = 1;


// Function to insert a block into the blocks array
void insertBlock(int pos, int start, int size, int id, int isFree)
{
    int i;
    struct Block *temp;
    if (count == capacity)
    {
        capacity = capacity * 2;
        temp = realloc(blocks, capacity * sizeof(struct Block));
        if (temp == NULL)
        {
            printf("Out of memory!\n");
            free(pool);
            free(blocks);
            exit(1);
        }
        blocks = temp;
    }
    for (i = count; i > pos; i--)
        blocks[i] = blocks[i - 1];
    blocks[pos].start = start;
    blocks[pos].size = size;
    blocks[pos].id = id;
    blocks[pos].isFree = isFree;
    count++;
}

// Function to remove a block from the blocks array
void removeBlock(int pos)
{
    int i;
    for (i = pos; i < count - 1; i++)
        blocks[i] = blocks[i + 1];
    count--;
}

// Function to merge adjacent free blocks
void mergeFreeBlocks()
{
    int i = 0;
    while (i < count - 1)
    {
        if (blocks[i].isFree && blocks[i + 1].isFree)
        {
            blocks[i].size += blocks[i + 1].size;
            removeBlock(i + 1);
        }
        else
            i++;
    }
}

// Function to find a free block based on the current strategy
int findFreeBlock(int size)
{
    int i, chosen = -1;
    for (i = 0; i < count; i++)
    {
        if (!blocks[i].isFree || blocks[i].size < size)
            continue;
        if (strategy == 1)
            return i;
        if (chosen == -1)
            chosen = i;
        else if (strategy == 2 && blocks[i].size < blocks[chosen].size)
            chosen = i;
        else if (strategy == 3 && blocks[i].size > blocks[chosen].size)
            chosen = i;
    }
    return chosen;
}

// Function to split a block into an allocated part and a free part
void splitBlock(int pos, int size, int id)
{
    int leftover = blocks[pos].size - size;
    int leftoverStart = blocks[pos].start + size;
    blocks[pos].size = size;
    blocks[pos].isFree = 0;
    blocks[pos].id = id;
    if (leftover > 0)
        insertBlock(pos + 1, leftoverStart, leftover, 0, 1);
}

// Function to allocate memory
void allocateMemory(int size)
{
    int pos = findFreeBlock(size);
    int i, totalFree = 0, id;
    if (pos == -1)
    {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        for (i = 0; i < count; i++)
        {
            if (blocks[i].isFree)
                totalFree += blocks[i].size;
        }

        if (totalFree >= size)
        {
            printf("External fragmentation: %d bytes are free in total,\n", totalFree);
            printf("but they are in separate pieces of:");
            for (i = 0; i < count; i++)
                if (blocks[i].isFree)
                    printf(" %d", blocks[i].size);
            printf("\n");
        }
        return;
    }
    id = nextId++;
    splitBlock(pos, size, id);
    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n",
           id, blocks[pos].start, size);
}

// Function to release an allocation
void releaseMemory(int id)
{
    int i, pos = -1;
    for (i = 0; i < count; i++)
        if (!blocks[i].isFree && blocks[i].id == id)
            pos = i;
    if (pos == -1)
    {
        if (id < nextId)
            printf("Error: allocation %d is already free.\n", id);
        else
            printf("Error: unknown allocation ID %d.\n", id);
        return;
    }
    blocks[pos].isFree = 1;
    blocks[pos].id = 0;
    mergeFreeBlocks();
    printf("Allocation released.\n");
}

// Function to resize an existing allocation
void resizeMemory(int id, int newSize)
{
    int i, pos = -1;
    int oldSize, oldStart, extra, newPos, newStart;

    for (i = 0; i < count; i++)
        if (!blocks[i].isFree && blocks[i].id == id)
            pos = i;
    if (pos == -1)
    {
        printf("Error: unknown allocation ID %d.\n", id);
        return;
    }
    oldSize = blocks[pos].size;
    oldStart = blocks[pos].start;

    if (newSize == oldSize)
    {
        printf("Size is already %d.\n", newSize);
        return;
    }

    // Shrink
    if (newSize < oldSize)
    {
        blocks[pos].size = newSize;
        insertBlock(pos + 1, oldStart + newSize, oldSize - newSize, 0, 1);
        mergeFreeBlocks();
        printf("Allocation %d shrunk to %d bytes.\n", id, newSize);
        return;
    }

    // Grow in place
    extra = newSize - oldSize;
    if (pos + 1 < count && blocks[pos + 1].isFree && blocks[pos + 1].size >= extra)
    {
        blocks[pos].size = newSize;
        blocks[pos + 1].start += extra;
        blocks[pos + 1].size -= extra;
        if (blocks[pos + 1].size == 0)
            removeBlock(pos + 1);
        printf("Allocation %d grown in place to %d bytes.\n", id, newSize);
        return;
    }

    // Move to a new place
    newPos = findFreeBlock(newSize);
    if (newPos == -1)
    {
        printf("Resize failed. Allocation %d is unchanged.\n", id);
        return;
    }
    splitBlock(newPos, newSize, id);
    newStart = blocks[newPos].start;
    for (i = 0; i < oldSize; i++)
        pool[newStart + i] = pool[oldStart + i];
    for (i = 0; i < count; i++)
        if (blocks[i].start == oldStart)
        {
            blocks[i].isFree = 1;
            blocks[i].id = 0;
        }
    mergeFreeBlocks();
    printf("Allocation %d moved to Start: %d, new size %d bytes.\n", id, newStart, newSize);
}

// Function to show memory statistics
void showStats()
{
    int i, usedBytes = 0, freeBytes = 0;
    int usedBlocks = 0, freeBlocks = 0, largestFree = 0;
    for (i = 0; i < count; i++)
    {
        if (blocks[i].isFree)
        {
            freeBytes += blocks[i].size;
            freeBlocks++;
            if (blocks[i].size > largestFree)
                largestFree = blocks[i].size;
        }
        else
        {
            usedBytes += blocks[i].size;
            usedBlocks++;
        }
    }
    printf("Total memory: %d bytes\n", poolSize);
    printf("Allocated: %d bytes\n", usedBytes);
    printf("Allocated blocks: %d\n", usedBlocks);
    printf("Free: %d bytes\n", freeBytes);
    printf("Free blocks: %d\n", freeBlocks);
    printf("Largest free block: %d bytes\n", largestFree);
}

void chooseStrategy()
{
    int choice = 0;
    while (choice < 1 || choice > 3)
    {
        printf("Select strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ");
        scanf("%d", &choice);
    }
    strategy = choice;
    if (strategy == 1)
        printf("Strategy set to First Fit.\n");
    else if (strategy == 2)
        printf("Strategy set to Best Fit.\n");
    else
        printf("Strategy set to Worst Fit.\n");
}

int main()
{
    int choice = 0, id, size, i;

    printf("Enter memory pool size: ");
    scanf("%d", &poolSize);
    while (poolSize < 1 || poolSize > 1000000)
    {
        printf("Invalid. Size must be between 1 and 1000000.\n");
        printf("Enter memory pool size: ");
        scanf("%d", &poolSize);
    }

    pool = malloc(poolSize);
    blocks = malloc(capacity * sizeof(struct Block));
    if (pool == NULL || blocks == NULL)
    {
        printf("Could not allocate memory.\n");
        free(pool);
        free(blocks);
        return 1;
    }
    blocks[0].start = 0;
    blocks[0].size = poolSize;
    blocks[0].id = 0;
    blocks[0].isFree = 1;

    chooseStrategy();

    while (choice != 7)
    {
        printf("\nMenu:\n");
        printf("1. Allocate memory\n");
        printf("2. Release memory\n");
        printf("3. Resize allocation\n");
        printf("4. Display memory map\n");
        printf("5. Display memory statistics\n");
        printf("6. Change allocation strategy\n");
        printf("7. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        if (choice == 1)
        {
            printf("Requested size: ");
            scanf("%d", &size);
            if (size < 1 || size > poolSize)
                printf("Invalid size. Must be 1 to %d.\n", poolSize);
            else
                allocateMemory(size);
        }
        else if (choice == 2)
        {
            printf("Allocation ID: ");
            scanf("%d", &id);
            if (id < 1)
                printf("Invalid ID.\n");
            else
                releaseMemory(id);
        }
        else if (choice == 3)
        {
            printf("Allocation ID: ");
            scanf("%d", &id);
            printf("New size: ");
            scanf("%d", &size);
            if (id < 1 || size < 1 || size > poolSize)
                printf("Invalid ID or size.\n");
            else
                resizeMemory(id, size);
        }
        else if (choice == 4)
        {
            printf("Memory Map (%d bytes):\n", poolSize);
            for (i = 0; i < count; i++)
            {
                if (blocks[i].isFree)
                    printf("[Free: %d bytes, Start: %d]\n", blocks[i].size, blocks[i].start);
                else
                    printf("[Allocated #%d: %d bytes, Start: %d]\n",
                           blocks[i].id, blocks[i].size, blocks[i].start);
            }
        }
        else if (choice == 5)
            showStats();
        else if (choice == 6)
            chooseStrategy();
        else if (choice != 7)
            printf("Invalid choice.\n");
    }

    free(pool);
    free(blocks);
    return 0;
}