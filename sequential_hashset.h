//
//  sequential_hashset.h
//  hash set
//
//  Created by Zhaohan Xi on 17/11/18.
//  Copyright © 2017 Zhaohan Xi. All rights reserved.
//

#ifndef sequential_hashset_h
#define sequential_hashset_h

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;


/////////////////////////////////////////////////////////////////////////


template <typename T>
class Sequential_hashset {
    
public:
    vector<T> *table0;
    vector<T> *table1;
    long eachTableSize;
    
public:
    
    /////////////////////////////////////////////////////////////////////////
    /*
     constructor:
     (1)initialize two hash table, which are Element<T> array,
     (2)alse store the size of each hash table
     
     */
    Sequential_hashset(long capacity) {
        table0 = new vector<T>[capacity];
        table1 = new vector<T>[capacity];
        
        eachTableSize = capacity;
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
     resize:
     double the size of each table, and re-add all original element
     to the new tables one by one
     */
    void resize() {
        
        //cout<<"sequential resize is working"<<endl;
       
        long oldCapacity = eachTableSize;
    
        
        vector<T> *table2 = table0;
        vector<T> *table3 = table1;
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
    
    
    /////////////////////////////////////////////////////////////////////////
    /*
     hash0 & hash1
     two hash functions, one for table1, one for table2
     */
    long hash0(T ele) {
        std::hash<T> h0;
        long res =h0(ele)%eachTableSize;
        return res<0? -res: res;
        
    }
    
    long hash1(T ele) {
        std::hash<T> h1;
        long res =(h1(ele)+19238534)%eachTableSize;
        return res<0? -res: res;
        
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
        
        if(!contains(ele))
            return false;
        
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
    
    
    /////////////////////////////////////////////////////////////////////////
    
    bool add(T ele) {
        bool isContains = contains(ele);
        //cout<<"You now adding an element ele = "<<ele<<" Do it contains ? "<<isContains<<endl;
        
        if(isContains)
        {
            //cout<<"This element is already here : "<<ele<<endl;
            return false;
        }
        
        /*
         in each loop of add, totally 3 situation of add
         (1) pos0 is empty - add ele into pos0
         (2) pos0 is not empty but pos1 is empty
         - add new ele into pos0 and push old pos0
         into pos1
         (3) both pos0 and pos1 is not empty - add new ele
         into pos0 and push old into pos1, and set
         ele = old pos1, go to next loop
         */
        int LIMIT = 500; //maximum limited num of iterations
        for(int i=0; i<LIMIT; i++) {
            
            long pos0 = hash0(ele);
            
            // 1st situation
            if(table0[pos0].empty())
            {
                table0[pos0].push_back(ele);
                return true;
            }
            else
            {
                long pos1 = hash1(table0[pos0].at(0));
                
                // 2nd situation
                if(table1[pos1].empty())
                {
                    table1[pos1].push_back(table0[pos0].at(0));
                    table0[pos0].erase(table0[pos0].begin());
                    table0[pos0].push_back(ele);
                    return true;
                }
                
                //3rd situation
                else
                {
                    T temp = table1[pos1].at(0);
                    table1[pos1].erase(table1[pos1].begin());
                    table1[pos1].push_back(table0[pos0].at(0));
                    table0[pos0].erase(table0[pos0].begin());
                    table0[pos0].push_back(ele);
                    ele = temp;
                }
            }
            
        }
        resize();
        return add(ele);
        
    }
    
    
};


#endif /* sequential_hashset_h */

