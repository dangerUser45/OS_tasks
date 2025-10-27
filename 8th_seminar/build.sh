# pthread_cat:
# Compilation
clang -c -I../lib/include  src/pthread_cat.c -o  build/pthread_cat.o -Wall -Wextra 
clang -c -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/pthread_cat.o ../lib/build/safe_lib.o -o build/pthread_cat.out