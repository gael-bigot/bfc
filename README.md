# bfc
Tiny brainf*ck compiler to Linux x86_64 assembly.

Build with your favorite compiler :
```
$ gcc -o bfc bfc.c
```

Compile any brainf*ck source code into Linux x86_64 assembly :
```
$ ./bfc < hello.bf > hello.asm
```

Assemble, link and enjoy!
```
$ nasm -f elf64 hello.asm
$ ld -m elf_x86_64 -o hello hello.o
$ ./hello
Hello, World!
```