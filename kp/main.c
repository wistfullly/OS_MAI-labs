#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <cstdlib>
#include <ctime>

// ------------------- Список свободных блоков -------------------

typedef struct Block {
    size_t size;
    bool is_free;
    struct Block* next;
} Block;

typedef struct {
    Block* head;
    void* memory_pool;
} ListAllocator;

ListAllocator* createListAllocator(void* memory, size_t size) {
    ListAllocator* allocator = (ListAllocator*)malloc(sizeof(ListAllocator));
    allocator->memory_pool = memory;
    allocator->head = (Block*)memory;
    allocator->head->size = size - sizeof(Block);
    allocator->head->is_free = true;
    allocator->head->next = NULL;
    return allocator;
}

void* listAlloc(ListAllocator* allocator, size_t size) {
    Block* current = allocator->head;
    while (current) {
        if (current->is_free && current->size >= size) {
            if (current->size > size + sizeof(Block)) {
                // Разделение блока
                Block* new_block = (Block*)((char*)current + sizeof(Block) + size);
                new_block->size = current->size - size - sizeof(Block);
                new_block->is_free = true;
                new_block->next = current->next;
                current->next = new_block;
            }
            current->is_free = false;
            current->size = size;
            return (char*)current + sizeof(Block);
        }
        current = current->next;
    }
    return NULL; // Нет подходящего блока
}

