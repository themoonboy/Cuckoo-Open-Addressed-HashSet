//
//  concurrent_hashset.h
//  hash set
//
//  Created by Zhaohan Xi on 17/11/18.
//  Copyright © 2017 Zhaohan Xi. All rights reserved.
//

#ifndef concurrent_hashset_h
#define concurrent_hashset_h

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;


/////////////////////////////////////////////////////////////////////////

template <typename T>
class Concurrent_hashset {
    
public:
    
    /*
        when OPEARTION_NUM>>CAPACITY, its better to set these two number bigger,
        since there may be many elements goes to a same place at same time
     */
    int PROBE_SIZE = 10;
    int THRESHOLD = 5;
    
    vector<T> *table0;
    vector<T> *table1;
    
    std::mutex *locks0;
    std::mutex *locks1;
    
    long eachTableSize;
    long lockSize;
    
public:
    
    /////////////////////////////////////////////////////////////////////////
    /*
     constructor:
     (1)initialize two hash table, which are queue<Element<T>> array,
     (2)alse store the size of each hash table
     
     */
    Concurrent_hashset(long capacity) {
        
        table0 = new vector<T>[capacity];
        table1 = new vector<T>[capacity];
        
        locks0 = new std::mutex[capacity];
        locks1 = new std::mutex[capacity];
        
        eachTableSize = (long)capacity;
        lockSize = (long)capacity;
        
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
     size:
     show valid size of my hash table
     */
    long size() {
        long res0 = 0, res1 = 0;
        for(long i=0; i<eachTableSize; i++) {
            if(!table0[i].empty()) { res0 += table0[i].size(); }
            if(!table1[i].empty()) { res1 += table1[i].size(); }
        }
        
        //cout<<"Valid size of table0 is: "<<res0<<endl;
        //cout<<"Valid size of table1 is: "<<res1<<endl;
        return res0+res1;
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
     contians:
     check whether an element already contains inside of two tables
     
     vector_contians:
     check whether an element is in the vector
     */
    
    bool vector_contains(vector<T> &v, T ele ) {
        for(int i=0; i<v.size(); i++) {
            if(v.at(i)==ele)
                return true;
        }
        return false;
    }
    
    bool contains(T ele) {
        
        //cout<<"Contains is working now, the ele is: "<<ele<<endl;
        long pos0 = hash0(ele);
        long pos1 = hash1(ele);
        bool res = false;
        if( vector_contains(table0[pos0], ele) || vector_contains(table1[pos1], ele) )
        {
            res = res || true;
        }
        //cout<<"The result is: "<<res<<endl;
        return res;
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
        populate:
        initialize two has tables
    */
    
    void populate(vector<T> &initialArray) {
        
        for(int i=0; i<initialArray.size(); i++) {
            add(initialArray.at(i));
        }
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
     hash0 & hash1
     two hash functions, one for table1, one for table2
     */
    long hash0(T ele) {
        std::hash<T> h0;
        long res = h0(ele)%eachTableSize;
        return res<0? -res: res;
        
    }
    
    long hash1(T ele) {
        std::hash<T> h1;
        long res = (h1(ele)+19238534)%eachTableSize;
        return res<0? -res: res;
        
    }
    
    long lock_hash0(T ele) {
        std::hash<T> h0;
        long res = h0(ele)%lockSize;
        return res<0? -res: res;
        
    }

    long lock_hash1(T ele) {
        std::hash<T> h1;
        long res = (h1(ele)+19238534)%lockSize;
        return res<0? -res: res;
        
    }
    
    /////////////////////////////////////////////////////////////////////////
    /*
        acquire & release:
     */
    void acquire(T ele) {
        
        //cout<<"Now we lock: "<<ele<<endl;
        locks0[lock_hash0(ele)].lock();
        locks1[lock_hash1(ele)].lock();
    }
    
    void release(T ele) {
        
        //cout<<"Now we release: "<<ele<<endl;
        locks0[lock_hash0(ele)].unlock();
        locks1[lock_hash1(ele)].unlock();
    }
    
    /////////////////////////////////////////////////////////////////////////
    /*
     resize:
     double the size of each table, and re-add all original element
     to the new tables one by one
     */
    void resize() {
        //cout << "concurrent resize working\n";
        long oldCapacity = eachTableSize;
        
        for(long i=0; i<lockSize; i++) {
            locks0[i].lock();
        }
        
        if(eachTableSize != oldCapacity) {
            return;
        }
        
        vector<T> *table2 = table0;
        vector<T> *table3 = table1;
        eachTableSize = 2*eachTableSize;
        table0 = new vector<T>[eachTableSize];
        table1 = new vector<T>[eachTableSize];
        
        for(long i = 0; i<oldCapacity; i++) {
            for(int j=0; j<table2[i].size(); j++) {
                nolock_add(table2[i].at(j));
            }
            for(int k=0; k<table3[i].size(); k++) {
                nolock_add(table3[i].at(k));
            }
        }
        
        for(long i=0; i<lockSize; i++) {
            locks0[i].unlock();
        }
        
        
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
     remove:
     given an element and try to remove it from our two hashtable,
     firstly judge whether our hashtables contains this element,
     if not contains, just return false means we don't have such element,
     else find this element and change the status of position of current
     element into "false" and return true (also change the value into
     nothing T(), this step is not necessary)
     */
    bool remove(T ele) {
        acquire(ele);
        vector<T> *set0 = &table0[hash0(ele)];
        auto it = std::find(set0->begin(), set0->end(), ele);
        if (it != set0->end()) {
            set0->erase(it);
            release(ele);
            return true;
        }
        else {
            vector<T> *set1 = &table1[hash1(ele)];
            it = std::find(set1->begin(), set1->end(), ele);
            if (it != set1->end()) {
                set1->erase(it);
                release(ele);
                return true;
            }
            
            release(ele);
            return false;
        }
        /*
        if(vector_contains(*set0, ele))
        {
            //remove from table0--set0
            for(int i=0; i<set0->size(); i++) {
                if(set0->at(i)==ele)
                {
                    set0->erase(set0->begin()+i);
                    release(ele);
                    return true;
                }
            }
        }
        else
        {
            vector<T> *set1 = &table1[hash1(ele)];
            if(vector_contains(*set1, ele))
            {
                //remove from table1--set1
                for(int i=0; i<set1->size(); i++) {
                    if(set1->at(i)==ele)
                    {
                        set1->erase(set1->begin()+i);
                        release(ele);
                        return true;
                    }
                }
            }
        }
         
        release(ele);
        return false;
         */
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    
    bool add(T ele) {
        
        //cout<<"Here we add a new element!!! "<<ele<<endl;
        //T y = T();
        acquire(ele);
        long h0 = hash0(ele), h1 = hash1(ele);
        bool mustResize = false;
        long i = -1, h = -1;
        
        if(contains(ele))
        {
            //cout<<"Already contains: "<<ele<<endl;
            release(ele);
            return false;
        }
        
        vector<T> *set0 = &table0[h0];
        vector<T> *set1 = &table1[h1];
        
        if(set0->size()<THRESHOLD)
        {
            set0->push_back(ele);
            release(ele);
            return true;
        }
        
        else if(set1->size()<THRESHOLD)
        {
            set1->push_back(ele);
            release(ele);
            return true;
        }
        
        else if(set0->size()<PROBE_SIZE)
        {
            set0->push_back(ele);
            i = 0;
            h = h1;
        }
        
        else if(set1->size()<PROBE_SIZE)
        {
            set1->push_back(ele);
            i = 1;
            h = h1;
        }
        
        else
        {
            mustResize = true;
        }
        
        release(ele);
        
        if(mustResize)
        {
            resize();
            return add(ele);
        }
        
        else if(!relocate(i, h))
        {
            resize();
        }
         
        return true;
    }
    
    /////////////////////////////////////////////////////////////////////////
    /*
        nolock_add:
        this function used for resize process, since in the add of process,
        its prossible that the add position is currently locked, we still 
        need to add element one element into new table, thus create this
        function
     */
    bool nolock_add(T ele) {
        
        
        long h0 = hash0(ele), h1 = hash1(ele);
        
        vector<T> *set0 = &table0[h0];
        vector<T> *set1 = &table1[h1];
        
        if(set0->size()<THRESHOLD)
        {
            set0->push_back(ele);
            return true;
        }
        
        else if(set1->size()<THRESHOLD)
        {
            set1->push_back(ele);
            
            return true;
        }
        
        else if(set0->size()<PROBE_SIZE)
        {
            set0->push_back(ele);
        }
        
        else if(set1->size()<PROBE_SIZE)
        {
            set1->push_back(ele);
        
        }
        
       
        return true;
    }

    
    
    /////////////////////////////////////////////////////////////////////////
    /*
        only when threshold and PORBE_SIZE is not very big,
        relocate will work
     */
    bool relocate(long i, long hi) {
        
        //cout<<"concurrent relocate working\n";
        long hj = 0;
        long j = 1 - i;
        int LIMIT = 500;
        
        for(int round = 0; round<LIMIT; round++) {
            
            vector<T> *iSet;
            if(i==0) { iSet = &table0[hi]; }
            else { iSet = &table1[hi]; }
            
            if (iSet == nullptr || iSet->size()==0){ return false; }
            
            T y = iSet->front();
            
            //Element<T> obj = *new Element<T>(y);
            
            if(i==0) { hj = hash1(y); }
            else { hj = hash0(y); }
            
            acquire(y);
            vector<T> *jSet;
            if(j==0) { jSet = &table0[hj]; }
            else { jSet = &table1[hj]; }
            
            if(vector_contains(*iSet, y))
               {
                   iSet->erase(iSet->begin());
                   if(jSet->size()<THRESHOLD)
                   {
                       jSet->push_back(y);
                       release(y);
                       return true;
                   }
                   
                   else if(jSet->size()<PROBE_SIZE)
                   {
                       jSet->push_back(y);
                       i = 1 - i;
                       j = 1 - j;
                       hi = hj;
                   }
                   else
                   {
                       iSet->push_back(y);
                       release(y);
                       return false;
                   }
               }
               else if(iSet->size()>=THRESHOLD)
               {
                   continue;
               }
               else
               {
                   release(y);
                   return true;
               }
               release(y);

            
        }
        return false;
    }
            
            
    
};



#endif /* concurrent_hashset_h */
