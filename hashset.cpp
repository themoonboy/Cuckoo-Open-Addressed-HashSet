//
//  main.cpp
//  hash set
//
//  Created by Zhaohan Xi on 17/11/16.
//  Copyright © 2017 Zhaohan Xi. All rights reserved.
//

// Cuckoo_hash.cpp : Defines the entry point for the console application.
//

#include "sequential_hashset.h"
#include "concurrent_hashset.h"
#include "transactional_hashset.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <unistd.h>
#include <unordered_set>

using namespace std;
using namespace std::chrono;

/*
 test function used for sequential test
 */
void sequential_test(Sequential_hashset<int> &myset, vector<int> &addedArray,vector<int> &removedArray) {
    
    for(long i=0; i<addedArray.size(); i++) {
        myset.add(addedArray.at(i));
    }
    
    for(long i=0; i<removedArray.size(); i++) {
        myset.remove(removedArray.at(i));
    }
}

// This function prints some helpful usage information
void usage() {
    cout << "Open-address Hash Set Implementation" << endl;
    cout << "  Usage: hashset [options]" << endl;
    cout << "    -t <int> : specify the number of threads" << endl;
    cout << "    -c <int> : specify the size of initial capacity" << endl;
    cout << "    -n <int> : specify the operation number" << endl;
    cout << "    -h       : print this message" << endl;
}

