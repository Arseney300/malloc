/*
 * @project malloc
 * @file main.c first realization
 * @author Soundwave
 * @date Mar 3, 2020
 * @brief simple memory allocator realization
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include <unistd.h>
#define HEAP_SIZE 1024  //1024 bytes

typedef struct Header{
	uint8_t *ptr; 
	//uint32_t  size;
	unsigned int size;
	unsigned int x;
} header_t;



//pointer on first free element:
static header_t* freep = NULL; // NULL if we have first call of malloc or pointer to first free block;

//flag memory like a free
void free(void *fptr){
	header_t *bp, *ptr;
	bp = (header_t*)fptr - 1; //point to block header
	//search near:
	for(ptr= freep; bp<ptr || bp>ptr->ptr  ; ptr = ptr->ptr){
		if(ptr>= ptr->ptr && (bp > ptr || bp < ptr->ptr))
			break;
	}
	if(bp + bp->size == ptr->ptr){
		header_t *t = ptr->ptr;
		bp->size += t->size;
		bp->ptr = t->ptr;
	}
	else{
		bp->ptr = ptr->ptr;
	}
	if(ptr + ptr->size == bp){
		ptr->size += bp->size;
		ptr->ptr = bp->ptr;
	}
	else{
		ptr->ptr = bp;
	}
	freep = ptr;
}

//function for take memory block from OS
//using unix syscall sbrk
static header_t* allocate(const size_t size){
	printf("allocating request: [%u]\n", size );
	char *cp; //pointer to heap
	header_t *up; //header of new block
	size_t size_ = size;
	if(size_ < HEAP_SIZE)
		size_ = HEAP_SIZE; //current size
	cp = sbrk(size_); //allocate memory 
	if(cp == (char*)-1) //if OS can't give memory -- return NULL
		return NULL;
	//create block:
	up = (header_t*) cp; //create header ref to new heap block
	up->size = size_;
	//set new memory free:
	free((void*)(up+1));
	return freep;
}


void*  sw_malloc(const size_t size){
	header_t *ptr, *prev_ptr;
	header_t timed;
	//current size with header:
	size_t size_ = sizeof(header_t) + size;
	prev_ptr = freep;
	if(freep == NULL){
		//first call malloc
		//need header, that be reference on self, because we will have only one block of data
		timed.ptr = freep = prev_ptr = &timed;
		timed.size = 0;
	}
	//run in free blocks.
	for(ptr = prev_ptr->ptr;; prev_ptr = ptr, ptr = ptr->ptr){
		if(ptr->size >= size_){
		//big enough:
			if(ptr->size == size_){
				//exactly:
				//set prev_ptr to next block after ptr
				//and set free block to this block
				prev_ptr->ptr = ptr->ptr;
				freep = prev_ptr;
				//return pointer to block:
				return (void*)(ptr+1);
			}
			else{
				//create new block in right part of big block:
				header_t *p;
				size_t current_size = ptr->size - size_;
				p = ptr + current_size;
				printf("p-size: [%u]; size_ : [%u] \n",p->size, size_);
				p->size = size_;
				p->ptr = ptr->ptr;
				ptr->size = current_size;
				freep = ptr;
				return (void*)(p+1);
			}
		}
		if(ptr == freep){
			ptr = allocate(size_);
			if(ptr == NULL){
				return NULL; //memory out
			}
		}
	}
}

void* sw_calloc(const size_t size){
	header_t *ptr, *prev_ptr;
	header_t timed;
	//current size with header:
	size_t size_ = sizeof(header_t) + size;
	prev_ptr = freep;
	if(freep == NULL){
		//first call malloc
		//need header, that be reference on self, because we will have only one block of data
		timed.ptr = freep = prev_ptr = &timed;
		timed.size = 0;
	}
	//run in free blocks.
	for(ptr = prev_ptr->ptr;; prev_ptr = ptr, ptr = ptr->ptr){
		if(ptr->size >= size_){
		//big enough:
			if(ptr->size == size_){
				//exactly:
				//set prev_ptr to next block after ptr
				//and set free block to this block
				prev_ptr->ptr = ptr->ptr;
				freep = prev_ptr;
				//return pointer to block:
				//init zeros:
				char*p_ = (char*)(ptr+1);
				for(size_t i=0;i<size;++i, p_++)
					*p_ = 0;
				
				return (void*)(ptr+1);
			}
			else{
				//create new block in right part of big block:
				header_t *p;
				size_t current_size = ptr->size - size_;
				p = ptr + current_size;
				p->size = size_;
				p->ptr = ptr->ptr;
				ptr->size = current_size;
				freep = ptr;
				//init zeros:
				char*p_ = (char*)(ptr+1);
				for(size_t i =0;i< size;++i, p_++)
					*p_ = 0;
				return (void*)(p+1);
			}
		}
		if(ptr == freep){
			ptr = allocate(size_);
			if(ptr == NULL){
				return NULL; //memory out
			}
		}
	}

}

void loger(){printf("%u\n",freep->size);}

void calloc_test(){
	int *a = sw_malloc(sizeof(int));
	printf("malloc: [%u]\n", *a);
	int *b = sw_calloc(sizeof(int));
	printf("calloc: [%u]", *b);
}

int main(int argc, char**argv){
	char *a = sw_malloc(100);
	loger();
	char *b = sw_malloc(60);
	loger();
	free(b);
	loger();
	calloc_test();
	return 0;
}


