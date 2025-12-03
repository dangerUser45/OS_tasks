# bridge.c:
# Compilation
clang -c -I../lib/include src/bridge.c -o build/bridge.o  -Wall -Wextra #-g -fsanitize=address
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra #-g -fsanitize=address
# Linking
clang build/bridge.o ../lib/build/safe_lib.o -o build/bridge.out -Wall -Wextra #-g -fsanitize=address
