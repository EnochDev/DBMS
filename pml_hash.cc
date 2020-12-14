#include "pml_hash.h"
/**
 * PMLHash::PMLHash 
 * 
 * @param  {char*} file_path : the file path of data file
 * if the data file exist, open it and recover the hash
 * if the data file does not exist, create it and initial the hash
 */
PMLHash::PMLHash(const char* file_path) {
    //获取初始化内存的起始地址
    if((start_addr=pmem_map_file(file_path,FILE_SIZE,PMEM_FILE_CREATE,0666,&mapped_len,&is_pmem))==NULL){
        cout<<"faild to map file."<<endl;
    }

    //初始化meta
    meta = (metadata*)start_addr;
    meta->level=1;
    meta->next=0;
    meta->overflow_num=0;
    meta->size=2;
    N=(int)pow(2,meta->level);

    //初始哈希表为两个桶
    table_addr=(pm_table*)((metadata*)start_addr+1);
    table_addr->fill_num=0;
    table_addr->next_offset=0;

    pm_table*table_addr2=table_addr+1;
    table_addr2->fill_num=0;
    table_addr2->next_offset=0;

    //初始化溢出页面的起始地址
    overflow_addr=(pm_table*)((char*)start_addr+FILE_SIZE/2);

    //建立bitmap用以实现溢出空间回收
    table_has_or_not_addr = (bool*)((pm_table*)overflow_addr+HASH_SIZE);
    for(int i = 0;i < HASH_SIZE;++i){
        table_has_or_not_addr[i] = false;
    }
}
/**
 * PMLHash::~PMLHash 
 * 
 * unmap and close the data file
 */
PMLHash::~PMLHash() {
    //将已修改内容同步到持久化内存上
    pmem_persist(start_addr, FILE_SIZE);
    pmem_unmap(start_addr, FILE_SIZE);
}
/**
 * PMLHash 
 * 
 * split the hash table indexed by the meta->next
 * update the metadata
 */
void PMLHash::split() { 
    //判断桶是否使用完毕
    if(meta->size == HASH_SIZE){
        cout << "桶已被使用完" << endl;
        exit(0);
    }
    
    //变量声明
    pm_table*new_table=table_addr+meta->size;
    new_table->fill_num=0;
    new_table->next_offset=0;
    pm_table * pre_table = table_addr+meta->next;
    pm_table * temp_table=pre_table;
    uint64_t i=0,j=0,off_set_recorder=0;

    //将next指向的桶进行分裂
    uint64_t newN=N*2;
    off_set_recorder=temp_table->next_offset;
    while(i<(pre_table->fill_num)||pre_table->next_offset!=0){
        if(((pre_table->kv_arr[i].key)%newN)!=meta->size){
            if(j==TABLE_SIZE){
                temp_table=(pm_table*)overflow_addr+off_set_recorder-1;
                off_set_recorder=temp_table->next_offset;
                j=0;
            }
            temp_table->kv_arr[j].key=pre_table->kv_arr[i].key;
            temp_table->kv_arr[j].value=pre_table->kv_arr[i].value;
            j++;
        }else{
            inside_insert(new_table,pre_table->kv_arr[i].key);
        }
        i++;
        if(i == TABLE_SIZE && pre_table->next_offset != 0){
            pre_table = (pm_table *)overflow_addr+pre_table->next_offset-1;
            i = 0;
        }
    }

    //回收处理
    pm_table * tmp = temp_table;
    while(tmp->next_offset != 0){
        off_set_recorder = tmp->next_offset;
        table_has_or_not_addr[off_set_recorder-1] = false;
        meta->overflow_num -= 1;
        tmp = (pm_table*)overflow_addr+off_set_recorder-1;
    }
    temp_table->next_offset=0;
    temp_table->fill_num=j;

    //更新meta
    meta->size+=1;
    meta->next+=1;
    if(meta->next==N){
        meta->level+=1;
        meta->next=0;
        N*=2;
    }
}
/**
 * PMLHash 
 * 
 * merge the hash table indexed by the meta->next
 * update the metadata
 */
