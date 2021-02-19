/******************************************************
*	Name: Colin MacRae
*	Class: CDA3100
*	Assignment 9 : Impelementing Data Cache Simulator 
*	Compile: "g++ -std=c++11 -o datacache datacache.cpp"
*	NOTE: ASSUMES trace.config AND trace.dat FILES 
*	EXIST IN SAME DIRECTORY AS EXECUTABLE AND THAT
*	trace.config IS IN THE PROPER FORMAT 
*******************************************************/
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string>
#include <fstream>
#include <bitset>
#include <array>

//Note: main function is at bottom of file 

using namespace std;


/* This function takes in the number of bytes in a block (determined by line size input from config file) 
	and returns the size of the offset in bits */
int determineOffsetSize(int numBlockBytes) 
{
	return static_cast<int>( log2(numBlockBytes));
}
/* This function takes in the number of sets in a cache (determined by number of sets input from config file) 
	and returns the size of the index in bits */
int determineIndexSize(int numSets) 
{
	return static_cast<int>( log2(numSets));
}
/* This function takes in the index size, the offset Size, (which have been assumed to have been previously calculated)
 and the linesize (which has been assumed to been given in the config file) and calculates and returns the tag size 
*/
int determineTagSize( int indexSize, int offsetSize, int lineSize)
{
	return lineSize - (indexSize + offsetSize);
}
/* This function takes in the hexAddress provided from stdin and the lineSize from the config file and converts and 
returns the string representing the 32 bit version of the hex address (assumes words are 32 bits in length)
*/
string hexToBinary(int hexAddress, int lineSize)
{
	const unsigned int bitSetSize = lineSize; 
	return bitset<32>(hexAddress).to_string().substr((32 - lineSize), lineSize);
}
/* This function takes in the access type variable from the .dat file and returns a string version ("read" or "write")
*/
string determineAccessType(char accessTypeVariable)
{
	string accessType;
	if(accessTypeVariable == 'R')
		accessType = "read";
	else 
		accessType = "write";
	return accessType;
}
/* This function extracts the tag as an interger from the bit-string version of the address 
* Assumes that the index size and offset size have been previously calculated 
*/
int getTag(string fullAddress, int offsetSize, int indexSize)
{
	string tagString = fullAddress.substr(0, fullAddress.size() - (offsetSize + indexSize)); // extract tag string
	return bitset<32>(tagString).to_ulong(); // return 
}
/* This function extracts the offset as an interger from the bit-string version of the address 
* Assumes that the index size and offset size have been previously calculated 
*/
int getOffset(string fullAddress, int offsetSize, int indexSize)
{
	string offsetString = fullAddress.substr(fullAddress.size() - offsetSize, fullAddress.size());
	return bitset<32>(offsetString).to_ulong();
}
/* This function extracts the index as an interger from the bit-string version of the address 
* Assumes that the index size, tagSize and offset size have been previously calculated
* and uses the lineSize as determined from the config file  
*/
int getIndex(string fullAddress, int offsetSize, int tagSize, int indexSize, int lineSize)
{
	string indexString = fullAddress.substr(lineSize - (indexSize + offsetSize),indexSize);
	return bitset<32>(indexString).to_ulong();
}
/* This struct represents a bit-string of the address read in as a hexadecimal
* It's bascially a messy way of keeping all the information relevant for an address 
* in one place. 
* Data for constructing an address struct is read in from stdin (and assumes proper input format)
*/
struct address{ 
	address(int hexAddress, char accessTypeVariable, int lineSize, int numSets) 
	{
		this -> hexAddress = hexAddress;				// fill in hexAddress
		this -> fullAddress = hexToBinary(hexAddress, lineSize); //calculate 32bit bitstring 
		this -> accessType = determineAccessType(accessTypeVariable); // calculate accesstype as a string 
		offsetSize = determineOffsetSize(lineSize);					// determine offset size from config
		indexSize = determineIndexSize(numSets);					// determine index size from config 
		tagSize = determineTagSize(indexSize, offsetSize, lineSize);	// determine tag size from config 
		tag = getTag(fullAddress, offsetSize, indexSize);				// determine tag as an integer
		offset = getOffset(fullAddress, offsetSize, indexSize);			// determine index as an integer
		index = getIndex(fullAddress, offsetSize, tagSize, indexSize, lineSize); // determine index as an int
	}
	int hexAddress;
	int offsetSize;
	int indexSize;
	int tagSize;
	string fullAddress;
	string accessType;  //"write" or "read"; just makes it easier to print out for table later 
	int tag;
	int index;
	int offset; 

};
/*
*	This struct represents a single cache block and all appropriate bits (NOTE: actual data storage NOT implemented)
*/
struct cacheBlock
{
	bool valid = false; // if cache block has been accessed yet or not 
	bool dirty = false; // if cache has been written to yet 
	int tag = 0; // tag stored into cache 
	int use = 0; // for implementing LRU (if block A has use 3, it's the LRU compared to block B with use 4)
};
/*
*	This struct represents an 8-way set of cacheBlocks (largest possible for this assignment). 
*	Depending on the setSize parameter read, only a certain number of indices will be available
*/
struct blockSet 
{
	struct cacheBlock ways[8]; 
};
/*
*	This function searches a specific set for a cacheBlock with a tag matching the parameter tag(the address tag from
*	the .dat file reference). If a match is found, the index of the cacheBlock is returned, aka the way of the cache. If 
*	a match isn't found, -1 is returned to represent no match.  
*/
int findTag(blockSet & set, int setSize, int tag)
{
	int tagIndex = -1;
	for(int i = 0; i < setSize; i++)
	{	if(set.ways[i].valid)
		{
			if(set.ways[i].tag == tag)
				tagIndex = i;
		}
	}
	return tagIndex; 
}
/*
*This function implements a cache read on the simulated cache by searching for a tag match. If a tag match isn't found, then the 
* LRU cache block is updated (the absolute LRU is a cache that has yet to be used, otherwise its the cache in the set with the 
* lowest reference number). Depending on a hit or a miss, the appropriate bool is returned (true for hit, false for miss).
*
*/
bool readCache(blockSet cache[], address a, int setSize, int accessNum)
{
	bool hit = false; // assumes miss until proven otherwise 
	blockSet & set = cache[a.index]; // access set in cache determined by the index previously extracted from the address
	
	if(findTag(set, setSize, a.tag) == -1) // tag isn't in specified set, so find invalid spot to write in
	{
		int i = 0;
		while(i < setSize && set.ways[i].valid) // search set for invalid cacheBlock until one is found 
			i++;
		if(i == setSize) // if no invalid cacheBlocks were found, find LRU valid cacheBlock 
		{
			int lruIndex = 0;
			for(int j = 0; j < setSize -1; j++) //find LRU by comparing refNum (aka "use") of each cacheBlock (lower means used earlier)
			{
				if(set.ways[j].use < set.ways[j+1].use)
					lruIndex = j;
				else if( set.ways[j].use > set.ways[j+1].use)
					lruIndex = j+1;
			}		
			cacheBlock & block = set.ways[lruIndex]; //get LRU block from set 
			block.tag = a.tag; // update tag 
			block.use = accessNum; // update refNum 
		}
		else // an invalid cacheBlock was found, so this is the LRU in the sense that it hasn't been used yet, so update this block
		{
			cacheBlock & block = set.ways[i]; // get block from set
			block.valid = true; // update valid flag
			block.tag = a.tag; // update tag 
			block.use = accessNum; // update reference number to block for future LRU checks 
		}
		
	}else 
		hit = true; // hit is true, so in context of simulation, nothing else happens to the cache 
			

	return hit;
}
/*
*	This function implements a write-back write-allocation to the simulated cache. If a tag match is found, the cache is updated 
*	and a hit is returned. Otherwise, the LRU block is updated and a miss is returned. The memRef number is passed in as a referenced 
*	parameter for ease of coding. It is updated accordingly. In essence , two things are "returned" from this function: whether the 
* 	cache write resulted in a hit or a miss, and whether the memory was "referenced" in the simulation 
*/
bool writeCache(blockSet cache[], address a, int setSize, int accessNum, int &memRef)
{
	bool hit = false; // assumes miss until proven otherwise 
	blockSet & set = cache[a.index]; // get set pertaining to index extracted from adddress parameter 
	
	int tagIndex = findTag(set, setSize, a.tag); // search for matchign tag in cache 
	if(tagIndex == -1) // if tag isn't in specified set, find LRU spot to write in
	{
		int i = 0;
		while(i < setSize && set.ways[i].valid) // look for invalid spot in the set 
			i++;
		if(i == setSize) // all spots were filled, so find LRU cache block to write over 
		{
			int lruIndex = 0;
			for(int j = 0; j < setSize -1; j++) //find LRU
			{
				if(set.ways[j].use < set.ways[j+1].use) // compare use (lower refNum means cache is older)
					lruIndex = j;
				else if( set.ways[j].use > set.ways[j+1].use)
					lruIndex = j+1;
			}		
			cacheBlock & block = set.ways[lruIndex]; // get LRU cache block 
			
			if(block.dirty) //return appropriate memRef result (0 or 1)
				memRef = 2;
			else 		// block isn't dirty, so memory must be referenced 
			{
				memRef = 0;
				hit = true;
			}
								//update tag and accessNum of cache 
			block.tag = a.tag;
			block.use = accessNum;
		}
		else // there's a empty block, so fill that  
		{
			memRef = 1; // memory must be referenced 
			cacheBlock & block = set.ways[i]; // get block and update contents 
			block.valid = true;
			block.dirty = true; 
			block.tag = a.tag;
			block.use = accessNum;
		}
		
	}else //tag match was found, so overwrite the block and set the dirty bit to true (assuming its not already so)
	{
		cacheBlock & block = set.ways[tagIndex];
		hit = true;
		block.tag = a.tag;
		block.dirty = true; 
		memRef = 0; 
	}
	
	return hit; // return miss or hit 
}

