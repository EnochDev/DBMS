# 数据库课程设计报告
**[仓库地址](https://github.com/EnochDev/DBMS/)**

## 成员及环境
**小组成员：陈杨平 王宇春 李怡臻**   
**环境：windows10 虚拟机 Ubuntu20.04**

## 报告目录
1. [环境配置](#环境配置)  
1.1 利用普通内存模拟NVM环境并测试是否配置正确  
1.2 根据PMDK的README安装教程进行库安装
2. [数据结构设计](#数据结构设计)   
3. [操作设计](#操作设计)
4. [YCSB测试](#YCSB测试) 
5. [加分项](#加分项)

## 环境配置

### NVM环境模拟  

### PMDK库安装  

## 数据结构设计
PMLHash的所有数据放在一个16MB的文件中存储，在原有基础下，在Overflow Hash Table后加入<font color=red>**Bitmap**</font>，结构如下：
```
// PMLHash
| Metadata | Hash Table Array | Overflow Hash Tables | Bitmap |
+---------- 8 MB -------------+------- 8 MB ------------------+

// Metadata
| size | level | next | overflow_num |

// Hash Table Array
| hash table[0] | hash table[1] | ... | hash table[n] | 

// Overflow Hash Tables
| hash table[0] | hash table[1] | ... | hash table[m] |

// Hash Table
| key | value | fill_num | next_offset |

// Bitmap
| bool[0] | bool[1] | ... | bool[m] |
```

## 操作设计
实现增删查改功能并运行，测试每个功能运行并截图相应结果

### Insert

####  思路
插入操作用于插入一个新的键值对，首先要找到相应的哈希桶，然后插入。插入的时候永远插入第一个空的槽位，维持桶的元素的连续性。若原本的哈希桶已满且有溢出桶，则插入溢出桶。插入后触发**分裂**操作，通过metadata中的next识别要分裂的桶号。

产生新的溢出桶从数据文件的Overflow Hash Tables区域获取空间，通过[桶回收](#溢出桶空间回收)的操作确认目前空闲的第一个溢出桶，并将加metadata中的overflow_num加1，直至溢出桶空间达到8MB。

#### 函数说明 
* int insert(const uint64_t &key, const uint64_t &value);  
为插入的主要函数实现，其中key，value分别为要插入的键和值，成功时返回0，失败时返回-1

* void PMLHash::split();  
为桶分裂时使用的函数，用于创建新的分裂桶，将原桶中的元素拆分到原桶和新的分裂桶

* void PMLHash::inside_insert(pm_table*new_table,uint64_t key);  
用于桶分裂后，将桶内重新计算哈希值的元素插入新的分裂桶

#### 测试代码
```
    PMLHash hash("/mnt/pmemdir/map_file");
    for(int i=0;i<500;i++)
        hash.insert(i,i);
    hash.show();
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;
```
#### 运行结果
![avatar](https://raw.githubusercontent.com/EnochDev/Code/master/002.png)

### Remove

#### 思路
删除操作用于移除目标键值对，总的来说，删操作是增操作的逆操作，先通过哈希和key找到要删除元素所在的桶，然后用桶里最后一个元素代替要删除的元素并去掉最后一个元素即可实现删除。  
去掉最后一个元素会有很多边界条件，这里不一一赘述了，但其中较为重要的几点，在于桶被删空时触发的**桶合并**操作，还有[空溢出桶的回收](#溢出桶空间回收)

#### 桶合并
* 算法原理，将要被合并的桶内的元素放到next指向的桶的后面即可，这里要留意也有[溢出空间的回收](#溢出桶空间回收)

#### 测试代码
```
    PMLHash hash("/mnt/pmemdir/map_file");
    for(int i=0;i<500;i++)
        hash.insert(i,i);
    hash.show();
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;
    for(int i=499;i>=0;i--)
        hash.remove(i);
    cout<<"After remove"<<endl;
    hash.show();
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;
```

#### 运行结果

![avatar](https://raw.githubusercontent.com/EnochDev/Code/master/003.png)

### Update

#### 思路
更新操作修改目标键值对的值，先将键值通过哈希函数得到相应的桶号，然后遍历该桶找到目标键值，最后修改value的值即可，若找不到返回-1 。

#### 测试代码
``` 
    PMLHash hash("/mnt/pmemdir/map_file");
    for(int i=0;i<10;i++){
        hash.insert(i,i);
        cout << i << " " << i <<endl;
    }
    hash.show();
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;
    for(int i=0;i<10;i++)
        hash.update(i,i+100);
    cout<<"After update"<<endl;
    cout<<"------------------------"<<endl;
    uint64_t val = 0;
    for(int i=0;i<10;i++){
        hash.search(i,val);
        cout << i << " " << val << endl;
    }
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;
```

#### 运行结果
```
0 0
1 1
2 2
3 3
4 4
5 5
6 6
7 7
8 8
9 9
共有元素10个
Work Done!
------------------------
After update
------------------------
0 100
1 101
2 102
3 103
4 104
5 105
6 106
7 107
8 108
9 109
Work Done!
------------------------
```

### Search

#### 思路
查找操作返回目标键值对的值，先将键值通过哈希函数得到相应的桶号，然后遍历该桶找到目标键值，最后返回value的值即可，若找不到返回-1 。

#### 测试代码
``` 
    PMLHash hash("/mnt/pmemdir/map_file");
    for(int i=0;i<500;i++){
        hash.insert(i,i);
        cout << i << " " << i <<endl;
    }
    hash.show();
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;
    int val = 0;
    for(int i=490;i<500;i++){
        hash.search(i,val);
        cout << i << " " << val;
    }
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;
```

#### 运行结果
```
共有元素500个
Work Done!
------------------------
searching...
490 490
491 491
492 492
493 493
494 494
495 495
496 496
497 497
498 498
499 499
Work Done!
------------------------
```

## YCSB测试

编写YCSB测试，运行Benchmark数据集并测试OPS(Operations per second)和延迟两个性能指标  
```
./benchmark/10w-rw-0-100-load.txt
./benchmark/10w-rw-0-100-run.txt
桶总数：9336
已使用溢出页面数：660
共有元素109997个
查找命中率：-nan
延时: 0.166998秒
OPS: 658691次每秒
------------------------
./benchmark/10w-rw-100-0-load.txt
./benchmark/10w-rw-100-0-run.txt
桶总数：8558
已使用溢出页面数：256
共有元素99997个
查找命中率：1
延时: 0.155535秒
OPS: 707236次每秒
------------------------
./benchmark/10w-rw-25-75-load.txt
./benchmark/10w-rw-25-75-run.txt
桶总数：9086
已使用溢出页面数：530
共有元素107513个
查找命中率：1
延时: 0.152981秒
OPS: 719044次每秒
------------------------
./benchmark/10w-rw-50-50-load.txt
./benchmark/10w-rw-50-50-run.txt
桶总数：8878
已使用溢出页面数：432
共有元素105099个
查找命中率：1
延时: 0.155417秒
OPS: 707773次每秒
------------------------
./benchmark/10w-rw-75-25-load.txt
./benchmark/10w-rw-75-25-run.txt
桶总数：8689
已使用溢出页面数：332
共有元素102485个
查找命中率：1
延时: 0.153407秒
OPS: 717047次每秒
------------------------
```

## 加分项

### 溢出桶空间回收

#### 思路
现有的溢出桶空间随着不断分配释放桶，最后会将8MB的空间使用完毕，为重复利用溢出页面，采取Bitmap的方式对溢出页面进行回收

#### 实现
1. 在Overflow Hash Table后申请一处空间用于存放Bitmap，使用bool数组实现，其位数与最大溢出页面数相同，每一位对应一溢出页面，初始化全部为真。该位为真时表示对应页面已被使用，为假时表示对应页面未被使用或已被回收。
2. 每次申请新的溢出页面的时候，扫描Bitmap，找到bool值为假且最小的一位，其对应的溢出页面即可作为新的溢出页面。

#### 回收溢出页面的操作
1. 处于分裂操作时，拆分后，原桶部分溢出页面可被回收
```
88    pm_table * tmp = temp_table;
89    while(tmp->next_offset != 0){
90        off_set_recorder = tmp->next_offset;
91        table_has_or_not_addr[off_set_recorder-1] = false;
92        meta->overflow_num -= 1;
93        tmp = (pm_table*)overflow_addr+off_set_recorder-1;
94    }
```
2. 处于删除操作时，溢出桶被清空时，该溢出桶可被回收
```
333    table_has_or_not_addr[pre_table->next_offset-1] = false;
334    meta->overflow_num -= 1;
335    pre_table->next_offset = 0;
```
3. 处于合并操作时，合并后，被合并桶的溢出页面可被回收
```
148    table_has_or_not_addr[idx_table2->next_offset-1] = false;
149    meta->overflow_num -= 1;
```
#### 对比

||测试集一|测试集二|测试集三|测试集四|测试集五|
|:----:|:----:|:----:|:----:|:----:|:----:|
|溢出页面使用数(未回收)|3711|3256|3565|3448|3337|
|溢出页面使用数(已回收)|660|256|530|432|332|
|比值|17.8%|7.9%|14.9%|12.5%|9.9%|

### 多线程实现的一些尝试
并行库：<pthread.h>  

#### 思路  
采用共享内存的方式利用pthread进行并行化处理，对增删查改四种事务采取对应的加锁策略，尝试实现并行化
#### 尝试
1. 将增删查改四种事务分为两类，增删改对应**写**操作，查对应**读**操作，写与写，读与写存在冲突，读与读不存在冲突，基于此，为每个桶设置一把读写锁。  
结果：发现桶的分裂操作中，需要同时对三个桶（触发分裂的桶，next指向的桶，分裂出的新桶）进行操作，存在多种数据依赖，读写锁不足以完成并发操作，合并操作同理。
2. 在读写锁的基础上，为解决数据依赖，引入新的全局锁  
结果：概率引发死锁现象  
举例：当一个线程A的桶处于分裂操作时，next指向的桶在其他线程B中插入数据也触发分裂，会导致A需要B拥有的next指向桶的使用权，而B需要A拥有的全局锁的使用权，触发死锁
