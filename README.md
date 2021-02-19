# datacachesim
Data Cache Simulator for a computer hardware course


COMPILATION/ CONFIGURATION STEPS:

STEP 1: Alter the trace.config file to configure the cache settings using the following format:
  
  Number of sets: X
  
  Set size: X
 
 Line size: X
  
STEP 2: Compile datacache.cpp using version c++11 or greater

Example in Unix: g++ -std=c++11 datacache.cpp -o datacache 
  
STEP 3:Run executable and enter in reads/writes in following format: 
  <R|W>:<SET_SIZE>:<HEX ADDRESS>

  OPTIONAL:
  I/O redirect an inputfile in the format:
  <R|W>:<SET_SIZE>:<HEX ADDRESS>
  <R|W>:<SET_SIZE>:<HEX ADDRESS>
  <R|W>:<SET_SIZE>:<HEX ADDRESS>
    
  refer to trace.dat for an example file 

Once executable is exited, stats will be printed a file created in the same directory labled "trace.stats"

Note: An example executable using the given trace.config file as well as an example trace.stats file demonstrating results with given trace.stats file


