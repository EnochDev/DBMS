#include "pml_hash.h"

 int main() {
    PMLHash hash("/mnt/pmemdir/map_file");
    hash.insert(6284781860667377211,6284781860667377211);
    hash.insert(8517097267634966620,8517097267634966620);
    hash.insert(1820151046732198393,1820151046732198393);
    hash.remove(8517097267634966620);
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
    cout<<"Done"<<endl;
    return 0;
 }