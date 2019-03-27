# Apriori_for_ARM

The most important aspect of this assignment was to understand there may be a huge difference between what a paper claims and the real implementation about the execution time. Not only the differences originate from the algorithm, but also from the data structure that we use in the implementation.

### Hardware and Software Specifications:
I compiled this code in Ubuntu 18.04 (64-bit version) using GCC version 7.3.0 and I had Intel i5-8500 CPU.
You may see different execution times using different OS, compiler, or hardware.

### To compile the code, use this line in the terminal:
g++ -std=c++17 -O2 AprioriImplementation.cpp

### to run the code and see all details (with proper dataset file, support and value):
./a.out 10Kdataset.txt 0.001 0.8

### This is a brief report of measures and run times for each of the three scenarios I executed the code:

#### results for \~10K transactions:
9949 transactions, 488 different items, run time =~ 12.74 seconds.  
number of 1 to 10 frequent itemsets: (472, 24085, 8049, 3786, 2329, 1115, 407, 111, 21, 2)  
There are 161856 strong association rules overall.


#### results for 1K transactions:
1000 transactions, 475 different items, run time =~ 0.11 seconds.  
number of 1 to 2 frequent itemsets:(334, 174)  
There is no strong association rules in this case.


#### results for 100 transactions:
100 transactions, 366 different items, run time =~ 0.00 seconds.  
number of 1 frequent itemsets: (10)  
There is no strong association rules in this case.


** To my knowledge, "-O3" is not as stable and consistent as "-O2", also there
   may be some problems if you compile the code with another version of g++. I
   have seen cases (on a computer with different version of GCC) with some 
   problems generating all the strong association rules.

** This code is designed in a way that can handle databases with .txt and .csv
   format (I need to evaluate my code using .csv files, so I add supporting of
   such format to the code). If the input is a text file, make sure to have '\n'
   at the end of each line. Also it should follow UTF-8 and Unix(LF) to work
   properly. Since there is a discrepancy between Windows and Linux for this
   matter, you may see problems if you use improper database file.
