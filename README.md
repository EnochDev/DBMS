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
PMLHash的所有数据放在一个16MB的文件中存储，结构如下：
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
插入操作用于插入一个新的键值对，首先要找到相应的哈希桶，然后插入。插入的时候永远插入第一个空的槽位，维持桶的元素的连续性。可能原本的哈希桶本身已经满了且有溢出桶，那就插入溢出桶。插入后桶满，就出发分裂操作，分裂的桶被metadata中的next标识。

产生新的溢出桶从数据文件的Overflow Hash Tables区域获取空间，其空闲的起始位置可以通过metadata的overflow_num在启动时计算出来，每获取一个新的溢出桶，就将空间位置的指针往后移动即可，可以支持到8MB的溢出桶空间使用完毕。

### Remove
删除操作用于移除目标键值对，为了位置哈希桶的连续性，采用将移除键值位置后的其他键值向前移动的方式进行覆盖，然后更新桶的fill_num指示桶的最后一个元素位置，达到删除的目的。

删除可能将一个溢出桶清空，直接将桶从溢出链中去除即可。对于空溢出桶的空间回收操作作为加分项进行。

### Update
更新操作修改目标键值对的值，先找到对应的位置。

### Search
查找操作根据给定的键找对应的值然后返回。

## YCSB设计

编写YCSB测试，运行Benchmark数据集并测试OPS(Operations per second)和延迟两个性能指标  

## 加分项

### 溢出桶空间回收
现有的溢出桶空间随着不断分配释放桶，最后会将8MB的空间使用完毕，因为分配目前是永远不回头的。自行确定一种识别空闲溢出桶的策略，不仅仅是依靠一个空闲指针指向未分配过的空闲空间。解决方案有两种，分别是bitmap方式和清空桶内容然后识别其是否是空闲桶。

### 多线程实现 
基于实现的单线程版本，加上相应的锁和加锁策略设计，实现多线程的PML-Hash，并运行YCSB测试，记录相应性能指标。