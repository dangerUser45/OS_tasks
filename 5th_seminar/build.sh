# att_relay_race:
# Compilation
clang -c -I../lib/include  src/att_relay_race.c -o  build/att_relay_race.o -Wall -Wextra 
clang -c -I../lib/include ../lib/src/safe_lib.c -o ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/att_relay_race.o ../lib/build/safe_lib.o -o build/att_relay_race.out

# posix_relay_race:
# Compilation
clang -c -I../lib/include src/posix_relay_race.c -o build/posix_relay_race.o -Wall -Wextra 
clang -c -I../lib/include  ../lib/src/safe_lib.c -o  ../lib/build/safe_lib.o -Wall -Wextra
# Linking
clang build/posix_relay_race.o ../lib/build/safe_lib.o -o build/posix_relay_race.out
