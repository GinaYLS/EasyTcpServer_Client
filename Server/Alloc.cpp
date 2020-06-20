#include"Alloc.h"
#include"MemoryMan.hpp"


void* operator new(size_t size) {	
	return MemoryMan::Instance().Alloc(size);
}

void operator delete(void* p) {
	MemoryMan::Instance().Free(p);
}
void* operator new[](size_t size) {
	return MemoryMan::Instance().Alloc(size);
}

void operator delete[](void* p) {
	MemoryMan::Instance().Free(p);
}
void* Malloc(size_t size) {
	return malloc(size);
}

void Free(void* p) {
	free(p);
}
