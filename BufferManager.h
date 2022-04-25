#pragma once

#include <string>
#include <iostream>
using namespace std;

#define MAX_FILE_NUM 40
#define MAX_BLOCK_NUM 100
#define BLOCK_SIZE 4096

class blockNode
{
public:
	blockNode() { address = new char[BLOCK_SIZE]; memset(address, 0, BLOCK_SIZE); }
	~blockNode() { delete[] address; }

	int offsetNum = -1; 
	bool pin = false; 
	bool ifbottom = false; 
	string fileName = ""; 
	bool dirty = false;
	friend class BufferManager;
	

	char * address;
	blockNode * preBlock = NULL;
	blockNode * nextBlock = NULL;
	bool reference = false; 
	
	int using_size = sizeof(int);

};

class fileNode
{
public:
	string fileName = "";
	bool pin = false;
	blockNode *blockHead = NULL;
	fileNode * nextFile = NULL;
	fileNode * preFile = NULL;
};



static int replaced_block = -1;

class BufferManager
{
public:

	BufferManager() {}
	~BufferManager()
	{
		writtenBackToDiskAll();
		for (int i = 0; i < MAX_FILE_NUM; i++)
		{
			file_pool[i].fileName.clear();
		}
	}

	void delete_fileNode(string fileName);
	fileNode* getFile(string fileName, bool if_pin = false);


	void set_pin(blockNode & block, bool pin);
	void set_pin(fileNode & file, bool pin);

	void set_usingSize(blockNode & block, int usage);
	int get_usingSize(blockNode & block);

	char* get_content(blockNode& block);

	blockNode* getNextBlock(fileNode * file, blockNode* block);
	blockNode* getBlockHead(fileNode* file);
	blockNode* getBlockByOffset(fileNode* file, int offestNumber);

private:
	fileNode *fileHead = NULL;
	fileNode file_pool[MAX_FILE_NUM];
	blockNode block_pool[MAX_BLOCK_NUM];
	int block_num = 0; 
	int file_num = 0;

	void clear_block(blockNode & block);
	void clear_file(fileNode & file);
	blockNode* getBlock(fileNode * file, blockNode* position, bool if_pin = false);
	void writtenBackToDiskAll();
	void writtenBackToDisk(string fileName, blockNode* block);
	int getUsingSize(blockNode* block);
	
};
