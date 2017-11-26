# Cuckoo-Open-Addressed-HashSet
Including sequential, concurrent and transactional version

This is an assignment of parallel computing course in Lehigh, here I paste the requirement below, 
all implementation and performance details can see this requirement and the REPORT.pdf.
Original alogrithm comes from the book "The Art of Multiprocessor Programming" chapter 13.4
(of course with some modification, since original algorithm has many potential defects inside)

Introduction:

  The purpose of this assignment is to implement three versions of an
  open-addressed hash set. To read more about open-addressed hash set design,
  you can find an introduction to that in Chapter 13.4 of the textbook
  "The Art of Multiprocessor Programming" and in many other books or online resources.
  
  The requested set implementation has to provide the following two APIs: 'add' and 'remove'.
  
  In addition to that, the set should provide a non-thread safe function (named 'size') 
  that counts the number of elements in the hash set, and a non-thread safe function 
  (named 'populate') that initializes the data structure with 1024 random elements. 
  
  The set has to allow applications to add any generic data type to the data structure.
  
  The following three versions of an open-addressed has set are required:
  - a sequential version, as specified in Chapter 13.4.1 (or a better variant of it);
  - a concurrent version as specified in Chapter 13.4.2 and 13.4.3;
  - a transactional version, which converts the sequential version in concurrent by
    exploiting the Transactional Memory abstraction as provided by GCC.
    
  In order to test the performance of the two concurrent versions, it is required
  to develop a simple application that spawns a pre-defined number of threads that
  operate on the implemented open-addressed hash set of integer elements. Each of these
  threads executes a pre-defined number of operations on the data structure with the
  following distribution: 60% insert; 40% remove. For simplicity, elements
  to be used by these data structure operations are random. To test the performance of
  the sequential implementation, you should use the same application configured to spawn
  only one thread. The outcome of each operation should be recorded and the expected size
  of the data structure at the end of the application execution should be computed.
  When all threads are joined, the function size should be invoked and it should match the
  value of the expected size.

  The goal of the homework is to contrast the performance of the thee implementations
  with each other. It is required to produce at least one plot that shows the performance
  of the concurrent implementations while increasing the number of working threads against
  the sequential implementation.
  
  The transactional version of the data structure uses very advanced synchronization
  abstractions, which are not yet as robust as classical locking mechanisms, such as mutex.
  Any effort made to accomplish the transactional version will be appreciated if documented.
  My expectation is that the concurrent version will outperform sequential and
  transactional.
  
  It is a requirement to use the machines available at sunlab.cse.lehigh.edu. An evaluation
  performed on your own laptop will not be considered enough, unless a specific
  authorization has been granted by me.
    
  Note that:
  - the developing language is C++;
  - TM is implemented in GCC starting from version 4.7. Use the latest version of GCC
    available in sunlab.
  - All the findings (successful and unsuccessful) should be recorded in a document to be
    submitted along with the implementations' source code.
  



