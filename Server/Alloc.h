#ifndef _ALLOC_
#define _ALLOC_

void* operator new(size_t size);
void operator delete(void* p);
void* operator new[](size_t size);
void operator delete[](void* p);
void* Malloc(size_t size);
void Free(void* p);
#endif // !_ALLOC_