void PMLHash::merge() {
    //只剩两个桶，不需要合并
    if(meta->size == 2){
        return;
    }

    //更新next指针
    if(meta->next != 0){
        meta->next -= 1;
    }
    else{
        N /= 2;
        meta->next = N-1;
    }meta->size -= 1;

    //分别指向要合并的两个桶
    pm_table*idx_table1 = table_addr+meta->next;
    pm_table*idx_table2 = idx_table1+N;

    //遍历到第一个桶的末尾
    while(idx_table1->next_offset != 0){
        idx_table1 = (pm_table *)overflow_addr+idx_table1->next_offset-1;
    }

    //将第二个桶的元素依顺序写入第一个桶的末尾，以实现合并
    uint64_t i = 0;uint64_t j = idx_table1->fill_num;
    while(i < idx_table2->fill_num || idx_table2->next_offset != 0){
        if(j == TABLE_SIZE){
            idx_table1 = newOverflowTable(idx_table1->next_offset);
            j = 0;
        }
        idx_table1->kv_arr[j].key = idx_table2->kv_arr[i].key;
        idx_table1->kv_arr[j].value = idx_table2->kv_arr[i].value;
        idx_table1->fill_num += 1;
        ++i;++j;
        if(i == TABLE_SIZE && idx_table2->next_offset != 0){
            //回收处理
            table_has_or_not_addr[idx_table2->next_offset-1] = false;
            meta->overflow_num -= 1;
            idx_table2 = (pm_table *)overflow_addr+idx_table2->next_offset-1;
            i = 0;
        }
    }

}
/**
 * PMLHash 
 * 
 * @param  {uint64_t} key     : key
 * @param  {size_t} hash_size : the N in hash func: idx = hash % N
 * @return {uint64_t}         : index of hash table array
 * 
 * need to hash the key with proper hash function first
 * then calculate the index by N module
 */
uint64_t PMLHash::hashFunc(const uint64_t &key, const size_t &hash_size) {
    uint64_t temp=key%N;
    //判断目标桶是否分裂
    if(temp+N<meta->size)
        //已分裂
        return key%(N*2);
    else
        //未分裂
        return key%N;
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} offset : the file address offset of the overflow hash table
 *                             to the start of the whole file
 * @return {pm_table*}       : the virtual address of new overflow hash table
 */
pm_table* PMLHash::newOverflowTable(uint64_t &offset) {
    uint64_t i = 0;
    
    //循环查找可使用的溢出页面的最小页号
    while(i < HASH_SIZE && table_has_or_not_addr[i]){
        ++i;
    }
    if(i == HASH_SIZE){
        cout << "溢出页面已使用完" << endl;
        exit(0);
    }
    offset = i+1;
    
    //增加新桶并初始化
    pm_table*new_table = (pm_table*)overflow_addr+i;
    table_has_or_not_addr[i] = true;
    new_table->fill_num = 0;
    new_table->next_offset = 0;
    meta->overflow_num += 1;
    return new_table;
}

//插入
void PMLHash::inside_insert(pm_table*new_table,uint64_t key){
    uint64_t num=new_table->fill_num;
    //页面未满
    if(num<TABLE_SIZE){
        entry*entry_addr=new_table->kv_arr;
        entry_addr=entry_addr+num;
        entry_addr->key=key;
        entry_addr->value=key;
        new_table->fill_num+=1;
    }else{
    //页面已满，使用新的溢出页面
        while(new_table->next_offset){
            new_table = (pm_table *)overflow_addr+new_table->next_offset-1;
        }
        if(new_table->fill_num==TABLE_SIZE){
            new_table=newOverflowTable(new_table->next_offset);
        }
        num=new_table->fill_num;
        entry*entry_addr=(entry*)new_table;
        entry_addr=entry_addr+num;
        entry_addr->key=key;
        entry_addr->value=key;
        new_table->fill_num+=1;
    }
}
/**
 * PMLHash 
 * 
 * @param  {uint64_t} key   : inserted key
 * @param  {uint64_t} value : inserted value
 * @return {int}            : success: 0. fail: -1
 * 
 * insert the new kv pair in the hash
 * 
 * always insert the entry in the first empty slot
 * 
 * if the hash table is full then split is triggered
 */
int PMLHash::insert(const uint64_t &key, const uint64_t &value) {
    uint64_t idx=hashFunc(key,N);
    pm_table*idx_table=table_addr+idx;
    uint64_t num=idx_table->fill_num;
    uint64_t val;
    
    //判断键是否已存在
    if(!search(key,val)){
        return 0;
    }
    
    //页面未满
    if(num<TABLE_SIZE){
        entry*entry_addr=idx_table->kv_arr;
        entry_addr=entry_addr+num;
        entry_addr->key=key;
        entry_addr->value=value;
        idx_table->fill_num+=1;
    }else{
        //页面已满并触发分裂
        while(idx_table->next_offset){
            idx_table = (pm_table *)overflow_addr+idx_table->next_offset-1;
        }
        if(idx_table->fill_num==TABLE_SIZE){
            idx_table=newOverflowTable(idx_table->next_offset);
        }
        num=idx_table->fill_num;
        entry*entry_addr=(entry*)idx_table;
        entry_addr=entry_addr+num;
        entry_addr->key=key;
        entry_addr->value=value;
        idx_table->fill_num+=1; 
        //分裂
        split(); 
    }
    return 0;
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} key   : the searched key
 * @param  {uint64_t} value : return value if found
 * @return {int}            : 0 found, -1 not found
 * 
 * search the target entry and return the value
 */
