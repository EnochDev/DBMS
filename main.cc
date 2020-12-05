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
      cout<<file_path;
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
      //hash.show();
      cout<<"------------------------"<<endl;
   }
   return 0;
}