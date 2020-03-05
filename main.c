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
	void*ptr;
	unsigned int size;
} header_t;



//pointer on first free element:
static header_t* freep = NULL; // NULL if we have first call of malloc or pointer to first free block;

//flag memory like a free
void sw_free(void *fptr){
	header_t *bp, *ptr;
	bp = (header_t*)fptr - 1; //point to block header
	//search near:
	for(ptr= freep; (uint8_t*)bp<=(uint8_t*)ptr || (uint8_t*)bp>=(uint8_t*)ptr->ptr  ; ptr = ptr->ptr){
		if((uint8_t*)ptr>= (uint8_t*)ptr->ptr && ((uint8_t*)bp > (uint8_t*)ptr ||(uint8_t*) bp < (uint8_t*)ptr->ptr))
			break;
	}
	if((uint8_t*)bp + bp->size == (uint8_t*)ptr->ptr){
		header_t *t = ptr->ptr;
		bp->size += t->size;
		bp->ptr = t->ptr;
	}
	else{
		bp->ptr = ptr->ptr;
	}
	if((uint8_t*)ptr + ptr->size == (uint8_t*)bp){
		ptr->size += bp->size;
		ptr->ptr = bp->ptr;
	}
	else{
		ptr->ptr = (void*)bp;
	}
	freep = ptr;
}

//function for take memory block from OS
//using unix syscall sbrk
static header_t* allocate(const size_t size){
	//printf("allocating request: [%lu]\n", size );
	char *cp; //pointer to heap
	header_t *up; //header of new block
	unsigned size_ = size;
	if(size_ < HEAP_SIZE)
		size_ = HEAP_SIZE; //current size
	cp = sbrk(size_); //allocate memory 
	if(cp == (char*)-1) //if OS can't give memory -- return NULL
		return NULL;
	//create block:
	up = (header_t*) cp; //create header ref to new heap block
	up->size = size_;
	//set new memory free:
	sw_free((void*)(up+1));
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
				p = (header_t*)((uint8_t*)ptr + current_size);
				p->size = (unsigned)size_;
				p->ptr = ptr->ptr;
				ptr->size = current_size;
				freep = ptr;
				return (void*)(p+1);
				/*ptr->size-=size_;
				ptr= (uint8_t*)ptr + ptr->size;
				ptr->size = size_;
				freep = prev_ptr;
				return(void*)(ptr+1);*/
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
void*  sw_calloc(const size_t size){
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
				uint8_t*p_ = (uint8_t*)(ptr+1);
				for(size_t i =0;i< size;++i,p_++)
					*p_=0;
				return (void*)(ptr+1);
			}
			else{
				//create new block in right part of big block:
				header_t *p;
				size_t current_size = ptr->size - size_;
				p = (uint8_t*)ptr + current_size;
				p->size = (unsigned)size_;
				p->ptr = ptr->ptr;
				ptr->size = current_size;
				freep = ptr;
				uint8_t*p_ = (uint8_t*)(p+1);
				for(size_t i=0;i<size;++i,p_++)
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
	loger();
	int *a = sw_malloc(sizeof(int));
	printf("malloc: [%u]\n", *a);
	loger();
	int *b = sw_calloc(sizeof(int));
	loger();
	printf("calloc: [%u]\n", *b);
}


int main(){
	char *a = sw_malloc(100);
	loger();
	char *b = sw_malloc(60);
	loger();
	sw_free(b);
	loger();
	char*e = sw_calloc(20);
	printf("%p\n",e);
	sw_free(e);
	e = "hello my friend\0";
	printf("%p\n",e);
	//strcpy(e,"hello my friend");
	printf("%s\n",e);
	loger();
	calloc_test();
	/*int *n = sw_malloc(4);
	printf("%u\n",n);
	int *n2*/
}


