#include "pml_hash.h"

void GetFiles(string path,vector<string>& filenames)
{
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(path.c_str()))){
        cout<<"Files doesn't Exist!"<<endl;
        return;
    }
    while((ptr = readdir(pDir))!=0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
            filenames.push_back(path + "/" + ptr->d_name);
    }
    }
    closedir(pDir);
    sort(filenames.begin(),filenames.end());
}

int main() {
    clock_t start_time;
    clock_t finish_time;
    
    vector<string>files;
    vector<string>::iterator iter;
    uint64_t search_cnt = 0;
    uint64_t search_all = 0;
    GetFiles("./benchmark",files);
    uint64_t i = 0;uint64_t n = files.size();
    
    //for(iter=files.begin();iter!=files.end();iter+= 2){
    while(i < n){
        start_time = clock();
        PMLHash hash("/mnt/pmemdir/map_file");
        search_cnt = 0;search_all = 0;
        for(int j=0;j<2;j++){
            string file_path=files[i+j];
            string operation="",number="",line="";
            uint64_t num=0;
            ifstream ifile(file_path);
            cout<<file_path << endl;
            while(getline(ifile,line)){
                stringstream input(line);
                input>>operation>>number;
                number=number.substr(0,8);
                stringstream input2(number);
                input2>>num;
                //cout<<num<<" "<<operation<<endl;
                if(operation=="INSERT")
                {
                    hash.insert(num,num);
                }
                else if(operation=="READ")
                {
                    if(!hash.search(num,num))
                        search_cnt++;
                    search_all++;
                }
            }ifile.close();
        }
        
        cout<<"桶总数："<<hash.meta->size<<endl;
        cout<<"已使用溢出页面数："<<hash.meta->overflow_num<<endl;
        hash.show();
        cout<<"查找命中率："<< 1.0*search_cnt/search_all <<endl;
        finish_time = clock();
        cout << "延时: " << 1.0*(finish_time-start_time)/CLOCKS_PER_SEC << "秒"<< endl;
        cout << "OPS: " << 110000/(1.0*(finish_time-start_time)/CLOCKS_PER_SEC) << "次每秒" << endl;
        cout<<"------------------------"<<endl;
        i += 2;
    }
    
    return 0;
}