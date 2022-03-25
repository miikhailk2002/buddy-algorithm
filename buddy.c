#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

struct Block;

struct Stack {
    struct Block *ptr;
    struct Stack *next;
};

struct Block {
    int num;
    int start;
    int size;
    struct Stack *buddy;
    struct Block *next;
    struct Block *prev;
    bool free;
};

struct Stack *new_Stack(struct Block *block){
    struct Stack *stack = (struct Stack*) malloc(sizeof(struct Stack));
    if(!stack){
        printf("malloc failed!\n");
        exit(-1);
    }
    stack->next = NULL;
    stack->ptr = block;
    return stack;
}

struct Stack *push(struct Stack *stack, struct Block *block){
    struct Stack *erg = new_Stack(block);
    erg->next = stack;
    return erg;
}


struct Block *pop(struct Stack **stack){
    struct Block *res = (*stack)->ptr;
    struct Stack *to_free = (*stack);
    *stack = (*stack)->next;
    free(to_free);
    return res;
}

void printHelp(char* executable_name);
void printMenu();
bool allocate_mem(int num);
bool free_mem(int num);
void print_mem();

int block_size = 4;
int block_num = 32;
int current_num = 1;

typedef struct Block block; 

block *list;

block *new_block(){
    block *block = (struct Block *) malloc(sizeof(struct Block));
    if(!block){
        printf("malloc failed!\n");
        exit(-1);
    }
    block->buddy = NULL;
    block->num = 0;
    block->prev = NULL;
    block->next = NULL;
    block->size = 0;
    block->start = 0;
    block->free = true;
    return block;
}

bool isPowerOfTwo(int num){
    return ((int)pow((double) 2, log2(num))) == num;
}

void handleCommand(){
    char cmd[512];
    fgets(cmd, 512, stdin);
    char c = *cmd;
    int num;
    switch (c)
    {
    case 'm':
        if(*(cmd+1) != ' '){
        printMenu();
        return;
        }
        num = atoi(cmd+2);
        if(num < 1){
            printf("error\n");
            return;
        }
        int required_blocks = (int) ceil((double)num/((double)block_size * 1024.));
        if(allocate_mem(required_blocks)){
            printf("success\n");
        }else{
            printf("error\n");
        }
        break;
    case 'f': 
        if(*(cmd+1) != ' '){
        printMenu();
        return;
        }
        num = atoi(cmd+2);
        if(num < 1){
            printf("error\n");
            return;
        }
        if(free_mem(num)){
            printf("success\n");
        }else{
            printf("Could not free %d - number is invalid!\n", num);
        }
        break;
    case 'l':
        print_mem();
        break;
    case 'q':
        exit(0);
        break;
    default:
        printMenu();
        break;
    }
}

int main(int argc, char* argv[]) {
    // parse command line arguments
    if(argc != 1){
        if(argc == 3){
            if(strcmp(argv[1],"-b") == 0){
                block_size = atoi(argv[2]);
            }else if(strcmp(argv[1], "-n") == 0){
                block_num = atoi(argv[2]);
            }else{
                printHelp(argv[0]);
                return EXIT_FAILURE;
                //FAILURE
            }
        }else if(argc == 5){
            if(strcmp(argv[1],"-b") == 0){
                block_size = atoi(argv[2]);
            }else if(strcmp(argv[1], "-n") == 0){
                block_num = atoi(argv[2]);
            }else{
                printHelp(argv[0]);
                return EXIT_FAILURE;
                //FAILURE
            }
            if(strcmp(argv[3],"-b") == 0){
                block_size = atoi(argv[4]);
            }else if(strcmp(argv[3], "-n") == 0){
                block_num = atoi(argv[4]);
            }else{
                printHelp(argv[0]);
                return EXIT_FAILURE;
                //FAILURE
            }
        }else{
            printHelp(argv[0]);
                return EXIT_FAILURE;
                //FAILURE
        }
        if(!isPowerOfTwo(block_num) || !isPowerOfTwo(block_size) || block_num <= 0 || block_size <= 0){
            printHelp(argv[0]);
                return EXIT_FAILURE;
                //FAILURE
        }
    }
    list = new_block();
    list->start = 0;
    list->size = block_num;
    printMenu();
    while (1) {
        // read commands and execute them
        handleCommand();
    }
}

