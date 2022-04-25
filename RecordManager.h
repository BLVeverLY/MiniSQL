#pragma once

#include "Attribute.h"
#include "RecordManager.h"
#include "BufferManager.h"
#include "Condition.h"
#include <string>
#include <vector>
using namespace std;

class API;

class RecordManager{
public:

    API *api;
	BufferManager bm;
    
	RecordManager(){}
	~RecordManager(){}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//table
    int rm_CreateTable(string tableName);
    int rm_DropTable(string tableName);
    
	//////////////////////////////////////////////////////////////////////////////////////////////////
    //record
    int rm_Insert(string tableName, char* record, int recordSize);
    
    int recordAllShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector);
    int recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, int blockOffset);
    
    int recordAllFind(string tableName, vector<Condition>* conditionVector);
    
    int recordAllDelete(string tableName, vector<Condition>* conditionVector);
    int recordBlockDelete(string tableName,  vector<Condition>* conditionVector, int blockOffset);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//index
	int rm_CreateIndex(string indexName);
	int rm_DropIndex(string indexName);
	
    ////////////////////////////////////////////////////////////////////////////////////////////////

    int indexRecordAllAlreadyInsert(string tableName,string indexName);
    
    string tableFileNameGet(string tableName);

private:
    int recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, blockNode* block);
    int recordBlockFind(string tableName, vector<Condition>* conditionVector, blockNode* block);
    int recordBlockDelete(string tableName,  vector<Condition>* conditionVector, blockNode* block);
    int indexRecordBlockAlreadyInsert(string tableName,string indexName, blockNode* block);
    
    bool recordConditionFit(char* recordBegin,int recordSize, vector<Attribute>* attributeVector,vector<Condition>* conditionVector);
    void recordPrint(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, vector<string> *attributeNameVector);
    bool contentConditionFit(char* content, int type, Condition* condition);
    void contentPrint(char * content, int type);
};