//
//  transactional_hashset.h
//  hash set
//
//  Created by Zhaohan Xi on 17/11/18.
//  Copyright © 2017 Zhaohan Xi. All rights reserved.
//

#ifndef transactional_hashset_h
#define transactional_hashset_h

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;


/////////////////////////////////////////////////////////////////////////

template <typename T>
class Transactional_hashset {
    
public:
    
    /*
     when OPEARTION_NUM>>CAPACITY, its better to set these two number bigger,
     since there may be many elements goes to a same place at same time
     */
    int PROBE_SIZE = 10;
    int THRESHOLD = 5;
    
    vector<T> *table0;
    vector<T> *table1;
    
    
    long eachTableSize;
    
public:
    
    /////////////////////////////////////////////////////////////////////////
    /*
     constructor:
     (1)initialize two hash table, which are queue<Element<T>> array,
     (2)alse store the size of each hash table
     
     */
    Transactional_hashset(long capacity) {
        
        table0 = new vector<T>[capacity];
        table1 = new vector<T>[capacity];
        
        
        eachTableSize = (long)capacity;
        
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
     generci push_back
    */
    __attribute__((transaction_pure))
    void pushback(vector<T> *v, T ele) {
        v->insert(v->end(), ele);
    }
    
    /////////////////////////////////////////////////////////////////////////
    /*
     contians:
     check whether an element already contains inside of two tables
     
     vector_contians:
     check whether an element is in the vector
     */
    
    __attribute__((transaction_pure))
    bool vector_contains(vector<T> &v, T ele ) {
        
        for(int i=0; i<v.size(); i++) {
            if(v.at(i)==ele)
                return true;
        }
        return false;
    
    }
    
    __attribute__((transaction_pure))
    bool contains(T ele) {
        
        //cout<<"Contains is working now, the ele is: "<<ele<<endl;
        long pos0 = hash0(ele);
        long pos1 = hash1(ele);
        bool res = false;

        __transaction_atomic {
            if( vector_contains(table0[pos0], ele) || vector_contains(table1[pos1], ele) )
            {
                res = res || true;
            }
            //cout<<"The result is: "<<res<<endl;
            return res;
        }
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
    
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
     resize:
     double the size of each table, and re-add all original element
     to the new tables one by one
     */
    __attribute__((transaction_pure))
    void resize() {
        //cout << "transactional resize working\n";
        vector<T> *table2 = table0;
        vector<T> *table3 = table1;

        __transaction_atomic {
            
            long oldCapacity = eachTableSize;
            
            if(eachTableSize != oldCapacity) {
                return;
            }
            
            eachTableSize = 2*eachTableSize;
            table0 = new vector<T>[eachTableSize];
            table1 = new vector<T>[eachTableSize];
            
            for(long i = 0; i<oldCapacity; i++) {
                for(int j=0; j<table2[i].size(); j++) {
                    add(table2[i].at(j));
                }
                for(int k=0; k<table3[i].size(); k++) {
                    add(table3[i].at(k));
                }
            }
            
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
    __attribute__((transaction_pure))
    bool remove(T ele) {
        
        __transaction_atomic {
            
            vector<T> *set0 = &table0[hash0(ele)];
            auto it = std::find(set0->begin(), set0->end(), ele);
            if (it != set0->end()) {
                set0->erase(it);
                
                return true;
            }
            else {
                vector<T> *set1 = &table1[hash1(ele)];
                it = std::find(set1->begin(), set1->end(), ele);
                if (it != set1->end()) {
                    set1->erase(it);
                    
                    return true;
                }
                
                return false;
            }
            
        }
        /*
         if(vector_contains(*set0, ele))
         {
         //remove from table0--set0
         for(int i=0; i<set0->size(); i++) {
         if(set0->at(i)==ele)
         {
         set0->erase(set0->begin()+i);
         
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
         
         return true;
         }
         }
         }
         }
         
         
         return false;
         */
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    __attribute__((transaction_pure))
    bool add(T ele) {
        
        //cout<<"Here we add a new element!!! "<<ele<<endl;
        //T y = T();
        
            
        long h0 = hash0(ele), h1 = hash1(ele);
            
        long i = -1, h = -1;
            
        if(contains(ele))
        {
                //cout<<"Already contains: "<<ele<<endl;
            return false;
        }
            

        vector<T> *set0 = &table0[h0];
        vector<T> *set1 = &table1[h1];
            
        __transaction_atomic {

        if(set0->size()<THRESHOLD)
        {
            pushback(set0, ele);
                
            return true;
        }
            
        else if(set1->size()<THRESHOLD)
        {
            pushback(set1, ele);
                
            return true;
        }
            
        else if(set0->size()<PROBE_SIZE)
        {
            pushback(set0, ele);
            i = 0;
            h = h1;
        }
            
        else if(set1->size()<PROBE_SIZE)
        {
            pushback(set1, ele);
            i = 1;
            h = h1;
        }
            
        else
        {
            resize();
            return add(ele);
        }

        }
        if(!relocate(i, h))
            {
            resize();
        }
            
         
            
            
             
        
        return true;
    }
    
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
     only when threshold and PORBE_SIZE is not very big,
     relocate will work
     */
    __attribute__((transaction_pure))
    bool relocate(long i, long hi) {
        
        //cout<<"transactional relocate working\n";
        long hj = 0;
        long j = 1 - i;
        int LIMIT = 500;
        
        for(int round = 0; round<LIMIT; round++) {

            

            vector<T> *iSet;
            if(i==0) { iSet = &table0[hi]; }
            else { iSet = &table1[hi]; }
            
            if (iSet == nullptr || iSet->size()==0)
                { return false; }
            
            T y = iSet->front();
            if(i==0) { hj = hash1(y); }
            else { hj = hash0(y); }
            
            __transaction_atomic {
                
            vector<T> *jSet;
            if(j==0) { jSet = &table0[hj]; }
            else { jSet = &table1[hj]; }
                
            if(vector_contains(*iSet, y))
            {
                iSet->erase(iSet->begin());
                if(jSet->size()<THRESHOLD)
                {
                    pushback(jSet, y);
                    return true;
                }
                    
                else if(jSet->size()<PROBE_SIZE)
                {
                    pushback(jSet, y);
                    i = 1 - i;
                    j = 1 - j;
                    hi = hj;
                }
                else
                {
                    pushback(iSet, y);;
                        
                    return false;
                }
            }
            else if(iSet->size()>=THRESHOLD)
            {
                continue;
            }
            else
            {
                    
                return true;
            }
                
            }//trnas
            
            
        }
        return false;
    }
    
};
    
    
#endif /* transactional_hashset_h */
