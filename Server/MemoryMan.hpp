#ifndef _MEMORY_MANAGE_
#define _MEMORY_MANAGE_
#include<stdlib.h>
#include<iostream>
#include<assert.h>
#include<mutex>

static const int Max_Memory_Size = 30000;

class MemoryAlloc;
class MemoryBlock{
public:
	int nID;
	int nRef;
	MemoryAlloc* pAlloc;
	MemoryBlock* pNext;
	bool InPool;
};

//the memory pool
class MemoryAlloc 
{
protected:
	char* pBuf;  //the memory pool with the sise of nSize*nBlock
	MemoryBlock* pHeader; //the current avilable memory
	size_t nSize; //the size of every piece of memory
	size_t nBlock; //the number of memory
	std::mutex Mutex;
public:
	MemoryAlloc(size_t nsize=0,size_t nblock=0) {
		const size_t n = sizeof(void*);
		nsize = (nsize / n) * n + (nsize % n ? n : 0);
		pBuf = nullptr;
		pHeader = nullptr;
		nSize = nsize;
		nBlock = nblock;
	}
	~MemoryAlloc() {
		if (pBuf) {
			free(pBuf);
		}
	}
	void* Alloc(size_t nsize) {
		std::lock_guard<std::mutex> lg(Mutex);   //use mutex when allocing
		if (!pBuf) {
			InitMemory();
		}
		MemoryBlock* pRes=nullptr;
		if (pHeader == nullptr) {//the memory pool has run up
			pRes = (MemoryBlock*)malloc(nsize + sizeof(MemoryBlock));
			pRes->InPool = false;
			pRes->nID = -1;
			pRes->nRef = 1;
			pRes->pAlloc = nullptr;
			pRes->pNext = nullptr;
		}
		else {
			pRes = pHeader;  //pHeader is the current block which can be used
			pHeader = pHeader->pNext;
			assert(0 == pRes->nRef);
			pRes->nRef = 1;
		}
		//std::cout << nsize << "__" << pRes->nID << "__" << pRes << std::endl;
		return ((char*)pRes+sizeof(MemoryBlock));
	}

	void Free(void* p) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)p - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (pBlock->InPool) {
			std::lock_guard<std::mutex> lg(Mutex); //use mutex if in pool
			if (--pBlock->nRef != 0) return;
			pBlock->pNext = pHeader;
			pHeader = pBlock;
		}
		else {
			if (--pBlock->nRef != 0) return;
			free(pBlock);
		}
	}
	void InitMemory() {
		//std::cout << "INit:"<<nSize<<std::endl;
		assert(pBuf == nullptr);
		if (pBuf)
			return;
		pBuf =(char*) malloc((nSize+sizeof(MemoryBlock)) * nBlock);
		pHeader = (MemoryBlock*)pBuf;//the head of every piece of memory is a block which recordes the information of the memory;
		pHeader->InPool = true;
		pHeader->nID = 0;
		pHeader->nRef = 0;
		pHeader->pAlloc = this;
		pHeader->pNext = nullptr;
		MemoryBlock* pTemp=pHeader;
		for (int n = 1; n < nBlock; n++) {
			pTemp->pNext = (MemoryBlock*)(pBuf +n* (nSize + sizeof(MemoryBlock)));
			pTemp = pTemp->pNext;
			pTemp->InPool = true;
			pTemp->nID = n;
			pTemp->nRef = 0;
			pTemp->pAlloc = this;
			pTemp->pNext = nullptr;
		}
	}
	
};

class MemoryMan
{
	MemoryAlloc mem64{ 64,1000000 };
	MemoryAlloc mem128{ 128,1000000 };
	//MemoryAlloc mem256{ 256,10000 };
	//MemoryAlloc mem512{ 512,10000 };
	MemoryAlloc mem30000{ 30000,10000 };

	MemoryAlloc* szAlloc[Max_Memory_Size + 1];
private:
	MemoryMan() {
		Init();
	}
	~MemoryMan() {

	}

public:
	static MemoryMan& Instance() {
		static MemoryMan man;
		return man;
	}
	void* Alloc(size_t nsize) {
		if (nsize <= Max_Memory_Size + 1) {
			return (MemoryBlock*)szAlloc[nsize]->Alloc(nsize);
		}
		else {
			MemoryBlock* pRes = (MemoryBlock*)malloc(nsize + sizeof(MemoryBlock));
			pRes->InPool = false;
			pRes->nID = -1;
			pRes->nRef = 1;
			pRes->pAlloc = nullptr;
			pRes->pNext = nullptr;
			//std::cout <<nsize<<"__"<< pRes->nID << "__" << pRes <<std::endl;
			return ((char*)pRes + sizeof(MemoryBlock));
		}
	}

	void Free(void* p) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)p - sizeof(MemoryBlock));
		//std::cout <<"Free:" << pBlock->nID << "__" <<pBlock<< std::endl;
		if (pBlock->InPool) {
			pBlock->pAlloc->Free(p);
		}
		else {
			if(--pBlock->nRef==0)
			    free(pBlock);
		}
	}
	void Init() {
		for (int i =0; i <=64; i++) {
			szAlloc[i] = &mem64;
		}
		for (int i = 65; i <= 128; i++) {
			szAlloc[i] = &mem128;
		}
		for (int i = 129; i <= 30000; i++) {
			szAlloc[i] = &mem30000;
		}
		
	}
	void addRef(void* p) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)p - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
};

#endif