int PMLHash::search(const uint64_t &key, uint64_t &value) {
    uint64_t idx=hashFunc(key,N);
    pm_table*idx_table=table_addr+idx;//桶地址
    uint64_t i=0;
    //遍历查找
    while(i<(idx_table->fill_num)||idx_table->next_offset!=0)
    {
        
        if(idx_table->kv_arr[i].key == key)
        {
            value = idx_table->kv_arr[i].value;
            return 0;
        }
        i++;
        if(i == TABLE_SIZE && idx_table->next_offset != 0){
            idx_table = (pm_table *)overflow_addr+idx_table->next_offset-1;
            i=0;
        } 
    }
    return -1;
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} key : target key
 * @return {int}          : success: 0. fail: -1
 * 
 * remove the target entry, move entries after forward
 * if the overflow table is empty, remove it from hash
 */
int PMLHash::remove(const uint64_t &key) {
    //根据key找到要删除元素所在的桶
    uint64_t idx=hashFunc(key,N);
    
    pm_table*idx_table=table_addr+idx;
    pm_table*now_table=idx_table; //指向目标键值所在的桶
    pm_table*pre_table=nullptr;   //last_table的前一个桶，用于找到最后一个桶
    pm_table*last_table=idx_table;//桶序号为idx的最后一个桶

    //计算桶里元素个数，同时找到桶里最后一个元素
    uint64_t num=idx_table->fill_num;
    while(last_table->next_offset != 0){
        pre_table = last_table;
        last_table = (pm_table *)overflow_addr+last_table->next_offset-1;
        num += last_table->fill_num;
    }
    
    //用桶里的最后一个元素代替要删除的元素以实现删除
    int ans = -1;
    for(uint64_t i = 0;i < num;++i){
        if(now_table->kv_arr[i%TABLE_SIZE].key == key){
            ans = now_table->kv_arr[i%TABLE_SIZE].value;
            now_table->kv_arr[i%TABLE_SIZE].key = last_table->kv_arr[(num-1)%TABLE_SIZE].key;
            now_table->kv_arr[i%TABLE_SIZE].value = last_table->kv_arr[(num-1)%TABLE_SIZE].value;
            last_table->fill_num -= 1;
            if(last_table->fill_num == 0){
                if(pre_table){
                    //回收处理
                    table_has_or_not_addr[pre_table->next_offset-1] = false;
                    meta->overflow_num -= 1;
                    pre_table->next_offset = 0;
                }else{//当桶被删空的时候触发桶合并
                    merge();
                }
            }break;
        }
        if((i+1)%TABLE_SIZE == 0){
            now_table = (pm_table *)overflow_addr+now_table->next_offset-1;
        }
    }
    return ans;
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} key   : target key
 * @param  {uint64_t} value : new value
 * @return {int}            : success: 0. fail: -1
 * 
 * update an existing entry
 */
int PMLHash::update(const uint64_t &key, const uint64_t &value) {
    uint64_t idx=hashFunc(key,N);
    pm_table*idx_table=table_addr+idx;//桶地址
    uint64_t i=0;
    //遍历查找并更新
    while(i<(idx_table->fill_num)||idx_table->next_offset!=0)
    {
        if(idx_table->kv_arr[i].key == key)
        {
            idx_table->kv_arr[i].value=value;
            return 0;
        }
        i++;
        if(i == TABLE_SIZE && idx_table->next_offset != 0){
            idx_table = (pm_table *)overflow_addr+idx_table->next_offset-1;
            i=0;
        } 
    }
    return -1;
}

void PMLHash::show(){
    //遍历并统计所有元素个数
    pm_table * idx_table;uint64_t j;
    uint64_t cnt = 0;
    for(uint64_t i = 0;i < meta->size;++i){
        idx_table = table_addr+i;j = 0;
        while(j < idx_table->fill_num || idx_table->next_offset != 0){
            ++cnt;++j;
            if(j == TABLE_SIZE && idx_table->next_offset != 0){
                idx_table = (pm_table *)overflow_addr+idx_table->next_offset-1;
                j = 0;
            }
        }
    }
    cout << "共有元素" << cnt << "个" << endl;
}