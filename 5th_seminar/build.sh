clang -c -I../lib/include src/relay_race.c -o build/relay_race.o -Wall -Wextra 
clang -c -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o -Wall -Wextra
clang build/relay_race.o ../lib/build/safe_lib.o -o build/relay_race.out