// MAIN FUNCTION FOR SIMULATED DATA CACHE ******************************************************************************
int main() 
{
	
	int hits = 0; // records number of hits from references for calculating statistics at end of program 
	int misses = 0;// records number of misses from references for calculating statistics at end of program 
	int accesses = 0; // records number of references in total for calculating statistics at end of program 
	
	struct blockSet cache[8192]; // assuming max number of sets will be 8192
	
/*
*	Read in the config file and check for errors 
*/
	int numSets = 0; 
	int setSize = 0;// read in from config file; acts as barrier to size for hardcoded size 8 array of cacheBlocks in blockSet struct 
	int lineSize = 0; // used to calculate appropriate address fields 
	
	FILE * config; 
	config = fopen("trace.config", "r"); // open config file 
	
	//scan config file (assuming proper format) 
	fscanf(config, "Number of sets: %i\nSet size: %i\nLine size: %i\n", &numSets, &setSize, &lineSize); 
	
	
	fclose(config); // close config file 

/*
*	Print out the results table header 
*/
	printf("Cache Configuration\n\n   %i %i-way set associative entries\n   of line size %i bytes\n\n\n", numSets, setSize, lineSize);
	printf("Results for Each Reference\n\n");
	printf("Ref  Access Address    Tag   Index Offset Result Memrefs\n");
	printf("---- ------ -------- ------- ----- ------ ------ -------\n");
	
	char accessTypeVariable;		// variable to read in cache access type (R or W for read and write, respectively) from .dat file
	int size;						// variable to read in the size of the cache (1, 2, 4, or 8) from .dat file 
	int hexAddress;					// variable to read in the hex address from the .dat file 
	string hitOrMiss;				// string to be printed in table according to result of read or write 
	int memref;						// memory reference to be changed according to read or write and printed in table 
	bool validFormat;				// flag for checking that alignment and data reference size are correct for each reference  
	int refNum = 1; 
	
/*
*	Read in reference and check for valid formatting
*/
	
	while(scanf( "%c:%d:%x\n", &accessTypeVariable, &size, &hexAddress) == 3 )	// while there are still references read in 
	{
		validFormat = true; // assume valid until proven otherwise 
		
/*
*	check for valid formatting (alignment and size of data reference) 
*/
		if( size != 1 && size != 2 && size != 4 && size != 8)	//size of data reference must be 1,2,4, or 8	
		{
			validFormat = false;
			fprintf(stderr, "line %d has illegal size %d\n", refNum, size);
		} else if( hexAddress % size != 0)						// reference address must be properly aligned 
		{
			validFormat = false;
			fprintf(stderr, "line %d has misaligned reference at address %x for size %d\n", refNum, hexAddress, size);
		}
			
/*
*	If format is valid, parse out scanned address from reference and perform read or write on cache
*/
			
		if(validFormat) // if format of reference was valid 
		{
			address a(hexAddress, accessTypeVariable, lineSize, numSets); // create  address struct with appropriate information 
			accesses++;	// increment valid access counter 
			
			if(a.accessType.compare("read") == 0)	//if reference is of type read 
			{
					if(readCache(cache, a, setSize, accesses))	// if read is a hit 
					{
						hitOrMiss = "hit";
						memref = 0;
						hits++;
					}
					else 										// if read is a miss 
					{
						memref = 1;
						hitOrMiss = "miss";
						misses++;
					}
			} else									// if reference is of type write (note: memref is effectively "returned",
													// in the sense that it is directly modified by writeCache() function)
			{
				if(writeCache(cache, a, setSize, accesses, memref))			// if write is a hit
				{
					hitOrMiss = "hit";
					hits++;
				} else 														// if write is a miss
				{
					hitOrMiss = "miss";
					misses++;
				}
			}
			// print formatted result of cache reference into table 
			printf("%*i %*s %*x %*x %*i %*i %*s %*i\n", 4, accesses, 6, a.accessType.c_str(),8 ,a.hexAddress, 7, a.tag, 5, a.index, 6, a.offset,6, hitOrMiss.c_str(), 7, memref );
		}
		refNum++; // increment reference counter
		
	} // if reference format wasn't valid, skip the reference and go to next one in .dat file 
	
/*
*	Calculate and print out formatted statistics after references are read 
*/	
	
	printf("\n\nSimulation Summary Statistics\n-----------------------------\n");
	printf("Total hits       : %i\n", hits);
	printf("Total misses     : %i\n", misses);
	printf("Total accesses   : %i\n", accesses);
	double hitratio = static_cast<double>(hits)/accesses; // calculate hit ratio
	printf("Hit ratio        : %f\n", hitratio);			// print hit ratio
	printf("Miss ratio       : %f\n\n", 1 - hitratio);		// print miss ratio 
}