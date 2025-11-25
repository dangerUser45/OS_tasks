# chat.c:
# Compilation
clang -c -I../lib/include src/chat.c -o build/chat.o  -Wall -Wextra #-g -fsanitize=address
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra #-g -fsanitize=address
# Linking
clang build/chat.o ../lib/build/safe_lib.o -o build/chat.out -Wall -Wextra #-g -fsanitize=address
