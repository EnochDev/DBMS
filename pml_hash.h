#ifndef PML_HASH_H
#define PML_HASH_H

#include <libpmem.h>
#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include <memory.h>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include <time.h>

#define TABLE_SIZE 16 // adjustable
#define HASH_SIZE  30000 // adjustable
#define FILE_SIZE 1024 * 1024 * 16 // 16 MB adjustable

using namespace std;

typedef struct metadata {
    size_t size;            // the size of whole hash table array 
    size_t level;           // level of hash
    uint64_t next;          // the index of the next split hash table
    uint64_t overflow_num;  // amount of overflow hash tables 
} metadata;

// data entry of hash table
typedef struct entry {
    uint64_t key;
    uint64_t value;
} entry;

// hash table
typedef struct pm_table {
    entry kv_arr[TABLE_SIZE];   // data entry array of hash table
    uint64_t fill_num;          // amount of occupied slots in kv_arr
    uint64_t next_offset;       // the file address of overflow hash table 
} pm_table;

// persistent memory linear hash
class PMLHash {
public:
    int is_pmem;
    size_t mapped_len;
    void* start_addr;      // the start address of mapped file
    void* overflow_addr;   // the start address of overflow table array
    metadata* meta;        // virtual address of metadata
    pm_table* table_addr;   // virtual address of hash table array
 //新加入变量   
    size_t N;//2^level
    size_t meta_size=sizeof(metadata);
    size_t pm_table_size=sizeof(pm_table);
    size_t entry_size=sizeof(entry);
//加分项变量
    bool * table_has_or_not_addr;    //溢出页面是否使用，true表示正在使用,false表示未使用

    void split();
    void merge();
    uint64_t hashFunc(const uint64_t &key, const size_t &hash_size);
    pm_table* newOverflowTable(uint64_t &offset);
//追加
    void inside_insert(pm_table*new_table,uint64_t key);
    
public:
    PMLHash() = delete;
    PMLHash(const char* file_path);
    ~PMLHash();

    int insert(const uint64_t &key, const uint64_t &value);
    int search(const uint64_t &key, uint64_t &value);
    int remove(const uint64_t &key);
    int update(const uint64_t &key, const uint64_t &value);

    void show();
};

#endif