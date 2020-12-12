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
    clock_t start_time = clock();
    /*pthread_rwlock_t m_lock;
    pthread_rwlock_init(&m_lock, NULL);
    pthread_rwlock_rdlock(&m_lock);
    pthread_rwlock_unlock(&m_lock);
    pthread_rwlock_wrlock(&m_lock);
    pthread_rwlock_unlock(&m_lock);
    pthread_rwlock_destroy(&m_lock);
    cout << "666" << endl;
    cout << sizeof(pthread_rwlock_t);
    return 0;*/

    PMLHash hash("/mnt/pmemdir/map_file");
    vector<string>files;
    vector<string>::iterator iter;
    GetFiles("./benchmark",files);
    for(iter=files.begin();iter!=files.end();iter++){
    //for(int i=0;i<2;i++){
        string file_path=*iter;
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
                hash.remove(num);
            }
        }
        ifile.close();
        cout<<"桶总数："<<hash.meta->size<<endl;
        cout<<"已使用溢出页面数："<<hash.meta->overflow_num<<endl;
        hash.show();
        cout<<"------------------------"<<endl;
    }
    clock_t finish_time = clock();
    cout << "用时" << 1.0*(finish_time-start_time)/CLOCKS_PER_SEC << "秒"<< endl;
    return 0;
}