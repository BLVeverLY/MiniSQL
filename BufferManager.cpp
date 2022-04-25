#include "BufferManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <queue>
using namespace std;

void BufferManager::clear_file(fileNode &file)
{
    file.nextFile = NULL;
    file.preFile = NULL;
    file.blockHead = NULL;
    file.pin = false;
	file.fileName.clear();
}

void BufferManager::clear_block(blockNode &block)
{
    memset(block.address,0,BLOCK_SIZE);
    int init_usage = 0;
    memcpy(block.address, (char*)&init_usage, sizeof(int));
    block.using_size = sizeof(int);
    block.dirty = false;
    block.nextBlock = NULL;
    block.preBlock = NULL;
    block.offsetNum = -1;
    block.pin = false;
    block.reference = false;
    block.ifbottom = false;
	block.fileName.clear();
}


fileNode* BufferManager::getFile(string fileName, bool if_pin)
{
    blockNode * btmp = NULL;
    fileNode * ftmp = NULL;
    if(fileHead != NULL)
    {
        for(ftmp = fileHead;ftmp != NULL;ftmp = ftmp->nextFile)
        {
            if(!fileName.compare(ftmp->fileName)) 
            {
                ftmp->pin = if_pin;
                return ftmp;
            }
        }
    }

    if(file_num == 0) 
    {
        ftmp = &file_pool[file_num];
        file_num ++;
        fileHead = ftmp;
    }
    else if(file_num < MAX_FILE_NUM)
    {
        ftmp = &file_pool[file_num];
        file_pool[file_num-1].nextFile = ftmp;
        ftmp->preFile = &file_pool[file_num-1];
        file_num ++;
    }
    else
    {
        ftmp = fileHead;
        while(ftmp->pin)
        {
            if(ftmp -> nextFile)ftmp = ftmp->nextFile;
            else
            {
                cout<<"There are no enough file node in the pool!";
                exit(2);
            }
        }
        for(btmp = ftmp->blockHead;btmp != NULL;btmp = btmp->nextBlock)
        {
            if(btmp->preBlock)
            {
                clear_block(*(btmp->preBlock));
                block_num --;
            }
            writtenBackToDisk(btmp->fileName,btmp);
        }
        clear_file(*ftmp);
    }
	ftmp->fileName = fileName;
    set_pin(*ftmp, if_pin);
    return ftmp;
}

blockNode* BufferManager::getBlock(fileNode * file,blockNode *position, bool if_pin)
{
    string fileName = file->fileName;
    blockNode * btmp = NULL;
    if(block_num == 0)
    {
        btmp = &block_pool[0];
        block_num ++;
    }
    else if(block_num < MAX_BLOCK_NUM)
    {
        for(int i = 0 ;i < MAX_BLOCK_NUM;i ++)
        {
            if(block_pool[i].offsetNum == -1)
            {
                btmp = &block_pool[i];
                block_num ++;
                break;
            }
            else
                continue;
        }
    }
    else
    {
        int i = replaced_block;
        while (true)
        {
            i ++;
            if(i >= block_num) i = 0;
            if(!block_pool[i].pin)
            {
                if(block_pool[i].reference == true)
                    block_pool[i].reference = false;
                else 
                {
                    btmp = &block_pool[i];
                    if(btmp->nextBlock) btmp -> nextBlock -> preBlock = btmp -> preBlock;
                    if(btmp->preBlock) btmp -> preBlock -> nextBlock = btmp -> nextBlock;
                    if(file->blockHead == btmp) file->blockHead = btmp->nextBlock;
                    replaced_block = i;
                    
                    writtenBackToDisk(btmp->fileName, btmp);
                    clear_block(*btmp);
                    break;
                }
            }
            else
                continue;
        }
    }
    if(position != NULL && position->nextBlock == NULL)
    {
        btmp -> preBlock = position;
        position -> nextBlock = btmp;
        btmp -> offsetNum = position -> offsetNum + 1;
    }
    else if (position !=NULL && position->nextBlock != NULL)
    {
        btmp->preBlock=position;
        btmp->nextBlock=position->nextBlock;
        position->nextBlock->preBlock=btmp;
        position->nextBlock=btmp;
        btmp -> offsetNum = position -> offsetNum + 1;
    }
    else
    {
        btmp -> offsetNum = 0;
        if(file->blockHead)
        {
            file->blockHead -> preBlock = btmp;
            btmp->nextBlock = file->blockHead;
        }
        file->blockHead = btmp;
    }
    set_pin(*btmp, if_pin);
	btmp->fileName = fileName;
    FILE * fileHandle;
    if((fileHandle = fopen(fileName.c_str(), "ab+")) != NULL)
    {
        if(fseek(fileHandle, btmp->offsetNum*BLOCK_SIZE, 0) == 0)
        {
            if(fread(btmp->address, 1, BLOCK_SIZE, fileHandle)==0)
                btmp->ifbottom = true;
            btmp ->using_size = getUsingSize(btmp);
        }
        else
        {
			cout << "Problem seeking the file " << fileName << " in reading";
            exit(1);
        }
        fclose(fileHandle);
    }
    else
    {
		cout << "Problem opening the file " << fileName << " in reading";
        exit(1);
    }
    return btmp;
}

