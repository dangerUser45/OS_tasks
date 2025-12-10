# size_pipe.c:
# Compilation
clang -c -I../lib/include src/size_pipe.c -o build/size_pipe.o  -Wall -Wextra #-g -fsanitize=address
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra #-g -fsanitize=address
# Linking
clang build/size_pipe.o ../lib/build/safe_lib.o -o build/size_pipe.out -Wall -Wextra #-g -fsanitize=address
