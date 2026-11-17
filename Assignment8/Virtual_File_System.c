#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define MAX_NAME_LENGTH 50
#define MAX_LINE_LENGTH 2048

#if NUM_BLOCKS < 1 || NUM_BLOCKS > 5000
#error "NUM_BLOCKS must be between 1 and 5000"
#endif

typedef struct FreeBlock
{
    int index;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode
{
    char name[MAX_NAME_LENGTH + 1];
    int isDirectory;
    struct FileNode *parent;
    struct FileNode *next;
    struct FileNode *prev;
    struct FileNode *firstChild;
    int *blockPointers;
    int blockCount;
    int contentSize;
} FileNode;

unsigned char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];
FreeBlock *freeHead = NULL;
FreeBlock *freeTail = NULL;
int freeBlockCount = 0;
FileNode *rootDir = NULL;
FileNode *cwd = NULL;

// Adds a new block with the given index at the tail of the free list
void addFreeBlockAtTail(int blockIndex)
{
    FreeBlock *newBlock = (FreeBlock *)malloc(sizeof(FreeBlock));
    if (newBlock == NULL)
    {
        printf("Out of memory!\n");
        exit(1);
    }
    newBlock->index = blockIndex;
    newBlock->next = NULL;
    newBlock->prev = freeTail;

    if (freeTail == NULL)
    {
        freeHead = newBlock;
    }
    else
    {
        freeTail->next = newBlock;
    }
    freeTail = newBlock;
    freeBlockCount++;
}

// Removes the head block of the free list and returns its index (-1 if none)
int removeFreeBlockFromHead()
{
    FreeBlock *removedBlock;
    int blockIndex;

    if (freeHead == NULL)
    {
        return -1;
    }

    removedBlock = freeHead;
    blockIndex = removedBlock->index;
    freeHead = removedBlock->next;

    if (freeHead == NULL)
    {
        freeTail = NULL;
    }
    else
    {
        freeHead->prev = NULL;
    }

    free(removedBlock);
    freeBlockCount--;
    return blockIndex;
}

// Creates and returns a new file or directory node
FileNode *createNode(const char *name, int isDirectory, FileNode *parentNode)
{
    FileNode *newNode = (FileNode *)malloc(sizeof(FileNode));
    if (newNode == NULL)
    {
        printf("Out of memory!\n");
        exit(1);
    }
    strcpy(newNode->name, name);
    newNode->isDirectory = isDirectory;
    newNode->parent = parentNode;
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->firstChild = NULL;
    newNode->blockPointers = NULL;
    newNode->blockCount = 0;
    newNode->contentSize = 0;
    return newNode;
}

// Inserts a node at the end of the directory's circular list
void addChild(FileNode *directory, FileNode *child)
{
    FileNode *firstEntry;
    FileNode *lastEntry;

    if (directory->firstChild == NULL)
    {
        directory->firstChild = child;
        child->next = child;
        child->prev = child;
    }
    else
    {
        firstEntry = directory->firstChild;
        lastEntry = firstEntry->prev;
        lastEntry->next = child;
        child->prev = lastEntry;
        child->next = firstEntry;
        firstEntry->prev = child;
    }
}

// Unlinks a node from the directory's circular list
void removeChild(FileNode *directory, FileNode *child)
{
    if (child->next == child)
    {
        directory->firstChild = NULL;
    }
    else
    {
        child->prev->next = child->next;
        child->next->prev = child->prev;
        if (directory->firstChild == child)
        {
            directory->firstChild = child->next;
        }
    }
    child->next = NULL;
    child->prev = NULL;
}

// Searches a directory for an entry with the given name, returns NULL if not found
FileNode *findChild(FileNode *directory, const char *name)
{
    FileNode *current = directory->firstChild;

    if (current == NULL)
    {
        return NULL;
    }

    do
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    while (current != directory->firstChild);

    return NULL;
}

