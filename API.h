#pragma once

#include "Attribute.h"
#include "Condition.h"
#include "IndexManager.h"
#include <string>
#include <cstring>
#include <vector>
#include <stdio.h>

class CatalogManager;
class RecordManager;
class IndexManager;
class API{
public:

    RecordManager *rm;
    CatalogManager *cm;
    IndexManager *im;

	API() {}
	~API(){}
	/////////////////////////////////////////////////////////////////////////////////////////////////
    //for table
    void DropTable(string tableName);
    void CreateTable(string tableName, vector<Attribute>* attributeVector, string primaryKeyName,int primaryKeyLocation);

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//for records
	void Select(string tableName,  vector<string>* attributeNameVector = NULL, vector<Condition>* conditionVector = NULL);
	void Insert(string tableName,vector<string>* recordContent);
	void Delete(string tableName, vector<Condition>* conditionVector = NULL);

    /////////////////////////////////////////////////////////////////////////////////////////////////
	//for index
	void CreateIndex(string indexName, string tableName, string attributeName);
	void DropIndex(string indexName);
    void InsertIndex(string indexName, char* value, int type, int blockOffset);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�������
	int recordSizeGet(string tableName);//rm����cm
	void allIndexAddressInfoGet(vector<IndexInfo> *indexNameVector);//im����cm
	int typeSizeGet(int type);//rm����cm
	int attributeGet(string tableName, vector<Attribute>* attributeVector);//rm����cm
	void recordIndexDelete(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, int blockOffset);//rm����im

};