void free_stack (struct Stack *stack){
    while(stack != NULL){
        struct Stack *to_free = stack;
        stack = stack->next;
        free(to_free);
    }
}

void print_mem(){
    block *current = list;
    while(current != NULL){
        if(current->free){
            for(int i = 0; i<current->size; i++){
                printf("F");
            }
        }else{
           for(int i = 0; i<current->size; i++){
                printf("P");
            } 
        }
        current = current->next;
    }
    printf("\n");
}

block *get_smallest_fitting(int blocks){
    block *current = list;
    block *res = NULL;
    while(current != NULL){
        if(current->free && current->size >= blocks){
            if(res == NULL || current->size < res->size){
            res = current;
            }
        }
        current = current->next;
    }
    return res;
}

int getSmallestPossibleBlockSize(int blocks){
    return (int) pow(2., ceil(log2(blocks)));
} 

bool allocate_mem(int blocks){
    if(blocks > block_num){
        return false;
    }
    block *current = get_smallest_fitting(blocks);
    if(!current){
        return false;
    }
    int target_blocksize = getSmallestPossibleBlockSize(blocks);
    while(current->size > target_blocksize){
        block *new = new_block();
        current->buddy = push(current->buddy, new);
        struct Stack *cur = current->buddy->next;
        while(cur){
            new->buddy = push(new->buddy, cur->ptr);
            cur = cur->next;
        }
        new->buddy = push(new->buddy, current);
        current->free = true;
        new->free = true;
        new->next = current->next;
        current->next = new;
        new->prev = current;
        if(new->next){
            new->next->prev = new;
        }
        current->size = current->size/2;
        new->size = current->size;
        new->start = current->start + current->size;
    }
    current->free = false;
    current->num = current_num++;

    return true;
}

block *find_block_with_num(int num){
    block *current = list;
    while(current != NULL){
        if(current->num == num){
            return current;
        }
        current = current->next;
    }
    return NULL;
}


bool free_mem(int num){
    if(num == 0){
        return false;
    }
    block *current = find_block_with_num(num);
    if(!current){
        return false;
    }
    while(true){
        current->free = true;
        current->num = 0;
        if(current->buddy == NULL){
            return true;
        }

        if(current->buddy && current->buddy->ptr->free && current->buddy->ptr->size == current->size){
            block *buddy = pop(&current->buddy);
            //merge
            if(buddy == current->prev){
                //der buddy liegt links
                current->prev = current->prev->prev;
                if(current->prev){
                    current->prev->next = current;
                }else{
                    list = current;
                }
                current->start = buddy->start;
            }else{
                current->next = current->next->next;
                if(current->next){
                    current->next->prev = current;
                }
            }
            free_stack(buddy->buddy);
            free(buddy);
            current->size = current->size * 2;
        }else{
            return true;
        }
    }
    return true;
}
/* Tester expects these exact (formatted) strings
 * V   V   V   V   V   V   V   V   V   V    */
void printHelp(char* executable_name) {
    printf("Usage: %s [-b BLOCKSIZE] [-n NUM_BLOCKS]\n"
           "BLOCKSIZE: integer, interpreted as kiB (i.e. -b 4 => 4096 Bytes). Has to be power of 2! The default is 4.\n"
           "NUM_BLOCKS: integer, number of blocks to be used. Has to be power of 2! The default is 32.\n",
           executable_name);
}

void printMenu() {
    printf("Type your command, then [enter]\n"
           "Available commands:\n"
           "m SIZE\t\tallocate SIZE Bytes of memory. SIZE has to be > 0.\n"
           "f POINTER\tfree the memory referenced by POINTER. If it is invalid, an error is shown and nothing else happens.\n"
           "l\t\tprint current memory layout.\n"
           "q\t\tquit the program.\n");
}
/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