// Returns all blocks of a file to the tail of the free list
void freeFileBlocks(FileNode *file)
{
    int i;

    for (i = 0; i < file->blockCount; i++)
    {
        addFreeBlockAtTail(file->blockPointers[i]);
    }
    if (file->blockPointers != NULL)
    {
        free(file->blockPointers);
    }
    file->blockPointers = NULL;
    file->blockCount = 0;
    file->contentSize = 0;
}

// Prints the path of a node by going up through parent pointers
void printPathParts(FileNode *node)
{
    if (node->parent == NULL)
    {
        return;
    }
    printPathParts(node->parent);
    printf("/%s", node->name);
}

// Prints the full path of a directory ("/" for root)
void printPath(FileNode *node)
{
    if (node == rootDir)
    {
        printf("/");
    }
    else
    {
        printPathParts(node);
    }
}

// Checks that a file or directory name is valid (1 to 50 characters, no '/')
int isValidName(const char *name)
{
    if (strlen(name) == 0 || strlen(name) > MAX_NAME_LENGTH)
    {
        printf("Error: name must be 1 to %d characters.\n", MAX_NAME_LENGTH);
        return 0;
    }
    if (strchr(name, '/') != NULL)
    {
        printf("Error: name cannot contain '/'.\n");
        return 0;
    }
    return 1;
}

// mkdir: creates a new directory inside the current directory
void doMkdir(const char *name)
{
    FileNode *newDir;

    if (!isValidName(name))
    {
        return;
    }
    if (findChild(cwd, name) != NULL)
    {
        printf("Error: '%s' already exists.\n", name);
        return;
    }
    newDir = createNode(name, 1, cwd);
    addChild(cwd, newDir);
    printf("Directory '%s' created successfully.\n", name);
}

// create: creates a new empty file (no blocks) inside the current directory
void doCreate(const char *name)
{
    FileNode *newFile;

    if (!isValidName(name))
    {
        return;
    }
    if (findChild(cwd, name) != NULL)
    {
        printf("Error: '%s' already exists.\n", name);
        return;
    }
    newFile = createNode(name, 0, cwd);
    addChild(cwd, newFile);
    printf("File '%s' created successfully.\n", name);
}

