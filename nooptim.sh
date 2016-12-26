#!/bin/bash
# nooptim.sh - build gengo with no compiler optimisations.

gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c alarm.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c firstrun.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c fileops.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c stringops.c

gcc -g alarm.o firstrun.o fileops.o stringops.o -o alarm
rm *.o
