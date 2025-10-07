# Compilation
clang -c -DDEBUG -I./include -I../lib/include    src/shell_parser.c -o    build/shell_parser.o  -Wall -Wextra
clang -c -DDEBUG -I./include -I../lib/include        src/my_shell.c -o        build/my_shell.o  -Wall -Wextra
clang -c -DDEBUG -I./include -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o  -Wall -Wextra

# Linking
clang build/my_shell.o build/shell_parser.o ../lib/build/safe_lib.o  -o build/my_shell
