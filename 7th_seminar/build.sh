# mmap_cp.c:
# Compilation
clang -c -I../lib/include src/mmap_cp.c -o build/mmap_cp.o -Wall -Wextra 
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/mmap_cp.o ../lib/build/safe_lib.o -o build/mmap_cp.out

# pizza.c:
# Compilation
clang -c -I../lib/include src/pizza.c -o build/pizza.o -Wall -Wextra -g -O0 -fsanitize=address
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra -g -O0 -fsanitize=address
# Linking
clang build/pizza.o ../lib/build/safe_lib.o -o build/pizza.out -fsanitize=address -g
