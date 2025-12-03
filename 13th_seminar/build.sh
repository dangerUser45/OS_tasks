# select_wc.c:
# Compilation
clang -c -I../lib/include src/select_wc.c -o build/select_wc.o  -Wall -Wextra #-g -fsanitize=address
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra #-g -fsanitize=address
# Linking
clang build/select_wc.o ../lib/build/safe_lib.o -o build/select_wc.out -Wall -Wextra #-g -fsanitize=address
