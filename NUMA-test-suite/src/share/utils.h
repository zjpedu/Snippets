#define align_size_up_(size, alignment) (((size) + ((alignment) - 1)) & ~((alignment) - 1))

inline bool is_size_aligned(size_t size, size_t alignment) {
    return align_size_up_(size, alignment) == size;                                                                                                                                       }

inline bool is_ptr_aligned(void* ptr, size_t alignment) {
    return align_size_up_((intptr_t)ptr, (intptr_t)alignment) == (intptr_t)ptr;
}

inline intptr_t align_size_up(intptr_t size, intptr_t alignment) {
    return align_size_up_(size, alignment);
}

#define align_size_down_(size, alignment) ((size) & ~((alignment) - 1))

inline intptr_t align_size_down(intptr_t size, intptr_t alignment) {
    return align_size_down_(size, alignment);
}

#define is_size_aligned_(size, alignment) ((size) == (align_size_up_(size, alignment)))

inline void* align_ptr_up(void* ptr, size_t alignment) {
    return (void*)align_size_up((intptr_t)ptr, (intptr_t)alignment);
}

inline void* align_ptr_down(void* ptr, size_t alignment) {
    return (void*)align_size_down((intptr_t)ptr, (intptr_t)alignment);
}
