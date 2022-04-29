#include <iostream>
#include <leveldb/db.h>
#include <cassert>
#include <fstream>
#include <vector>
#include <regex>
using namespace std;
using namespace leveldb;

int main(){
    DB * db;
    Options options;
    options.create_if_missing = true;
    Status status = DB::Open(options, "/tmp/grade", &db);
    assert(status.ok());
    ifstream in1("input1");
    ifstream in2("input2");
    ofstream out("output");

    string line1;
    string line2;
    //方法一：先将in1的数据全部读入，并设置一个snapshot，然后再读入in2的内容，放入数据库，接着删除snapshot
    //while(getline(in1, line1) && getline(in2, line2)){
       	//regex ws_re("\\s+");
	//vector<string> v1(sregex_token_iterator(line1.begin(), line1.end(), ws_re, -1), sregex_token_iterator());
	//status = db->Put(WriteOptions(), v1[0], v1[1]);
	//assert(status.ok());
	//建立一个snapshot并将数据读入
	//ReadOptions read_options;
	//read_options.snapshot = db->GetSnapshot();
	//vector<string> v2(sregex_token_iterator(line2.begin(), line2.end(), ws_re, -1), sregex_token_iterator());
	//status = db->Put(WriteOptions(), v2[0], v2[1]);
	//assert(status.ok());
	//string grade;
	//status = db->Get(read_options, v1[0], &grade);
	//assert(status.ok());
	//out << v1[0] << " " << grade << " ";
	//db->ReleaseSnapshot(read_options.snapshot);
        //db->Get(ReadOptions(),v2[0], &grade);
	//out << grade << endl;
    //}
    //方法二：应该先将input1全部写入到数据中，接着在加一个snapshot，再将第二次开始成绩写入，然后两个两个迭代器分别读取
    while(getline(in1, line1)){
        regex ws_re("\\s+");
	vector<string> v(sregex_token_iterator(line1.begin(), line1.end(), ws_re, -1), sregex_token_iterator());
	db->Put(WriteOptions(), v[0], v[1]);
	assert(status.ok());
    }
    ReadOptions read_options;
    read_options.snapshot = db->GetSnapshot();
    while(getline(in2, line2)){
        regex ws_re("\\s+");
        vector<string> v(sregex_token_iterator(line2.begin(), line2.end(), ws_re, -1), sregex_token_iterator());
        db->Put(WriteOptions(), v[0], v[1]);
        assert(status.ok());
    }
    
    Iterator* it1 = db->NewIterator(read_options);
    Iterator* it2 = db->NewIterator(ReadOptions());

    for(it1->SeekToFirst(), it2->SeekToFirst(); it1->Valid(), it2->Valid(); it1->Next(), it2->Next()){
        out << it1->key().ToString() << " " << it1->value().ToString()  << " "<< it2->value().ToString() << endl;
    }
    delete it2;
    delete it1; 
    db->ReleaseSnapshot(read_options.snapshot);
    delete db;
    return 0;
}
