# datacachesim
Data Cache Simulator for a computer hardware course


STEPS:

1) Alter the trace.config file to configure the cache settings using the following format:

Number of sets: X

Set size: X

Line size: X

2) Compile datacache.cpp using version c++11 or greater

3) Run executable and enter in reads/writes in following format:

<R|W>:<SET_SIZE>:<HEX ADDRESS>

refer to trace.dat for an example file 

Once executable is exited, stats will be printed a file created in the same directory labled "trace.stats"


