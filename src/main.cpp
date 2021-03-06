/**
 * @file      main.cpp
 * @brief     Stream compaction test program
 * @authors   Kai Ninomiya, Xiang Deng
 * @date      2015, 2016
 * @copyright University of Pennsylvania
 */

#include <cstdio>
#include <stream_compaction/cpu.h>
#include <stream_compaction/naive.h>
#include <stream_compaction/efficient.h>
#include <stream_compaction/thrust.h>
#include <stream_compaction/radixSort.h>
#include "testing_helpers.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
int main(int argc, char* argv[]) {
    const int SIZE = 1 << 20;
    const int NPOT = SIZE - 3;
    int a[SIZE], b[SIZE], c[SIZE];
	float milscs;
	int nitercpu = 100;
	
    // Scan tests

    printf("\n");
    printf("****************\n");
    printf("** SCAN TESTS **\n");
    printf("****************\n");

    genArray(SIZE - 1, a, 50);  // Leave a 0 at the end to test that edge case
    a[SIZE - 1] = 0;
    printArray(SIZE, a, true);

    zeroArray(SIZE, b);
    printDesc("cpu scan, power-of-two");
	auto begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < nitercpu; i++){
		StreamCompaction::CPU::scan(SIZE, b, a);
	}
	auto end = std::chrono::high_resolution_clock::now(); 
	milscs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()  / (float)nitercpu;
    printArray(SIZE, b, true);
	printf("time lapsed %f ms\n", milscs);


    zeroArray(SIZE, c);
    printDesc("cpu scan, non-power-of-two");
	 begin = std::chrono::high_resolution_clock::now();
	 for (int i = 0; i < nitercpu; i++){
		 StreamCompaction::CPU::scan(NPOT, c, a);
	 }
	 end = std::chrono::high_resolution_clock::now();
	 milscs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()  / nitercpu;
    printArray(NPOT, b, true);
    printCmpResult(NPOT, b, c);
	printf("time lapsed %f ms\n", milscs);


    zeroArray(SIZE, c);
    printDesc("naive scan, power-of-two");
	milscs = StreamCompaction::Naive::scan(SIZE, c, a);
	printArray(SIZE, c, true);
    printCmpResult(SIZE, b, c);
	printf("time lapsed %f ms\n", milscs);

    zeroArray(SIZE, c);
    printDesc("naive scan, non-power-of-two");
	milscs = StreamCompaction::Naive::scan(NPOT, c, a);
    printArray(SIZE, c, true);
    printCmpResult(NPOT, b, c);
	printf("time lapsed %f ms\n", milscs);
	
	 
    zeroArray(SIZE, c);
    printDesc("work-efficient scan, power-of-two");
    milscs = StreamCompaction::Efficient::scan(SIZE, c, a);
    printArray(SIZE, c, true);
    printCmpResult(SIZE, b, c);	 
	printf("time lapsed %f ms\n", milscs);

    zeroArray(SIZE, c);
    printDesc("work-efficient scan, non-power-of-two");
    milscs = StreamCompaction::Efficient::scan(NPOT, c, a);
    printArray(NPOT, c, true);
    printCmpResult(NPOT, b, c);
	printf("time lapsed %f ms\n", milscs);


    zeroArray(SIZE, c);
    printDesc("thrust scan, power-of-two");
	begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < nitercpu; i++){
		StreamCompaction::Thrust::scan(SIZE, c, a);
	}
	end = std::chrono::high_resolution_clock::now();
	milscs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / nitercpu;
    printArray(SIZE, c, true);
    printCmpResult(SIZE, b, c);
	printf("time lapsed %f ms\n", milscs);

    zeroArray(SIZE, c);
    printDesc("thrust scan, non-power-of-two");
	begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < nitercpu; i++){
		StreamCompaction::Thrust::scan(NPOT, c, a);
	}
	end = std::chrono::high_resolution_clock::now();
	milscs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / nitercpu;
    printArray(NPOT, c, true);
    printCmpResult(NPOT, b, c);
	printf("time lapsed %f ms\n", milscs);

    printf("\n");
    printf("*****************************\n");
    printf("** STREAM COMPACTION TESTS **\n");
    printf("*****************************\n");

    // Compaction tests

    genArray(SIZE - 1, a, 4);  // Leave a 0 at the end to test that edge case
    a[SIZE - 1] = 0;
    printArray(SIZE, a, true);

    int count, expectedCount, expectedNPOT;

    zeroArray(SIZE, b);
    printDesc("cpu compact without scan, power-of-two");
	begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < nitercpu; i++){
		count = StreamCompaction::CPU::compactWithoutScan(SIZE, b, a);
	}
	end = std::chrono::high_resolution_clock::now();
	milscs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()  / nitercpu;
    expectedCount = count;
    printArray(count, b, true);
    printCmpLenResult(count, expectedCount, b, b);
	printf("time lapsed %f ms\n", milscs);

    zeroArray(SIZE, c);
    printDesc("cpu compact without scan, non-power-of-two");
	begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < nitercpu; i++){
		count = StreamCompaction::CPU::compactWithoutScan(NPOT, c, a);
	}
	end = std::chrono::high_resolution_clock::now();
    expectedNPOT = count;
    printArray(count, c, true);
    printCmpLenResult(count, expectedNPOT, b, c);
	milscs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()  / nitercpu;
	printf("time lapsed %f ms\n", milscs);

    zeroArray(SIZE, c);
    printDesc("cpu compact with scan");
	begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < nitercpu; i++){
		count = StreamCompaction::CPU::compactWithScan(SIZE, c, a);
	}
	end = std::chrono::high_resolution_clock::now();
    printArray(count, c, true);
    printCmpLenResult(count, expectedCount, b, c);
	milscs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()  / nitercpu;
	printf("time lapsed %f ms\n", milscs);

    zeroArray(SIZE, c);
    printDesc("work-efficient compact, power-of-two");
	count = StreamCompaction::Efficient::compact(SIZE, c, a, milscs);
    printArray(count, c, true);
    printCmpLenResult(count, expectedCount, b, c);
	printf("time lapsed %f ms\n", milscs);

    zeroArray(SIZE, c);
    printDesc("work-efficient compact, non-power-of-two");
	count = StreamCompaction::Efficient::compact(NPOT, c, a, milscs);
    printArray(count, c, true);
    printCmpLenResult(count, expectedNPOT, b, c);
	printf("time lapsed %f ms\n", milscs);

    printf("\n");
    printf("*****************************\n");
    printf("** RADIX SORT TESTS **\n");
    printf("*****************************\n");
    genArray(SIZE - 1, a, 200);  // Leave a 0 at the end to test that edge case
    a[SIZE - 1] = 0;
	printDesc("Array to be sorted power of 2");
    printArray(SIZE, a, true);
	zeroArray(SIZE, b);
    printDesc("RADIX SORT POT");
	int msb=31;
	printf("size of int is %d bits\n", sizeof(int)*CHAR_BIT);
    StreamCompaction::RadixSort::sort(SIZE, b, a, msb); 
    printArray(SIZE, b, true);

	printDesc("C++ SORT POT"); 
	memcpy(c, a, SIZE*sizeof(int)); 
	std::sort(c,c+SIZE); 
	printArray(SIZE, c, true);
    printCmpResult(SIZE, b, c);

	genArray(SIZE - 1, a, 2000);  // Leave a 0 at the end to test that edge case
    a[SIZE - 1] = 0;
	printDesc("Array to be sorted not power of 2");
    printArray(SIZE, a, true);
	zeroArray(SIZE, b);
	zeroArray(SIZE, c);
    printDesc("RADIX SORT NPOT"); 
	printf("size of int is %d bits\n", sizeof(int)*CHAR_BIT);
    StreamCompaction::RadixSort::sort(NPOT, b, a, msb); 
	printArray(NPOT, b, true);

	printDesc("C++ SORT NPOT"); 
	memcpy(c, a, SIZE*sizeof(int)); 
	std::sort(c,c+NPOT); 
	printArray(NPOT, c, true);
    printCmpResult(NPOT, b, c);

}
