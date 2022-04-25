#include "CatalogManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include "Attribute.h"
#define UNKNOWN_FILE 8
#define TABLE_FILE 9
#define INDEX_FILE 10

int CatalogManager::cm_DropTable(string tableName)
{
    bm.delete_fileNode(tableName.c_str());
    if (remove(tableName.c_str()))
    {
        return 0;
    }
    return 1;
}
int CatalogManager::getIndexType(string indexName)
{
    fileNode *ftmp = bm.getFile("Indexs");
    blockNode *btmp = bm.getBlockHead(ftmp);
    if (btmp )
    {
        char* addressBegin;
        addressBegin = bm.get_content(*btmp);
        IndexInfo * i = (IndexInfo *)addressBegin;
        for(int j = 0 ;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
        {
            if((*i).indexName==indexName)
            {
                return i->type;
            }
            i ++;
        }
        return -2;
    }

    return -2;
}

int CatalogManager::getAllIndex(vector<IndexInfo> * indexs)
{
    fileNode *ftmp = bm.getFile("Indexs");
    blockNode *btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        char* addressBegin;
        addressBegin = bm.get_content(*btmp);
        IndexInfo * i = (IndexInfo *)addressBegin;
        for(int j = 0 ;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
        {
            indexs->push_back((*i));
            i ++;
        }
    }

    return 1;
}
int CatalogManager::cm_InsertIndex(string indexName,string tableName,string Attribute,int type)
{
    fileNode *ftmp = bm.getFile("Indexs");
    blockNode *btmp = bm.getBlockHead(ftmp);
    IndexInfo i(indexName,tableName,Attribute,type);
    while (true)
    {
        if (btmp == NULL)
        {
            return 0;
        }
        if (bm.get_usingSize(*btmp) <= BLOCK_SIZE - sizeof(int) - sizeof(IndexInfo))
        {

            char* addressBegin;
            addressBegin = bm.get_content(*btmp) + bm.get_usingSize(*btmp);
            memcpy(addressBegin, &i, sizeof(IndexInfo));
            bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + sizeof(IndexInfo));
			btmp->dirty = true;


            return this->setIndexOnAttribute(tableName,Attribute,indexName);
        }
        else
        {
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }

    return 0;
}
int CatalogManager::cm_table_file_name(string tableName)
{
    FILE *fp;
    fp = fopen(tableName.c_str(), "r");
    if (fp == NULL)
    {
        return 0;
    }
    else
    {
        fclose(fp);
        return TABLE_FILE;
    }

}
int CatalogManager::cm_find_index(string fileName)
{
    fileNode *ftmp = bm.getFile("Indexs");
    blockNode *btmp = bm.getBlockHead(ftmp);
    if (btmp )
    {
        char* addressBegin;
        addressBegin = bm.get_content(*btmp);
        IndexInfo * i = (IndexInfo *)addressBegin;
        int flag = UNKNOWN_FILE;
        for(int j = 0 ;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
        {
            if((*i).indexName==fileName)
            {
                flag = INDEX_FILE;
                break;
            }
            i ++;
        }
        return flag;
    }

    return 0;
}
int CatalogManager::cm_DropIndex(string index)
{
    fileNode *ftmp = bm.getFile("Indexs");
    blockNode *btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        char* addressBegin;
        addressBegin = bm.get_content(*btmp);
        IndexInfo * i = (IndexInfo *)addressBegin;
        int j = 0;
        for(j = 0 ;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
        {
            if((*i).indexName==index)
            {
                break;
            }
            i ++;
        }
        this->revokeIndexOnAttribute((*i).tableName,(*i).Attribute,(*i).indexName);
        for(;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo)-1);j++)
        {
            (*i) = *(i + sizeof(IndexInfo));
            i ++;
        }
        bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) - sizeof(IndexInfo));
		btmp->dirty = true;
        return 1;
    }

    return 0;
}
int CatalogManager::revokeIndexOnAttribute(string tableName,string AttributeName,string indexName)
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {

        char* addressBegin = bm.get_content(*btmp) ;
        addressBegin += (1+sizeof(int));
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *)addressBegin;
        int i;
        for(i =0;i<size;i++)
        {
            if(a->name == AttributeName)
            {
                if(a->index == indexName)
                {
                    a->index = "";
					btmp->dirty = true;
                }
                else
                {
                    cout<<"revoke unknown index: "<<indexName<<" on "<<tableName<<"!"<<endl;
                    cout<<"Attribute: "<<AttributeName<<" on table "<<tableName<<" has index: "<<a->index<<"!"<<endl;
                }
                break;
            }
            a ++;
        }
        if(i<size)
            return 1;
        else
            return 0;
    }
    return 0;
}
int CatalogManager::indexNameListGet(string tableName, vector<string>* indexNameVector)
{
	if (!tableExist(tableName)) {
		return 0;
	}

    fileNode *ftmp = bm.getFile("Indexs");
    blockNode *btmp = bm.getBlockHead(ftmp);
    if (btmp )
    {
        char* addressBegin;
        addressBegin = bm.get_content(*btmp);
        IndexInfo * i = (IndexInfo *)addressBegin;
        for(int j = 0 ;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
        {
            if((*i).tableName==tableName)
            {
                (*indexNameVector).push_back((*i).indexName);
            }
            i ++;
        }
        return 1;
    }

    return 0;
}

int CatalogManager::cm_Delete(string tableName, int deleteNum)
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {

        char* addressBegin = bm.get_content(*btmp) ;
        int * recordNum = (int*)addressBegin;
        if((*recordNum) <deleteNum)
        {
            cout<<"error in CatalogManager::cm_Delete"<<endl;
            return 0;
        }
        else
            (*recordNum) -= deleteNum;

		btmp->dirty = true;
        return *recordNum;
    }
    return 0;
}
int CatalogManager::cm_Insert(string tableName, int recordNum)
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {

        char* addressBegin = bm.get_content(*btmp) ;
        int * originalRecordNum = (int*)addressBegin;
        *originalRecordNum += recordNum;
		btmp->dirty = true;
        return *originalRecordNum;
    }
    return 0;
}


