#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


/* Simple stack to store the state of openned/closed loops */

struct stack{
    int val;
    struct stack* next;
};

struct stack* push(struct stack* stack, int val){
    struct stack* new = malloc(sizeof(struct stack));
    new->val = val;
    new->next = stack;
    return new;
}

struct stack* pop(struct stack* stack, int* dest){
    *dest = stack->val;
    struct stack* temp = stack->next;
    free(stack);
    return temp;
}


int main(int argc, char* argv[]){
    FILE* input_file = stdin;
    FILE* output_file = stdout;

    // Usual asm stuff
    fprintf(output_file, "section .text\nglobal _start\n_start:\n");

    /* Setting rax as memory pointer. 64k bytes of memory is alocated in .bss section */
    fprintf(output_file, "\tmov rax, mem\n\tadd rax, 0x7FFF\n");


    bool do_incr;
    bool do_shift;
    int incr_acc = 0; // stores the sum of a succession of + and - (for optimisation)
    int shift_acc = 0; // same with < and >

    bool do_write = false;
    bool do_read = false;

    bool do_new_loop = false;
    bool do_close_loop = false;

    int loop_index = 0;
    struct stack* loop_stack = NULL;
    int curr_loop;

    char c;
    while (c != EOF){
        c = fgetc(input_file);
        switch (c) {
            case '>':
                do_incr = true;
                do_shift = false;
                shift_acc++;
                break;
            case '<':
                do_incr = true;
                do_shift = false;
                shift_acc--;
                break;
            case '+':
                do_shift = true;
                do_incr = false;
                incr_acc++;
                break;
            case '-':
                do_shift = true;
                do_incr = false;
                incr_acc--;
                break;
            case '.':
                do_shift = true;
                do_incr = true;
                do_write = true;
                break;
            case ',':
                do_shift = true;
                do_incr = true;
                do_read = true;
                break;
            case '[':
                do_shift = true;
                do_incr = true;
                do_new_loop = true;
                break;
            case ']':
                do_shift = true;
                do_incr = true;
                do_close_loop = true;
                break;
        }
        if (do_incr && incr_acc){
            // This if statement is purely cosmetic as ADD works with negative values
            if (incr_acc > 0){
                fprintf(output_file, "\tadd BYTE [rax], %d\n", incr_acc);
            } else {
                fprintf(output_file, "\tsub BYTE [rax], %d\n", -incr_acc);
            }
            incr_acc = 0;
        }
        if (do_shift && shift_acc){
            if (shift_acc > 0){
                fprintf(output_file, "\tsub rax, %d\n", shift_acc);
            } else {
                fprintf(output_file, "\tadd rax, %d\n", -shift_acc);
            }
            shift_acc = 0;
        }
        if (do_write){
            // write syscall
            fprintf(output_file, "\tpush rax\n\tmov rdi, 1\n\tmov rsi, rax\n\tmov rdx, 1\n\tmov rax, 1\n\tsyscall\n\tpop rax\n");
            do_write =false;
        }
        if (do_read){
            // read syscall
            fprintf(output_file, "\tpush rax\n\tmov rdi, 0\n\tmov rsi, rax\n\tmov rdx, 1\n\tmov rax, 0\n\tsyscall\n\tpop rax\n");
            do_read = false;
        }
        if (do_new_loop){
            loop_stack = push(loop_stack, loop_index);
            fprintf(output_file, "\tcmp BYTE [rax], 0\n\tje loop%d_end\n\tloop%d:\n", loop_index, loop_index);
            loop_index++;
            do_new_loop = false;
        }
        if (do_close_loop){
            loop_stack = pop(loop_stack, &curr_loop);
            fprintf(output_file, "\tcmp BYTE [rax], 0\n\tjne loop%d\n\tloop%d_end:\n", curr_loop, curr_loop);
            do_close_loop = false;
        }

    }
    // Exit syscall and .bss section
    fprintf(output_file, "\tmov rdi, 0\n\tmov rax, 60\n\tsyscall\nsection .bss\nmem: resb 0xFFFF");
    
    return 0;
}