#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DEBUG
#define stack_max 30
#define TO32(x) (stack[x] << 16 | stack[x + 1])

#if DEBUG >= 4
    #define RAMDUMP() {printf("RAM:\n");int i;for(i = 0; i < sizeof(RAM) / sizeof(RAM[0]); i++){printf("0x%04X, ", RAM[i]);}printf("\n");if(ip < sizeof(RAM) / sizeof(RAM[0])){printf("\n%*c^\n", 8 * ip, ' ');}}
#else
    #define RAMDUMP() {}
    #if DEBUG >= 3
        #define STACKDUMP() {printf("STACK:\n");int i;for(i = sptr + 1; i < stack_max; i++) {stack[i] = 0;}for(i = 0; i < stack_max; i++) {printf("%04d, ", stack[i]);}if(sptr <= 0){if(sptr == 0){printf("\n^\n");}else{printf("\n\n");}}else{printf("\n%*c^\n", 6 * sptr, ' ');}}
        #define INDEXSTACKDUMP() {printf("STACK:\n");int i;for(i = sptr + 1; i < stack_max; i++) {stack[i] = 0;}for(i = 0; i < stack_max; i++) {if(sptr == i){printf("%d: %04d <\n", i, stack[i]);}else{printf("%d: %04d\n", i, stack[i]);}}}
    #else
        #define STACKDUMP() {}
        #define INDEXSTACKDUMP() {}
        #if DEBUG >= 2
            #define PRINTF(x...) {printf(x);}
        #else
            #define PRINTF(x...) {}
            #if DEBUG >= 1
                #define SLEEP(x) {usleep((int)((x) * 100000));}
            #else
                #define SLEEP(x) {}
            #endif
        #endif
    #endif
#endif

#define END 0
#define PUSH 1
#define PIP 2
#define POP 3
#define COPY 4
#define SET 5
#define JUMP 6
#define IF 7
#define EXT 8
#define ADD 9
#define SUB 10
#define MUL 11
#define AND 12
#define OR 13
#define XOR 14
#define LSH 15
#define RSH 16
#define NOT 17
#define MORE 18
#define LESS 19

typedef void funcptr();

int sptr = -1;
uint16_t stack[stack_max];

uint16_t RAM[] = {
    PUSH, 0x1234,
    PUSH, 0x0000, // 1
    PUSH, 0x000A, // 3
    PUSH, 0x0021, // 5
    PUSH, 0x0064, // 7
    PUSH, 0x006C, // 9
    PUSH, 0x0072, // 11
    PUSH, 0x006F, // 13
    PUSH, 0x0057, // 15
    PUSH, 0x0020, // 17
    PUSH, 0x002C, // 19
    PUSH, 0x006F, // 21
    PUSH, 0x006C, // 23
    PUSH, 0x006C, // 25
    PUSH, 0x0065, // 27
    PUSH, 0x0048, // 29
    PUSH, 0x0001, // if boolean
    PUSH, 0x0000, // false if jump #1
    PUSH, 52, // false if jump #2
    IF,
    EXT,
    PUSH, 0x0001, COPY, // copy char to if boolean, so if(char 0) stops printing
    PUSH, 0x0000, // false if jump #1
    PUSH, 52, // false if jump #2
    PUSH, 0, // jump to if
    PUSH, 38, // jump to if
    JUMP,
    END
};

uint32_t ip = 0x00000000;

void end() {
    PRINTF("EXIT\n");
    exit(0);
}

void push() {
    PRINTF("Pushing %d\n", RAM[ip + 1]);
    stack[++sptr] = RAM[ip + 1];
    ip += 2;
}

void pip() {
    stack[++sptr] = ip;
    ip++;
}

void pop() {
    sptr--;
    ip++;
}

void copy() {
    stack[sptr] = stack[sptr - stack[sptr]];
    ip++;
}

void set() {
    RAM[TO32(sptr - 2)] = stack[sptr--];
    sptr -= 2;
    ip += 3;
}

void jump() {
    PRINTF("ip -> %d\n", TO32(sptr - 1));
    ip = TO32(sptr - 1);
    sptr -= 2;
}

// stack.get() => stack[sptr]
// stack.pop() => stack[sptr--]
// stack.push(value) => stack[++sptr] = value
// stack.offset(value) => stack[sptr - value]

void eif() {
    PRINTF("IF(stack[%d])\n  ip -> %d\nELSE\n  ip -> %d\n", sptr - 2, ip + 1, TO32(sptr - 1));
    if(stack[sptr - 2]) {
        PRINTF("ip -> %d\n", ip + 1);
        ip++;
    } else {
        PRINTF("ip -> %d\n", TO32(sptr - 1));
        ip = TO32(sptr - 1);
    }
    sptr -= 3;
}

void ext() { // TODO
    PRINTF("print %d\n", stack[sptr]);
    putchar((char)(stack[sptr--] & 0x7F));
    ip += 1;
}

void add() {
    PRINTF("stack[%d] += stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] += stack[sptr--];
    ip += 1;
}

void sub() {
    PRINTF("stack[%d] -= stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] -= stack[sptr--];
    ip += 1;
}

void mul() {
    PRINTF("stack[%d] *= stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] *= stack[sptr--];
    ip += 1;
}

void and() {
    PRINTF("stack[%d] &= stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] &= stack[sptr--];
    ip += 1;
}

void or() {
    PRINTF("stack[%d] |= stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] |= stack[sptr--];
    ip += 1;
}

void xor() {
    PRINTF("stack[%d] ^= stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] ^= stack[sptr--];
    ip += 1;
}

void lsh() {
    PRINTF("stack[%d] <<= stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] <<= stack[sptr--];
    ip += 1;
}

void rsh() {
    PRINTF("stack[%d] >>= stack[%d];\n", sptr - 1, sptr);
    stack[sptr - 1] >>= stack[sptr--];
    ip += 1;
}

void not() {
    PRINTF("~stack[%d];\n", sptr);
    stack[sptr] = ~stack[sptr];
    ip += 1;
}

void more() {
    PRINTF("stack[%d] > stack[%d]?\n", sptr - 1, sptr);
    stack[sptr - 1] = stack[sptr - 1] > stack[sptr];
    sptr--;
    ip += 1;
}

void less() {
    PRINTF("stack[%d] < stack[%d]?\n", sptr - 1, sptr);
    stack[sptr - 1] = stack[sptr - 1] < stack[sptr];
    sptr--;
    ip += 1;
}

funcptr *funcs[] = {
    &end,
    &push,
    &pip,
    &pop,
    &copy,
    &set,
    &jump,
    &eif,
    &ext,
    &add,
    &sub,
    &mul,
    &and,
    &or,
    &xor,
    &lsh,
    &rsh,
    &not,
    &more,
    &less
};

void main() {
    loop:
        RAMDUMP();
        PRINTF("Processing \"%d\"\n", RAM[ip]);
        (*funcs[RAM[ip]])();
        STACKDUMP();
        SLEEP(1.5);
    goto loop;
}