int CatalogManager::cm_CreateTable(string tableName, vector<Attribute>* attributeVector, string primaryKeyName = "",int primaryKeyLocation = 0)
{
    FILE *fp;
    fp = fopen(tableName.c_str(), "w+");
    if (fp == NULL)
    {
        return 0;
    }
    fclose(fp);
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp )
    {
        char* addressBegin = bm.get_content(*btmp) ;
        int * size = (int*)addressBegin;
        *size = 0;// 0 record number
        addressBegin += sizeof(int);
        *addressBegin = primaryKeyLocation;//1 as what it says
        addressBegin++;
        *addressBegin = (*attributeVector).size();// 2 attribute number
        addressBegin++;
        //memcpy(addressBegin, attributeVector, (*attributeVector).size()*sizeof(Attribute));
        for(int i= 0;i<(*attributeVector).size();i++)
        {
            memcpy(addressBegin, &((*attributeVector)[i]), sizeof(Attribute));
            addressBegin += sizeof(Attribute);
        }
        bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + (*attributeVector).size()*sizeof(Attribute)+2+sizeof(int));
		btmp->dirty = true;
        return 1;
    }
    return 0;
}
int CatalogManager::setIndexOnAttribute(string tableName,string AttributeName,string indexName)
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {

        char* addressBegin = bm.get_content(*btmp) ;
        addressBegin += 1+sizeof(int);
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *)addressBegin;
        int i;
        for(i =0;i<size;i++)
        {
            if(a->name == AttributeName)
            {
                a->index = indexName;
				btmp->dirty = true;
                break;
            }
            a ++;
        }
        if(i<size)
            return 1;
        else
            return 0;
    }
    return 0;
}
int CatalogManager::attributeGet(string tableName, vector<Attribute>* attributeVector)
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {

        char* addressBegin = bm.get_content(*btmp) ;
        addressBegin += 1+sizeof(int);
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *)addressBegin;
        for(int i =0;i<size;i++)
        {
            attributeVector->push_back(*a);
            a ++;
        }

        return 1;
    }
    return 0;
}

int CatalogManager::calcuteRecordLenth(string tableName)
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {
        int singleRecordSize =  0;
        char* addressBegin = bm.get_content(*btmp) ;
        addressBegin += 1+sizeof(int);
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *)addressBegin;
        for(int i =0;i<size;i++)
        {
            if((*a).type==-1)
            {
                singleRecordSize += sizeof(float);
            }
            else if((*a).type == 0)
            {
                singleRecordSize += sizeof(int);
            }
            else if((*a).type>0)
            {
                singleRecordSize += (*a).type * sizeof(char);
            }
            else
            {
                cout<<"Catalog data damaged!"<<endl;
                return 0;
            }
            a ++;
        }

        return singleRecordSize;
    }
    return 0;
}

int CatalogManager::calcuteTypeLenth(int type){
    if (type == TYPE_INT) {
        return sizeof(int);
    }
    else if (type == TYPE_FLOAT)
    {
        return sizeof(float);
    }
    else{
        return (int) type; // Note that the type stores in Attribute.h
    }
}

int CatalogManager::tableExist(string tableName)
{
	if (cm_table_file_name(tableName) != TABLE_FILE)
	{
		cout << "There is no table " << tableName << endl;
		return 0;
	}
	else
	{
		return 1;
	}
}

