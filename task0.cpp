/* Task 0: Mini-Hekaton

This file contains an incomplete implementation of a storage backend
for a relation with three attributes (a, b, and c). Each row is
dynamically allocated on the heap. For attribute a the relation has a
hash index which stores pointers to the rows. Rows with the same hash
value are linked using the next field and can only be accessed through
the hash table (to scan all rows one has to iterate through the hash
table). This storage format is used in Hekaton, the main-memory engine
of Microsoft SQL Server 2014.

Complete the constructor, destructor, insert, lookup, and remove
functions (see TODO). You need a C++11 compiler. In total the code
required is less than 50 lines. The main function contains test code. */

#include <cassert>
#include <iostream>  
#include <cstring>  
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct Row {
   /// Attribute a
   uint64_t a;
   /// Attribute b
   uint64_t b;
   /// Attribute c
   uint64_t c;
   /// The next pointer for linking rows that have the same hash value in the hash table
   Row* next;
};

struct Relation {
   /// Number of rows in relation
   uint64_t size;
   /// Size of the hash table, must be a power of two
   uint64_t sizeIndex;
   /// Hash table
   Row** index;

   // Construct a relation
   Relation(uint64_t sizeIndex) : size(0), sizeIndex(sizeIndex) {
      // Check that sizeIndex is a power of two
      assert((sizeIndex&(sizeIndex-1))==0);

      //create Hash-Table with NULL-values
      index = new Row*[sizeIndex];
      for(uint64_t i=0;i<=(sizeIndex-1);i++){
            index[i]=NULL;
      }
   }

   // Destroy relation (free all memory)
   ~Relation() {
      for(uint64_t i=0;i<=(sizeIndex-1);i++){
            delete index[i];
      }
      delete[] index;
   }

   // Insert a new row
   void insert(uint64_t a,uint64_t b,uint64_t c) {
      //the row that will be inserted
      Row* oneRow = new Row;
      oneRow->a=a;
      oneRow->b=b;
      oneRow->c=c;
      oneRow->next=NULL;

      if(index[hash(a)]==NULL){
            //no entry with this hash value so far
            index[hash(a)]=oneRow;
            size++;    
      } else if(index[hash(a)]->next==NULL){
                  //only one entry so far
                  index[hash(a)]->next=oneRow;
                  size++;
            } else {
                  Row* temp = index[hash(a)]->next;
                  while(temp->next!=NULL){
                        temp = temp->next;
                  }
                  //insert as last element in the list
                  temp->next=oneRow;
                  size++;
            }
      
   }

   /// Find a row using the index
   Row* lookup(uint64_t a) {
      if(index[hash(a)]==NULL){
            //no entry found
            return NULL;
      } else {
            if(index[hash(a)]->a == a){
                  //first entry is the searched one
                  return index[hash(a)];      
            } else {
                  Row* temp = index[hash(a)];
                  while(temp->next!=NULL && temp->a!=a){
                        temp=temp->next;
                  }
                  if(temp->a==a){
                        //entry found
                        return temp;
                  } else {
                        //no entry found
                        return NULL;
                  }
            }
      }
   }

   // Remove a row
   void remove(Row* row) {
      if(index[hash(row->a)]!=row){
            Row* temp= index[hash(row->a)];
            while(temp->next->next!=NULL && temp->next!=row){
                  temp=temp->next;
            }   
            if(temp->next==row && temp->next->next!=NULL){
                  //remove entry with successors
                  temp->next=temp->next->next;
                  size--;
            } else if(temp->next==row && temp->next->next==NULL){
                  //remove entry with no successors
                  delete temp->next;
                  temp->next=NULL;
                  size--;
            }
      }
      if(index[hash(row->a)]==row && row->next==NULL){
            //remove first entry with no successors
            index[hash(row->a)]=NULL; 
            size--;
      } else if(index[hash(row->a)]==row && row->next!=NULL){
            //remove first entry with successors
            index[hash(row->a)]=row->next;   
            size--;
      } 
   }

   // Computes index into hash table for attribute value a
   uint64_t hash(uint64_t a) {
      return a&(sizeIndex-1);
   }
};

int main() {
   uint64_t n=2500000;

   Relation R(1ull<<20);

   // Random test data
   vector<Row> v;
   for (uint64_t i=0; i<n; i++)
      v.push_back({i,i/3,i/7,nullptr});   

   {
      random_shuffle(v.begin(),v.end());
      // Insert test data
      auto start=high_resolution_clock::now();
      for (Row& r : v)
         R.insert(r.a,r.b,r.c);
      cout << "insert " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
   }

   {
      random_shuffle(v.begin(),v.end());
      // Lookup rows
      auto start=high_resolution_clock::now();
      for (Row& r : v) {
         Row* r2=R.lookup(r.a);
         assert(r2&&(r2->a==r.a));
      }
      cout << "lookup " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
   }

   {
      auto start=high_resolution_clock::now();
      // Scan all entries and add attribute a
      uint64_t sum=0;
      for (uint64_t i=0; i<R.sizeIndex; i++) {
         Row* r=R.index[i];
         while (r) {
            sum+=r->a;
            r=r->next;
         }
      }
      cout << "scan " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
      assert(sum==((n*(n-1))/2));
   }

   {
      random_shuffle(v.begin(),v.end());
      // Delete all entries
      auto start=high_resolution_clock::now();
      for (Row& r : v) {
         Row* r2=R.lookup(r.a);
         assert(r2);
         R.remove(r2);
         assert(!R.lookup(r.a));
      }
      cout << "remove " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
      // Make sure the table is empty
      for (unsigned i=0; i<R.sizeIndex; i++)
            assert(R.index[i]==nullptr);
   }

   return 0;
}
