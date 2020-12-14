#include "pml_hash.h"

void LoadFiles(string path,vector<string>& files)
{
    DIR *Dir= opendir(path.c_str());
    struct dirent* ptr=readdir(Dir);
    if(!Dir){
        cout<<"文件未找到!\n";
        return;
    }
    while(ptr) {
        if (strcmp(ptr->d_name, "..")&& strcmp(ptr->d_name, ".")){
            files.push_back(path + "/" + ptr->d_name);
        }
        ptr=readdir(Dir);
    }
    closedir(Dir);
    sort(files.begin(),files.end());
}

int main() {
    clock_t start_time;
    clock_t finish_time;

    vector<string>files;
    uint64_t search_cnt = 0;
    uint64_t search_all = 0;

    LoadFiles("./benchmark",files);
    
    uint64_t i = 0;
    uint64_t n = files.size();

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
    int val = 0;
    for(int i=0;i<10;i++){
        hush.search(i,val);
        cout << i << " " << val;
    }
    cout<<"Work Done!"<<endl;
    cout<<"------------------------"<<endl;

    //遍历所有文件
    // while(i < n){
    //     start_time = clock();
    //     PMLHash hash("/mnt/pmemdir/map_file");
    //     search_cnt = 0;search_all = 0;
    //     //load 和 run
    //     for(int j=0;j<2;j++){
    //         string file_path=files[i+j];
    //         string operation="",number="",line="";
    //         uint64_t num=0;
    //         ifstream ifile(file_path);
    //         cout<<file_path << endl;
    //         //按行读取
    //         while(getline(ifile,line)){
    //             stringstream input(line);
    //             input>>operation>>number;
    //             number=number.substr(0,8);
    //             stringstream input2(number);
    //             input2>>num;
    //             if(operation=="INSERT")
    //             {
    //                 hash.insert(num,num);
    //             }
    //             else if(operation=="READ")
    //             {
    //                 if(!hash.search(num,num))
    //                     search_cnt++;
    //                 search_all++;
    //             }
    //             else if(operation=="REMOVE")
    //             {
    //                 hash.remove(num);
    //             }
    //             else if(operation=="READ")
    //             {
    //                 hash.update(num,num);
    //             }
    //         }ifile.close();
    //     }
        
    //     cout<<"桶总数："<<hash.meta->size<<endl;
    //     cout<<"已使用溢出页面数："<<hash.meta->overflow_num<<endl;
    //     hash.show();
    //     cout<<"查找命中率："<< 1.0*search_cnt/search_all <<endl;
    //     finish_time = clock();
    //     cout << "延时: " << 1.0*(finish_time-start_time)/CLOCKS_PER_SEC << "秒"<< endl;
    //     cout << "OPS: " << 110000/(1.0*(finish_time-start_time)/CLOCKS_PER_SEC) << "次每秒" << endl;
    //     cout<<"------------------------"<<endl;
    //     i += 2;
    // }
    
    return 0;
}