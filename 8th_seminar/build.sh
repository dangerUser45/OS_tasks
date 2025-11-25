# pthread_pizza:
# Compilation
clang -c -I../lib/include  src/pthread_pizza.c -o  build/pthread_pizza.o -Wall -Wextra -g -O0
clang -c -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o -Wall -Wextra -g -O0
# Linking
clang build/pthread_pizza.o ../lib/build/safe_lib.o -o build/pthread_pizza.out -g -O0