int main(int argc, char* argv[])
{

    //////////////////////  Section 1: Parameters Setting  ///////////////////////

    int THREAD_NUM = 8;
    long CAPACITY = 10000;
    long OPERATION_NUM = 0.5*CAPACITY;

    int o;
    while ((o = getopt(argc, argv, "t:c:n:h")) != -1) {
        switch (o) {
          case 't': THREAD_NUM = atoi(optarg);  break;
          case 'c': CAPACITY = atoi(optarg); OPERATION_NUM = 0.5*CAPACITY; break;
          case 'n': OPERATION_NUM = atoi(optarg); break;
          case 'h': usage();              break;
          default:  usage();              exit(-1);
        }
    }

    if(OPERATION_NUM%(5*THREAD_NUM)!=0) {

        cout<<"\n-------------------!!!NOTE!!!-------------------\n";
        cout<<"In order to ensure each thread operates with same amount\n";
        cout<<"of operations, we have to modify the operation number\n"<<endl;

        long diff = 5 - (OPERATION_NUM/THREAD_NUM)%5;
        long ratio = diff + (OPERATION_NUM/THREAD_NUM);
        OPERATION_NUM = ratio*THREAD_NUM;
    }
    
    cout<<"Number of thread is: "<<THREAD_NUM<<endl;
    cout<<"Initial capacity is: "<<CAPACITY<<endl;
    cout<<"Operation number is: "<<OPERATION_NUM<<endl;
    /*
        note!!: based my concurrent_addtest(removetest),
                addSize/THRAED_NUM and removeSize/THREAD_NUM
                should be integer, which means the parameter
                above should sastifying:
     
                0.6*OPERATION/THREAD_NUM = int and 
                0.4*OPERATION/THREAD_NUM = int
     
                therefore,
     
                OPEARTION/THREAD_NUM%5 = 0 
     
                should be satisfied, otherwise the resultes 
                of sequential and concurrent version cannot match
     
     */
    
    Sequential_hashset<int> sequentialSet(CAPACITY);
    Concurrent_hashset<int> concurrentSet(CAPACITY);
    Transactional_hashset<int> transactionalSet(CAPACITY);
    

    //////////////////////  Section 2: Operation Array Definition  ///////////////////////

    /*
     define the array whose element we needed to added in and
     remove off from set, three part of test should use same
     array,also initialArray used together with populate function
     to initial 1024 random elements for each set.
     */
    vector<int> addedArray;
    vector<int> removedArray;
    vector<int> initialArray;

    for(long i=0; i<OPERATION_NUM; i++) {
        if(i%5<3)  //i=0,1,2, 60% percentage
        {
            int key =rand()%max(CAPACITY,OPERATION_NUM);
            addedArray.push_back(key);
        }
        else    // i=3,4, 40% percentage
        {
            int key =rand()%max(CAPACITY,OPERATION_NUM);
            removedArray.push_back(key);
        }
    }

    for(int i=1; i<=1024; i++) {
        initialArray.push_back(rand());
    }
    sequentialSet.populate(initialArray);
    concurrentSet.populate(initialArray);
    transactionalSet.populate(initialArray);



    //////////////////////  Section 3: Conference Set Operation  ///////////////////////

    /*
     a reference set used for shown actual size
     after series add and remove operations
     */
    unordered_set<int> realSet;
    for(int i=0; i<initialArray.size(); i++) {
        realSet.insert(initialArray.at(i));
    }
    for(long i=0; i<addedArray.size(); i++) {
        realSet.insert(addedArray.at(i));
    }
    for(long i=0; i<removedArray.size(); i++) {
        realSet.erase(removedArray.at(i));
    }

    
    //////////////////////  Section 4: Sequential Test  /////////////////////// 

    /*
     sequential test block:
     call "sequential_test", added and remove element inside of
     sequentialSet, then print out the final size of sequentialSet
     */
    std::chrono::high_resolution_clock::time_point start_time0, end_time0;
    start_time0 = std::chrono::high_resolution_clock::now();
    
    std::thread t0(sequential_test, std::ref(sequentialSet), std::ref(addedArray), std::ref(removedArray));
    t0.join();
    
    end_time0 = std::chrono::high_resolution_clock::now();
    duration<double> time_span0 = duration_cast<duration<double>>(end_time0 - start_time0);
    double exce_time0 = time_span0.count();
    
    cout<<endl;
    cout<<"SEQUENTIAL TEST:"<<endl;
    cout<<"sequential version takes : "<<exce_time0<<" seconds"<<endl;
    cout<<"After operations, set valid size is: "<<sequentialSet.size()<<endl;
    
    
    //////////////////////  Section 5: Concurrency Test  ///////////////////////

    /*
     concurrency test block:
     call "concurrent_test", added and remove element inside of
     concurrentSet, then print out the final size of concurrentSet
     */
    long addSize = addedArray.size();
    long removeSize = removedArray.size();

    /*
     anonymous functions for concurrency test,
     
     note!!!!!! we should seperate add and remove operations,
                which means in multi-threads, I should firstly
                create multi-threads for add only, then join,
                then create multi-threads again for remove only,
                then join. The reason are showns by an example:
     
                assume we need to add [1,2,3,4,5,6] to my set
                and remove [4, 2] from set, after sequential 
                the remained elements are [1,3,5,6], thus the
                set size is 4. However, when we use two threads
                and first thread will be allocate first half of
                these two array, which are [1,2,3] and [4],
                if I put remove following with add in one thread,
                then after first part of add and remove the set will
                still be [1,2,3] sicne I add [1,2,3] but there is no
                4 to remove. After second part of operations the set
                will finally be [1,3,4,5,6], size will be 5
     
                therefore, it's better to seperate add and remove in
                multi-threads.
     */
    auto concurrent_addtest = [&](int tid) {
        for(long i=0; i<addSize/THREAD_NUM; i++) {
            concurrentSet.add(addedArray.at(i+(addSize/THREAD_NUM)*tid));
        }
    };
    auto concurrent_removetest = [&](int tid) {
        for(int i=0; i<removeSize/THREAD_NUM; i++) {
            concurrentSet.remove(removedArray.at(i+(removeSize/THREAD_NUM)*tid));
        }
    };

    std::chrono::high_resolution_clock::time_point start_time1, end_time1;
    start_time1 = std::chrono::high_resolution_clock::now();
    
    vector<thread> add_threads;
    for(int i=0; i<THREAD_NUM; i++) {
        add_threads.push_back(thread(concurrent_addtest, i));
    }
    for(int i=0; i<THREAD_NUM; i++) {
        add_threads.at(i).join();
    }
    
    vector<thread> remove_threads;
    for(int i=0; i<THREAD_NUM; i++) {
        remove_threads.push_back(thread(concurrent_removetest, i));
    }
    for(int i=0; i<THREAD_NUM; i++) {
        remove_threads.at(i).join();
    }
    
    end_time1 = std::chrono::high_resolution_clock::now();
    duration<double> time_span1 = duration_cast<duration<double>>(end_time1 - start_time1);
    double exce_time1 = time_span1.count();

    cout<<endl;
    cout<<"CONCURRENCY TEST:"<<endl;
    cout<<"concurrent version takes : "<<exce_time1<<" seconds"<<endl;
    cout<<"After operations, set valid size is: "<<concurrentSet.size()<<endl;


    //////////////////////  Section 6:  Transaction Test  //////////////////////

    /*
     transactional test block
    */
    auto transactional_addtest = [&](int tid) {
        for(long i=0; i<addSize/THREAD_NUM; i++) {
            transactionalSet.add(addedArray.at(i+(addSize/THREAD_NUM)*tid));
        }
    };
    auto transactional_removetest = [&](int tid) {
        for(int i=0; i<removeSize/THREAD_NUM; i++) {
            transactionalSet.remove(removedArray.at(i+(removeSize/THREAD_NUM)*tid));
        }
    };

    std::chrono::high_resolution_clock::time_point start_time2, end_time2;
    start_time2 = std::chrono::high_resolution_clock::now();
    
    vector<thread> add_threads1;
    for(int i=0; i<THREAD_NUM; i++) {
        add_threads1.push_back(thread(transactional_addtest, i));
    }
    for(int i=0; i<THREAD_NUM; i++) {
        add_threads1.at(i).join();
    }
    
    vector<thread> remove_threads1;
    for(int i=0; i<THREAD_NUM; i++) {
        remove_threads1.push_back(thread(transactional_removetest, i));
    }
    for(int i=0; i<THREAD_NUM; i++) {
        remove_threads1.at(i).join();
    }
    
    end_time2 = std::chrono::high_resolution_clock::now();
    duration<double> time_span2 = duration_cast<duration<double>>(end_time2 - start_time2);
    double exce_time2 = time_span2.count();

    cout<<endl;
    cout<<"TRANSCATION TEST:"<<endl;
    cout<<"transactional version takes : "<<exce_time2<<" seconds"<<endl;
    cout<<"After operations, set valid size is: "<<transactionalSet.size()<<endl;
    
    cout<<endl;
    cout <<"Actual size should be: "<<realSet.size() << endl;
    
    return 0;
}
