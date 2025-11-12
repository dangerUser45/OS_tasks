# handler.c:
# Compilation
clang -c -I../lib/include  src/handler.c -o  build/handler.o -Wall -Wextra 
clang -c -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/handler.o ../lib/build/safe_lib.o -o build/handler.out

# signal_cat.c:
# Compilation
clang -c -I../lib/include  src/signal_cat.c -o  build/signal_cat.o -Wall -Wextra 
clang -c -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/signal_cat.o ../lib/build/safe_lib.o -o build/signal_cat.out
