# Cuckoo-Open-Addressed-HashSet

### < I > Cuckoo HashSet: <br>
  A type of open-addressed hashset could be found in Chapter 13.4 of the textbook ["The Art of Multiprocessor Programming"](https://www.e-reading.club/bookreader.php/134637/Herlihy,_Shavit_-_The_art_of_multiprocessor_programming.pdf), please note that there are some defects of original algorithms.
  
### < II > Code Structure:<br>
  Three ".h" file contain the sequential hashset, concurrent hashset and transactional hashset(also a version of concurrent), main function in "hash.cpp" calls these three hashset header file then uses timer to count running time of all these three versions of hashset.

### < III > How main function works:<br>
  Variable named "OPERATION_NUM" set total number of "add" and "remove" operations, there are 60% "add" and 40% "remove" amaong all these operations. And there is "addedArray" and "removedArray" store all elements we need to add into and remove from our hashset. Sequential version of hashset just call add then call remove sequentially. Concurrent and transactional versions need to devide all opeartions in same amount into each thread then call them. We firstly use muli-thread to finish add operation then use same number of threads to finish remove, since call them together in same thread will cause some loss of removing.
  In addition, there is an official unordered hashset in main function and do the same operations to show that whether our self-defined hashsets could correctly match results of official hashset.
  
### < IV > About Transaction Toolkits<br>
   The transactional version of the data structure uses very advanced synchronization abstractions, which are not yet as robust as classical locking mechanisms, such as mutex. Thus under strict test case (like large amount of operation number and resizing case) transactional version will not work.
   There is a "transaction relax" tool maybe useful to solve this issue. I haven't try this tech but you can have a learning a try :)
   
### < V > Platforms<br>
    Linux is the matched OS, the transaction memory toolkits are implemented in GCC starting from version 4.7. Use the latest version of GCC available (I use gcc-7.1.0).
   
