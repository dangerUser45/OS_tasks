# shower.c:
# Compilation
clang -c -I../lib/include src/shower.c          -o build/shower.o          -Wall -Wextra 
clang -c -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/shower.o ../lib/build/safe_lib.o -o build/shower.out

# shower_opt.c:
# Compilation
clang -c -I../lib/include src/shower_opt.c      -o build/shower_opt.o       -Wall -Wextra 
clang -c -I../lib/include ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/shower_opt.o ../lib/build/safe_lib.o -o build/shower_opt.out
