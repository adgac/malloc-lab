/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "awesome_steminists_power_rangers_the_justice_league",
    /* First member's full name */
    "Khaled AlHosani",
    /* First member's email address */
    "kah579@nyu.edu",
    /* Second member's full name (leave blank if none) */
    "Yana Chala",
    /* Second member's email address (leave blank if none) */
    "ydc223@nyu.edu"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4 /* Word and header/footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x,y) ((x)>(y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size)|(alloc))

/* Read and write word at adress p*/
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from adress p*/

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute adress of its header and footer p*/
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute adress of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - WSIZE)))

static *heap_listp; 
static void *extend_heap(size_t words);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1)
        return -1;
    PUT(heap_listp, 0); /* Alignment padding*/
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Alignment padding*/ 
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Alignment padding*/
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); /* Alignment padding*/
    heap_listp += (2*WSIZE);

    /* Extend the empty heao with a free block of CHUNKSIZE bytes*/
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}


static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocates an even number of words to maintain alignment*/
    size = (words % 2) ? (words+1) * WSIZE: words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size,0)); /* Free block header*/
    PUT(FTRP(bp), PACK(size,0)); /* Free block footer*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0 , 1)); /* New epilogue footer */

    /* Coalesce if the prev block was free */
    return coalesce(bp);
}


/* 
 * mm_malloc 
 */
void *mm_malloc(size_t size)
{
	size_t asize;		//Adjusted block size
	size_t extendsize;	//Amount to extend hea if no fit
	char *bp;

	if (size==0)
		return NULL;

	//Adjust block size to include overhead and alignment reqs
	if (size<=DSIZE)
		asize=2*DSIZE;

	else
		asize = DSIZE *((size + (DSIZE)+(DSIZE-1))/DSIZE);

	//search the free list for a fit
	if((bp=find_fit(asize))!=NULL)
	{
		place(bp,asize);
		return bp;
	}

	extendsize=MAX(asize,CHUNKSIZE);
	if((bp=extend_heap(extendsize/WSIZE)==NULL))
		return NULL;

	place(bp,asize);
	return bp;

 //Given commented out


 //    int newsize = ALIGN(size + SIZE_T_SIZE);
 //    void *p = mem_sbrk(newsize);
 //    if (p == (void *)-1)
	// return NULL;
 //    else {
 //        *(size_t *)p = size;
 //        return (void *)((char *)p + SIZE_T_SIZE);
 //    	}
}

/*
 * mm_free
 */
void mm_free(void *ptr)
{
	size_t =GET_SIZE(HDRP(bp));

	PUT(HDRP(bp) , PACK(size, 0));
	PUT(FTRP(bp) , PACK(size, 0));
	coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/*
 * coalesce
 */

static void *coalesce(void *bp)
{
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(FTRP(NEXT_BLKP(bp)));

	size_t size = GET_SIZE(HDRP(bp));

	if(prev_alloc && next_alloc)
	{
		return bp;
	}

	if(prev_alloc && !next_alloc)
	{
		size+=GET_SIZE
	}

	return bp;
}

/*
 * find_fit
 */
static void *find_fit(size_t asize){}

/*
 * place
 */
static void place(void *bp, size_t asize){}