void listFree(ListAllocator* allocator, void* ptr) {
    if (!ptr) return;
    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->is_free = true;

    // Объединение соседних свободных блоков
    Block* current = allocator->head;
    while (current) {
        if (current->is_free && current->next && current->next->is_free) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void destroyListAllocator(ListAllocator* allocator) {
    allocator->head = NULL; // Блоки в memory_pool автоматически освобождаются.
    free(allocator);
}



// ------------------- Алгоритм двойников -------------------

#define DIV_ROUNDUP(A, B) (((A) + (B)-1) / (B))
#define ALIGN_UP(A, B) (DIV_ROUNDUP((A), (B)) * (B))

typedef struct BuddyBlock {
    size_t blockSize;
    bool isFree;
} BuddyBlock;

typedef struct BuddyAllocator {
    BuddyBlock *head;
    BuddyBlock *tail;
    void *data;
    size_t totalSize;
    bool expanded;
    bool debug;
} BuddyAllocator;

BuddyBlock *next(BuddyBlock *block) {
    return (BuddyBlock *)((uint8_t *)block + block->blockSize);
}

BuddyBlock *split(BuddyBlock *block, size_t size) {
    // Пока размер блока больше нужного, делим его пополам
    while (block->blockSize > size) {
        size_t newSize = block->blockSize >> 1;
        block->blockSize = newSize;
        BuddyBlock *buddy = next(block);
        buddy->blockSize = newSize;
        buddy->isFree = true;
    }
    block->isFree = false;
    return block;
}

BuddyBlock *findBest(BuddyAllocator *allocator, size_t size) {
    BuddyBlock *block = allocator->head;
    BuddyBlock *bestBlock = NULL;

    // Ищем первый блок, который подходит по размеру
    while (block < allocator->tail) {
        if (block->isFree && block->blockSize >= size &&
            (bestBlock == NULL || block->blockSize < bestBlock->blockSize)) {
            bestBlock = block;
        }
        block = next(block);
    }

    // Если нашли подходящий блок, но его размер больше требуемого, разделяем его
    if (bestBlock) {
        // Если блок слишком велик, разделим его до нужного размера
        if (bestBlock->blockSize > size) {
            return split(bestBlock, size);
        } else {
            // Если блок идеально подходит, выделяем его
            bestBlock->isFree = false;
            return bestBlock;
        }
    }
    return NULL;
}

size_t requiredSize(size_t size) {
    size += sizeof(BuddyBlock);
    size = ALIGN_UP(size, sizeof(BuddyBlock));
    size_t actualSize = sizeof(BuddyBlock);
    while (actualSize < size) {
        actualSize <<= 1;
    }
    return actualSize;
}

void coalesce(BuddyAllocator *allocator) {
    BuddyBlock *block = allocator->head;

    // Пробегаем все блоки и сливаем соседние свободные блоки
    while (block < allocator->tail) {
        BuddyBlock *buddy = next(block);
        if (buddy >= allocator->tail) break;

        if (block->isFree && buddy->isFree && block->blockSize == buddy->blockSize) {
            block->blockSize <<= 1;
            continue;
        }
        block = next(block);
    }
}

void expand(BuddyAllocator *allocator, size_t size) {
    size_t currentSize = allocator->head ? allocator->head->blockSize : 0;
    size_t requiredSize = size + currentSize;

    // Округляем размер до ближайшей степени двойки
    size_t newSize = 1;
    while (newSize < requiredSize) {
        newSize <<= 1; // Сдвиг влево, чтобы удвоить размер
    }


    void *newData = realloc(allocator->data, newSize);
    if (!newData) {
        fprintf(stderr, "Failed to expand memory.\n");
        exit(EXIT_FAILURE);
    }

    // Обновляем указатели на новую память
    allocator->data = newData;
    allocator->head = (BuddyBlock *)allocator->data;
    allocator->tail = (BuddyBlock *)((uint8_t *)allocator->data + newSize);

    // Настраиваем блок
    allocator->head->blockSize = newSize;
    allocator->head->isFree = true;

    if (allocator->debug) {
        printf("Expanded heap to %zu bytes\n", newSize);
    }
}


BuddyAllocator *createBuddyAllocator(size_t size, bool debug) {
    BuddyAllocator *allocator = (BuddyAllocator *)malloc(sizeof(BuddyAllocator));
    allocator->data = NULL;
    allocator->head = NULL;
    allocator->tail = NULL;
    allocator->totalSize = 0;
    allocator->expanded = false;
    allocator->debug = debug;

    expand(allocator, size);
    return allocator;
}

void destroyBuddyAllocator(BuddyAllocator *allocator) {
    if (allocator->data) {
        free(allocator->data);
    }
    free(allocator);
}

void *buddyMalloc(BuddyAllocator *allocator, size_t size) {
    if (size == 0) return NULL;

    size_t actualSize = requiredSize(size);
    BuddyBlock *block = findBest(allocator, actualSize);

    if (!block) {
        if (allocator->expanded) {
            return NULL;
        }
        allocator->expanded = true;
        expand(allocator, actualSize);
        return buddyMalloc(allocator, size);
    }

    allocator->expanded = false;
    return (void *)((uint8_t *)block + sizeof(BuddyBlock));
}

void buddyFree(BuddyAllocator *allocator, void *ptr) {
    if (!ptr) return;

    BuddyBlock *block = (BuddyBlock *)((uint8_t *)ptr - sizeof(BuddyBlock));
    if ((uint8_t *)block < (uint8_t *)allocator->data || 
        (uint8_t *)block >= (uint8_t *)allocator->tail) {
        //fprintf(stderr, "Invalid pointer passed to buddyFree.\n");
        return;
    }

    block->isFree = true;

    if (allocator->debug) {
        printf("Freed %zu bytes\n", block->blockSize - sizeof(BuddyBlock));
    }

    coalesce(allocator);
}

// ------------------- Тестирование -------------------
#define MEMORY_POOL_SIZE 2048*2048*10

void testListAllocator() {
    void* memory_pool = malloc(MEMORY_POOL_SIZE);
    ListAllocator* list_allocator = createListAllocator(memory_pool, MEMORY_POOL_SIZE);

    const int NUM_ALLOCS = 100000;
    void* blocks[NUM_ALLOCS];

    // Измеряем время выделения памяти
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ALLOCS/2; ++i) {
        blocks[i] = listAlloc(list_allocator, 25);
        if (blocks[i] == NULL) {
            std::cerr << "Failed to allocate block #" << i << std::endl;
        }
    }
    for (int i = NUM_ALLOCS/2; i < NUM_ALLOCS; ++i) {
        blocks[i] = listAlloc(list_allocator, 367);
        if (blocks[i] == NULL) {
            std::cerr << "Failed to allocate block #" << i << std::endl;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> alloc_duration = end - start;
    std::cout << "ListAllocator: Time for allocations: " << alloc_duration.count() << " seconds" << std::endl;

    // Измеряем время освобождения памяти
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ALLOCS; ++i) {
        listFree(list_allocator, blocks[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> free_duration = end - start;
    std::cout << "ListAllocator: Time for frees: " << free_duration.count() << " seconds" << std::endl;

    destroyListAllocator(list_allocator);
    free(memory_pool);
}

void testBuddyAllocator() {
    BuddyAllocator *allocator = createBuddyAllocator(MEMORY_POOL_SIZE, false);

    const int NUM_ALLOCS = 100000;
    void* blocks[NUM_ALLOCS];

    // Измеряем время выделения памяти
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ALLOCS/2; ++i) {
        blocks[i] = buddyMalloc(allocator, 25);
        if (blocks[i] == NULL) {
            std::cerr << "Failed to allocate block #" << i << std::endl;
        }
    }
    for (int i = NUM_ALLOCS/2; i < NUM_ALLOCS; ++i) {
        blocks[i] = buddyMalloc(allocator, 367);
        if (blocks[i] == NULL) {
            std::cerr << "Failed to allocate block #" << i << std::endl;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> alloc_duration = end - start;
    std::cout << "BuddyAllocator: Time for allocations: " << alloc_duration.count() << " seconds" << std::endl;

    // Измеряем время освобождения памяти
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ALLOCS; ++i) {
        buddyFree(allocator, blocks[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> free_duration = end - start;
    std::cout << "BuddyAllocator: Time for frees: " << free_duration.count() << " seconds" << std::endl;

    destroyBuddyAllocator(allocator);
}

int main() {
    std::cout << "Starting performance tests...\n";

    // Тестирование ListAllocator
    testListAllocator();

    // Тестирование BuddyAllocator
    testBuddyAllocator();

    std::cout << "Performance tests completed." << std::endl;

    return 0;
}