// write: allocates blocks from the free list and stores the content in the file
void doWrite(const char *name, const char *content)
{
    FileNode *targetFile = findChild(cwd, name);
    int contentLength;
    int blocksNeeded;
    int i;
    int blockIndex;
    int startOffset;
    int bytesToCopy;

    if (targetFile == NULL)
    {
        printf("Error: file '%s' not found.\n", name);
        return;
    }
    if (targetFile->isDirectory)
    {
        printf("Error: '%s' is a directory.\n", name);
        return;
    }

    contentLength = (int)strlen(content);
    blocksNeeded = (contentLength + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (blocksNeeded > freeBlockCount + targetFile->blockCount)
    {
        printf("Error: disk full, write failed.\n");
        return;
    }

    freeFileBlocks(targetFile);

    if (blocksNeeded > 0)
    {
        targetFile->blockPointers = (int *)malloc(blocksNeeded * sizeof(int));
        if (targetFile->blockPointers == NULL)
        {
            printf("Out of memory!\n");
            exit(1);
        }
    }

    for (i = 0; i < blocksNeeded; i++)
    {
        blockIndex = removeFreeBlockFromHead();
        targetFile->blockPointers[i] = blockIndex;

        startOffset = i * BLOCK_SIZE;
        bytesToCopy = contentLength - startOffset;
        if (bytesToCopy > BLOCK_SIZE)
        {
            bytesToCopy = BLOCK_SIZE;
        }
        memset(virtualDisk[blockIndex], 0, BLOCK_SIZE);
        memcpy(virtualDisk[blockIndex], content + startOffset, bytesToCopy);
    }

    targetFile->blockCount = blocksNeeded;
    targetFile->contentSize = contentLength;
    printf("Data written successfully (size=%d bytes).\n", contentLength);
}

// read: prints the file content by going through its block pointers
void doRead(const char *name)
{
    FileNode *targetFile = findChild(cwd, name);
    int i;
    int bytesLeft;
    int bytesToPrint;

    if (targetFile == NULL)
    {
        printf("Error: file '%s' not found.\n", name);
        return;
    }
    if (targetFile->isDirectory)
    {
        printf("Error: '%s' is a directory.\n", name);
        return;
    }
    if (targetFile->blockCount == 0)
    {
        printf("(empty)\n");
        return;
    }

    bytesLeft = targetFile->contentSize;
    for (i = 0; i < targetFile->blockCount; i++)
    {
        bytesToPrint = bytesLeft;
        if (bytesToPrint > BLOCK_SIZE)
        {
            bytesToPrint = BLOCK_SIZE;
        }
        fwrite(virtualDisk[targetFile->blockPointers[i]], 1, bytesToPrint, stdout);
        bytesLeft = bytesLeft - bytesToPrint;
    }
    printf("\n");
}

// delete: removes a file and returns its blocks to the free list
void doDelete(const char *name)
{
    FileNode *targetFile = findChild(cwd, name);

    if (targetFile == NULL)
    {
        printf("Error: file '%s' not found.\n", name);
        return;
    }
    if (targetFile->isDirectory)
    {
        printf("Error: '%s' is a directory, use rmdir.\n", name);
        return;
    }

    freeFileBlocks(targetFile);
    removeChild(cwd, targetFile);
    free(targetFile);
    printf("File deleted successfully.\n");
}

// rmdir: removes a directory only if it is empty
void doRmdir(const char *name)
{
    FileNode *targetDir = findChild(cwd, name);

    if (targetDir == NULL)
    {
        printf("Error: directory '%s' not found.\n", name);
        return;
    }
    if (!targetDir->isDirectory)
    {
        printf("Error: '%s' is a file, use delete.\n", name);
        return;
    }
    if (targetDir->firstChild != NULL)
    {
        printf("Error: directory '%s' is not empty.\n", name);
        return;
    }

    removeChild(cwd, targetDir);
    free(targetDir);
    printf("Directory removed successfully.\n");
}

// ls: lists all files and directories in the current directory
void doLs()
{
    FileNode *current = cwd->firstChild;

    if (current == NULL)
    {
        printf("(empty)\n");
        return;
    }

    do
    {
        if (current->isDirectory)
        {
            printf("%s/\n", current->name);
        }
        else
        {
            printf("%s\n", current->name);
        }
        current = current->next;
    }
    while (current != cwd->firstChild);
}

// cd: changes the current working directory (supports "..", "/" and a directory name)
void doCd(const char *name)
{
    FileNode *targetDir;

    if (strcmp(name, "..") == 0)
    {
        if (cwd->parent != NULL)
        {
            cwd = cwd->parent;
        }
    }
    else if (strcmp(name, "/") == 0)
    {
        cwd = rootDir;
    }
    else if (strcmp(name, ".") != 0)
    {
        targetDir = findChild(cwd, name);
        if (targetDir == NULL)
        {
            printf("Error: directory '%s' not found.\n", name);
            return;
        }
        if (!targetDir->isDirectory)
        {
            printf("Error: '%s' is not a directory.\n", name);
            return;
        }
        cwd = targetDir;
    }

    printf("Moved to ");
    printPath(cwd);
    printf("\n");
}

// pwd: prints the path of the current working directory
void doPwd()
{
    printPath(cwd);
    printf("\n");
}

// df: shows total, used and free blocks along with disk usage percentage
void doDf()
{
    int usedBlocks = NUM_BLOCKS - freeBlockCount;
    double diskUsage = (usedBlocks * 100.0) / NUM_BLOCKS;

    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", usedBlocks);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", diskUsage);
}