void BufferManager::writtenBackToDisk(string fileName,blockNode* block)
{
    if(!block->dirty)
    {
        return;
    }
    else
    {
        FILE * fileHandle = NULL;
        if((fileHandle = fopen(fileName.c_str(), "rb+")) != NULL)
        {
            if(fseek(fileHandle, block->offsetNum*BLOCK_SIZE, 0) == 0)
            {
                if(fwrite(block->address, block->using_size+sizeof(int), 1, fileHandle) != 1)
                {
					cout << "Problem writing the file " << fileName << " in writtenBackToDisking";
                    exit(1);
                }
            }
            else
            {
				cout << "Problem seeking the file " << fileName << " in writtenBackToDisking";
                exit(1);
            }
            fclose(fileHandle);
        }
        else
        {
			cout << "Problem opening the file " << fileName << " in writtenBackToDisking";
            exit(1);
        }
    }
}

void BufferManager::writtenBackToDiskAll()
{
    blockNode *btmp = NULL;
    fileNode *ftmp = NULL;
    if(fileHead)
    {
        for(ftmp = fileHead;ftmp != NULL;ftmp = ftmp ->nextFile)
        {
            if(ftmp->blockHead)
            {
                for(btmp = ftmp->blockHead;btmp != NULL;btmp = btmp->nextBlock)
                {
                    if(btmp->preBlock)clear_block(*(btmp -> preBlock));
                    writtenBackToDisk(btmp->fileName, btmp);
                }
            }
        }
    }
}

blockNode* BufferManager::getNextBlock(fileNode* file,blockNode* block)
{
    if(block->nextBlock == NULL)
    {
        if(block->ifbottom) block->ifbottom = false;
        return getBlock(file, block);
    }
    else
    {
        if(block->offsetNum == block->nextBlock->offsetNum - 1)
        {
            return block->nextBlock;
        }
        else
        {
            return getBlock(file, block);
        }
    }
}


blockNode* BufferManager::getBlockHead(fileNode* file)
{
    blockNode* btmp = NULL;
    if(file->blockHead != NULL)
    {
        if(file->blockHead->offsetNum == 0)
        {
            btmp = file->blockHead;
        }
        else
        {
            btmp = getBlock(file, NULL);
        }
    }
    else
    {
        btmp = getBlock(file,NULL);
    }
    return btmp;
}


blockNode* BufferManager::getBlockByOffset(fileNode* file, int offsetNumber)
{
    blockNode* btmp = NULL;
    if(offsetNumber == 0) return getBlockHead(file);
    else
    {
        btmp = getBlockHead(file);
        while(offsetNumber > 0)
        {
            btmp = getNextBlock(file, btmp);
			offsetNumber--;
        }
        return btmp;
    }
}

void BufferManager::delete_fileNode(string fileName)
{
    fileNode* ftmp = getFile(fileName);
    blockNode* btmp = getBlockHead(ftmp);
    queue<blockNode*> blockQ;
    while (true) {
        if(btmp == NULL) return;
        blockQ.push(btmp);
        if(btmp->ifbottom) break;
        btmp = getNextBlock(ftmp,btmp);
    }
    block_num -= blockQ.size();
    while(!blockQ.empty())
    {
        clear_block(*blockQ.back());
        blockQ.pop();
    }
    if(ftmp->preFile) ftmp->preFile->nextFile = ftmp->nextFile;
    if(ftmp->nextFile) ftmp->nextFile->preFile = ftmp->preFile;
    if(fileHead == ftmp) fileHead = ftmp->nextFile;
    clear_file(*ftmp);
    file_num --;
}


void BufferManager::set_pin(blockNode &block,bool pin)
{
    block.pin = pin;
    if(!pin)
        block.reference = true;
}

void BufferManager::set_pin(fileNode &file,bool pin)
{
    file.pin = pin;
}


int BufferManager::getUsingSize(blockNode* block)
{
    return *(int*)block->address;
}

void BufferManager::set_usingSize(blockNode & block,int usage)
{
    block.using_size = usage;
    memcpy(block.address,(char*)&usage,sizeof(int));
}

int BufferManager::get_usingSize(blockNode & block)
{
    return block.using_size;
}


char* BufferManager::get_content(blockNode& block)
{
    return block.address + sizeof(int);
}





