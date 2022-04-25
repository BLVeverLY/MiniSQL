#pragma once

#include <stdio.h>
#include <map>
#include <string>
#include <sstream>
#include "Attribute.h"
#include "BPlusTree.h"

class IndexInfo
{
public:
	IndexInfo(string i, string t, string a, int ty)
	{
		indexName = i;tableName = t;Attribute = a;type = ty;
	}

	string indexName;
	string tableName;
	string Attribute;
	int type;
};

class API;

class IndexManager{
public:

	IndexManager(API *api);
	~IndexManager();

	void createIndex(string filePath, int type);
	void DropIndex(string filePath, int type);
	int searchIndex(string filePath, string key, int type);
	void insertIndex(string filePath, string key, int blockOffset, int type);
	void deleteIndexByKey(string filePath, string key, int type);

private:

	API *api;

    typedef map<string,BPlusTree<int> *> intMap;
    typedef map<string,BPlusTree<string> *> stringMap;
    typedef map<string,BPlusTree<float> *> floatMap;
    
    intMap indexIntMap;
    stringMap indexStringMap;
    floatMap indexFloatMap;

    struct keyTmp{
        int intTmp;
        float floatTmp;
        string stringTmp;
    };
    struct keyTmp kt;
    
    int getDegree(int type);
    int getKeySize(int type);
    void setKey(int type,string key);
};
