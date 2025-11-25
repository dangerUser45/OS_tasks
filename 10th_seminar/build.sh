# my_ls.c:
# Compilation
clang -c -I../lib/include src/my_ls.c -o build/my_ls.o -Wall -Wextra 
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/my_ls.o ../lib/build/safe_lib.o -o build/my_ls.out
