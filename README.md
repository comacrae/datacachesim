# datacachesim
Data Cache Simulator for a computer hardware course

This program simulates a datacache using a LRU (least recently used) replacement algorithm and a write-back and write-allocate policy.

ASSUMPTIONS:

1)   Number of sets <= 8192

2)   Associativity Level <= 8

3)   Number of sets and line size = 2^x for some integer btw 0 and 13

4)   Line size >= 8 bytes

5)   Format and numbers specified in format and data input are valid

COMPILATION/ CONFIGURATION STEPS:

STEP 1: Alter the trace.config file to configure the cache settings using the following format:
  
  Number of sets: X
  
  Set size: X
 
 Line size: X
  
STEP 2: Compile datacache.cpp using version c++11 or greater

Example in Unix: g++ -std=c++11 datacache.cpp -o datacache 
  
STEP 3:Run executable and enter in reads/writes in following format: 
  access type (read or write):reference size in bytes:
  <R|W>:<REFERENCE_SIZE>:<HEX_ADDRESS>

  
  OPTIONAL:
  
  I/O redirect an inputfile in the format:
  
 <R|W>:<REFERENCE_SIZE>:<HEX_ADDRESS>
 <R|W>:<REFERENCE_SIZE>:<HEX_ADDRESS>
 <R|W>:<REFERENCE_SIZE>:<HEX_ADDRESS>
  

    
  refer to trace.dat for an example file 

Once executable is exited, stats will be printed a file created in the same directory labled "trace.stats"

Note: An example executable using the given trace.config file as well as an example trace.stats file demonstrating results with given trace.stats file


