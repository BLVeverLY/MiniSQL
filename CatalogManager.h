#pragma once

#include <string>
#include <vector>
#include "Attribute.h"
#include "BufferManager.h"
#include "IndexManager.h"
using namespace std;

class CatalogManager {
public:

    BufferManager bm;

    CatalogManager(){}
    ~CatalogManager(){}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//table
	int cm_CreateTable(string tableName, vector<Attribute>* attributeVector, string primaryKeyName, int primaryKeyLocation);
	int cm_DropTable(string tableName);
	int cm_table_file_name(string tableName);
	int tableExist(string tableName);

	/////////////////////////////////////////////////////////////////////////////////////////////
	//record
	int cm_Insert(string tableName, int recordNum);  // increment the number of record
	int cm_Delete(string tableName, int deleteNum);// delete the number of record
	
	////////////////////////////////////////////////////////////////////////////////////////
	//index
	int cm_InsertIndex(string indexName, string tableName, string attributeName, int type);
	int cm_DropIndex(string index);
	int cm_find_index(string indexName);

	int setIndexOnAttribute(string tableName, string AttributeName, string indexName);
	int revokeIndexOnAttribute(string tableName, string AttributeName, string indexName);
	int getAllIndex(vector<IndexInfo> * indexs);//用于重启时读取所有已有index
	int indexNameListGet(string tableName, vector<string>* indexNameVector);//用于删除table的所有index
	int getIndexType(string indexName);
    
    /////////////////////////////////////////////////////////////////////////////////////
    
    int attributeGet(string tableName, vector<Attribute>* attributeVector);
    int calcuteRecordLenth(string tableName);
    int calcuteTypeLenth(int type);
};