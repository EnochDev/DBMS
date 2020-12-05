#include "pml_hash.h"

 int main() {
    PMLHash hash("/mnt/pmemdir/map_file");
    for(uint64_t i=0;i<1000;i++){
        hash.insert(i,i);
    }
    hash.show();
    for(uint64_t i=0;i<1000;i++){
        hash.remove(i);
    }
//     for (uint64_t i = 1; i <= HASH_SIZE * TABLE_SIZE; i++) {
//         hash.insert(i, i);
//     }
//     for (uint64_t i = 1; i <= HASH_SIZE; i++) {
//         uint64_t val;
//         hash.search(i, val);
//         cout << "key: " << i << "\nvalue: " << val << endl;
//     }

//     for (uint64_t i = HASH_SIZE * TABLE_SIZE + 1; 
//          i <= (HASH_SIZE + 1) * TABLE_SIZE; i++) {
//         hash.insert(i, i);
//     }
//     for (uint64_t i = HASH_SIZE * TABLE_SIZE + 1;
//          i <= (HASH_SIZE + 1) * TABLE_SIZE; i++) {
//         uint64_t val;
//         hash.search(i, val);
//         cout << "key: " << i << "\nvalue: " << val << endl;
//     }
    hash.show();
    cout<<"Work Done"<<endl;
    cout << "hello world" << endl;
    return 0;
 }