// Sets up the virtual disk, fills the free list and creates the root directory
void initFileSystem()
{
    int i;

    memset(virtualDisk, 0, sizeof(virtualDisk));
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        addFreeBlockAtTail(i);
    }
    rootDir = createNode("/", 1, NULL);
    cwd = rootDir;
}

// Frees a node and everything stored under it
void freeTree(FileNode *node)
{
    FileNode *child;

    while (node->firstChild != NULL)
    {
        child = node->firstChild;
        removeChild(node, child);
        freeTree(child);
    }
    if (node->blockPointers != NULL)
    {
        free(node->blockPointers);
    }
    free(node);
}

// exit: releases the whole directory tree and the free list
void cleanupFileSystem()
{
    FreeBlock *currentBlock;

    freeTree(rootDir);
    rootDir = NULL;
    cwd = NULL;

    while (freeHead != NULL)
    {
        currentBlock = freeHead;
        freeHead = freeHead->next;
        free(currentBlock);
    }
    freeTail = NULL;
    freeBlockCount = 0;

    printf("Memory released. Exiting program...\n");
}

// Reads commands from the user and calls the matching function until exit
int main()
{
    char inputLine[MAX_LINE_LENGTH];
    char command[MAX_LINE_LENGTH];
    char argument[MAX_LINE_LENGTH];
    char *firstQuote;
    char *lastQuote;
    int wordCount;
    int lineLength;

    initFileSystem();
    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    while (1)
    {
        printPath(cwd);
        printf(" > ");

        if (fgets(inputLine, MAX_LINE_LENGTH, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        lineLength = (int)strlen(inputLine);
        while (lineLength > 0 && (inputLine[lineLength - 1] == '\n' || inputLine[lineLength - 1] == '\r'))
        {
            inputLine[lineLength - 1] = '\0';
            lineLength--;
        }

        command[0] = '\0';
        argument[0] = '\0';
        wordCount = sscanf(inputLine, "%s %s", command, argument);

        if (wordCount < 1)
        {
            continue;
        }

        if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else if (strcmp(command, "mkdir") == 0)
        {
            if (wordCount < 2)
            {
                printf("Usage: mkdir <name>\n");
            }
            else
            {
                doMkdir(argument);
            }
        }
        else if (strcmp(command, "create") == 0)
        {
            if (wordCount < 2)
            {
                printf("Usage: create <name>\n");
            }
            else
            {
                doCreate(argument);
            }
        }
        else if (strcmp(command, "write") == 0)
        {
            firstQuote = strchr(inputLine, '"');
            lastQuote = strrchr(inputLine, '"');
            if (wordCount < 2 || firstQuote == NULL || lastQuote == firstQuote)
            {
                printf("Usage: write <name> \"text\"\n");
            }
            else
            {
                *lastQuote = '\0';
                doWrite(argument, firstQuote + 1);
            }
        }
        else if (strcmp(command, "read") == 0)
        {
            if (wordCount < 2)
            {
                printf("Usage: read <name>\n");
            }
            else
            {
                doRead(argument);
            }
        }
        else if (strcmp(command, "delete") == 0)
        {
            if (wordCount < 2)
            {
                printf("Usage: delete <name>\n");
            }
            else
            {
                doDelete(argument);
            }
        }
        else if (strcmp(command, "rmdir") == 0)
        {
            if (wordCount < 2)
            {
                printf("Usage: rmdir <name>\n");
            }
            else
            {
                doRmdir(argument);
            }
        }
        else if (strcmp(command, "ls") == 0)
        {
            doLs();
        }
        else if (strcmp(command, "cd") == 0)
        {
            if (wordCount < 2)
            {
                printf("Usage: cd <name | ..>\n");
            }
            else
            {
                doCd(argument);
            }
        }
        else if (strcmp(command, "pwd") == 0)
        {
            doPwd();
        }
        else if (strcmp(command, "df") == 0)
        {
            doDf();
        }
        else
        {
            printf("Unknown command: %s\n", command);
        }
    }

    cleanupFileSystem();
    return 0;
}