#include "API.h"
#include "RecordManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"

#define UNKNOWN_FILE 8
#define TABLE_FILE 9
#define INDEX_FILE 10

CatalogManager *cm;
IndexManager* im;

void API::DropTable(string tableName)
{
    if (!cm->tableExist(tableName)) return;
    
    vector<string> indexNameVector;
    
    //get all index in the table, and drop them all
	cm->indexNameListGet(tableName, &indexNameVector);
    for (int i = 0; i < indexNameVector.size(); i++)
    {       
        DropIndex(indexNameVector[i]);
    }
    
    //delete a table file
    if(rm->rm_DropTable(tableName))
    {
        //delete a table information
        cm->cm_DropTable(tableName);
        printf("Drop table %s successfully\n", tableName.c_str());
    }
}

void API::DropIndex(string indexName)
{
    if (cm->cm_find_index(indexName) != INDEX_FILE)
    {
        printf("There is no index %s \n", indexName.c_str());
        return;
    }
    
    //delete a index file
    if (rm->rm_DropIndex(indexName))
    {
        
        //get type of index
        int indexType = cm->getIndexType(indexName);
        if (indexType == -2)
        {
            printf("error\n");
            return;
        }
        
        //delete a index information
        cm->cm_DropIndex(indexName);
        
        //delete a index tree
        im->DropIndex("INDEX_FILE_" + indexName, indexType);
        printf("Drop index %s successfully\n", indexName.c_str());
    }
}

void API::CreateIndex(string indexName, string tableName, string attributeName)
{
    if (cm->cm_find_index(indexName) == INDEX_FILE)
    {
        cout << "There is index " << indexName << " already" << endl;
        return;
    }
    
    if (!cm->tableExist(tableName)) return;
    
    vector<Attribute> attributeVector;
    cm->attributeGet(tableName, &attributeVector);
    int i;
    int type = 0;
    for (i = 0; i < attributeVector.size(); i++)
    {
        if (attributeName == attributeVector[i].name)
        {
            if (!attributeVector[i].ifUnique)
            {
                cout << "the attribute is not unique" << endl;
                
                return;
            }
            type = attributeVector[i].type;
            break;
        }
    }
    
    if (i == attributeVector.size())
    {
        cout << "there is not this attribute in the table" << endl;
        return;
    }
    
     //RecordManager to create a index file
    if (rm->rm_CreateIndex(indexName))
    {
        //CatalogManager to add a index information
        cm->cm_InsertIndex(indexName, tableName, attributeName, type);
        
        //get type of index
        int indexType = cm->getIndexType(indexName);
        if (indexType == -2)
        {
            cout << "error";
            return;
        }
        
        //indexManager to create a index tress
        im->createIndex("INDEX_FILE_" + indexName, indexType);
        
        //recordManager insert already record to index
        rm->indexRecordAllAlreadyInsert(tableName, indexName);
        printf("Create index %s successfully\n", indexName.c_str());
    }
    else
    {
        cout << "Create index " << indexName << " fail" << endl;
    }
}

void API::CreateTable(string tableName, vector<Attribute>* attributeVector, string primaryKeyName,int primaryKeyLocation)
{    
    if(cm->cm_table_file_name(tableName) == TABLE_FILE)
    {
        cout << "There is a table " << tableName << " already" << endl;
        return;
    }
    
    //RecordManager to create a table file
    if(rm->rm_CreateTable(tableName))
    {
        //CatalogManager to create a table information
        //cm->cm_CreateTable(tableName, attributeVector, primaryKeyName, primaryKeyLocation);
   
        printf("Create table %s successfully\n", tableName.c_str());
    }
    
    if (primaryKeyName != "")
    {
        //get a primary key
        string indexName = "PRIMARY_" + tableName;
        CreateIndex(indexName, tableName, primaryKeyName);
    }
}

void API::Select(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector)
{
    if (cm->cm_table_file_name(tableName) == TABLE_FILE)
    {
        int num = 0;
        vector<Attribute> attributeVector;
        attributeGet(tableName, &attributeVector);
        
        vector<string> allAttributeName;
        if (attributeNameVector == NULL) {
            for (Attribute attribute : attributeVector)
            {
                allAttributeName.insert(allAttributeName.end(), attribute.name);
            }
            
            attributeNameVector = &allAttributeName;
        }
        
        //print attribute name you want to show
		int i = 0;
		for (i = 0; i < (*attributeNameVector).size(); i++)
		{
			printf("%s ", (*attributeNameVector)[i].c_str());
		}
		if (i != 0)
			printf("\n");
        
        for (string name : (*attributeNameVector))
        {
            int i = 0;
            for (i = 0; i < attributeVector.size(); i++)
            {
                if (attributeVector[i].name == name)
                {
                    break;
                }
            }
            
            if (i == attributeVector.size())
            {
                cout << "the attribute which you want to print is not exist in the table" << endl;
                return;
            }
        }
        
        int blockOffset = -1;
        if (conditionVector != NULL)
        {
            for (Condition condition : *conditionVector)
            {
                int i = 0;
                for (i = 0; i < attributeVector.size(); i++)
                {
                    if (attributeVector[i].name == condition.attributeName)
                    {
                        if (condition.operate == Condition::OP_eq && attributeVector[i].index != "")
                        {
                                blockOffset = im->searchIndex("INDEX_FILE_" + attributeVector[i].index, condition.value, attributeVector[i].type);
                        }
                        break;
                    }
                }
                
                if (i == attributeVector.size())
                {
                    cout << "the attribute is not exist in the table" << endl;
                    return;
                }
            }
        }
        
        if (blockOffset == -1)
        {
            //cout << "if we con't find the block by index,we need to find all block" << endl;
            num = rm->recordAllShow(tableName, attributeNameVector,conditionVector);
        }
        else
        {
            
            //find the block by index,search in the block
            num = rm->recordBlockShow(tableName, attributeNameVector, conditionVector, blockOffset);
        }
        
        printf("%d records selected\n", num);
    }
    else
    {
        cout << "There is no table " << tableName << endl;
    }
}

void API::Insert(string tableName, vector<string>* recordContent)
{
    if (!cm->tableExist(tableName)) return;
    
    string indexName;
    
    //deal if the record could be insert (if index is exist)
    vector<Attribute> attributeVector;
    
    vector<Condition> conditionVector;
    
    attributeGet(tableName, &attributeVector);
    for (int i = 0 ; i < attributeVector.size(); i++)
    {
        indexName = attributeVector[i].indexNameGet();
        if (indexName != "")
        {
            //if the attribute has a index
            int blockoffest = im->searchIndex("INDEX_FILE_" + indexName, (*recordContent)[i], attributeVector[i].type);
            
            if (blockoffest != -1)
            {
                //if the value has exist in index tree then fail to insert the record
                cout << "insert fail because index value exist" << endl;
                return;
            }
        }
        else if (attributeVector[i].ifUnique)
        {
            //if the attribute is unique but not index
            Condition condition(attributeVector[i].name, (*recordContent)[i], Condition::OP_eq);
            conditionVector.insert(conditionVector.end(), condition);
        }
    }
    
    if (conditionVector.size() > 0)
    {
        for (int i = 0; i < conditionVector.size(); i++) {
            vector<Condition> conditionTmp;
            conditionTmp.insert(conditionTmp.begin(), conditionVector[i]);
            
            int recordConflictNum =  rm->recordAllFind(tableName, &conditionTmp);
            if (recordConflictNum > 0) {
                cout << "insert fail because unique value exist" << endl;
                return;
            }

        }
    }
    
	// get the record string
    char recordString[2000];
    memset(recordString, 0, 2000);
	char * contentBegin = recordString;
	for (int i = 0; i < attributeVector.size(); i++)
	{
		Attribute attribute = attributeVector[i];
		string content = (*recordContent)[i];
		int type = attribute.type;
		int typeSize = cm->calcuteTypeLenth(type);
		stringstream ss;
		ss << content;
		if (type == TYPE_INT)
		{
			//if the content is a int
			int intTmp;
			ss >> intTmp;
			memcpy(contentBegin, ((char*)&intTmp), typeSize);
		}
		else if (type == TYPE_FLOAT)
		{
			//if the content is a float
			float floatTmp;
			ss >> floatTmp;
			memcpy(contentBegin, ((char*)&floatTmp), typeSize);
		}
		else
		{
			//if the content is a string
			memcpy(contentBegin, content.c_str(), typeSize);
		}
		contentBegin += typeSize;
	}

    
    //RecordManager to insert the record into file; and get the position of block being insert
    int recordSize = cm->calcuteRecordLenth(tableName);
    int blockOffset = rm->rm_Insert(tableName, recordString, recordSize);
    
    if(blockOffset >= 0)
    {
		char* contentBegin = recordString;
		for (int i = 0; i < attributeVector.size(); i++)
		{
			int type = attributeVector[i].type;
			int typeSize = typeSizeGet(type);
			if (attributeVector[i].index != "")
			{
				InsertIndex(attributeVector[i].index, contentBegin, type, blockOffset);
			}

			contentBegin += typeSize;
		}

        cm->cm_Insert(tableName, 1);
        printf("insert record into table %s successful\n", tableName.c_str());
    }
    else
    {
        cout << "insert record into table " << tableName << " fail" << endl;
    }
}

void API::Delete(string tableName, vector<Condition>* conditionVector)
{
    if (!cm->tableExist(tableName)) return;
    
    int num = 0;
    vector<Attribute> attributeVector;
    attributeGet(tableName, &attributeVector);

    int blockOffset = -1;
    if (conditionVector != NULL)
    {
        for (Condition condition : *conditionVector)
        {
            if (condition.operate == Condition::OP_eq)
            {
                for (Attribute attribute : attributeVector)
                {
                    if (attribute.index != "" && attribute.name == condition.attributeName)
                    {
                        blockOffset = im->searchIndex("INDEX_FILE_" + attribute.index, condition.value, attribute.type);
                    }
                }
            }
        }
    }

    
    if (blockOffset == -1)
    {
        //if we con't find the block by index,we need to find all block
        num = rm->recordAllDelete(tableName, conditionVector);
    }
    else
    {
        //find the block by index,search in the block
        num = rm->recordBlockDelete(tableName, conditionVector, blockOffset);
    }
    
    //delete the number of record in in the table
    cm->cm_Delete(tableName, num);
    printf("delete %d record in table %s\n", num, tableName.c_str());
}


/**
 *
 * get the size of a type
 * @param type:  type of attribute
 */
int API::typeSizeGet(int type)
{
    return cm->calcuteTypeLenth(type);
}



/**
 *
 * get the vector of all name of index's file
 * @param indexNameVector: will set all index's
 */
void API::allIndexAddressInfoGet(vector<IndexInfo> *indexNameVector)
{
    cm->getAllIndex(indexNameVector);
    for (int i = 0; i < (*indexNameVector).size(); i++)
    {
        (*indexNameVector)[i].indexName = "INDEX_FILE_" + (*indexNameVector)[i].indexName;
    }
}

/**
 *
 * get the vector of a attribute‘s type in a table
 * @param tableName:  name of table
 * @param attributeNameVector:  a point to vector of attributeType(which would change)
 */
int API::attributeGet(string tableName, vector<Attribute>* attributeVector)
{
    if (!cm->tableExist(tableName)) {
        return 0;
    }
    return cm->attributeGet(tableName, attributeVector);
}

/**
 *
 * insert a value to index tree
 * @param indexName: name of index
 * @param contentBegin: address of content
 * @param type: the type of content
 * @param blockOffset: the block offset num
 */
void API::InsertIndex(string indexName, char* contentBegin, int type, int blockOffset)
{
    string content= "";
    stringstream tmp;
    //if the attribute has index
    ///这里传*attributeVector)[i].index这个index的名字, blockOffset,还有值
    if (type == TYPE_INT)
    {
        int value = *((int*)contentBegin);
        tmp << value;
    }
    else if (type == TYPE_FLOAT)
    {
        float value = *((float* )contentBegin);
        tmp << value;
    }
    else
    {
        char value[255];
        memset(value, 0, 255);
        memcpy(value, contentBegin, sizeof(type));
        string stringTmp = value;
        tmp << stringTmp;
    }
    tmp >> content;
    im->insertIndex("INDEX_FILE_" + indexName, content, blockOffset, type);
}

/**
 *
 * delete all index value of a record to index tree
 * @param recordBegin: point to record begin
 * @param recordSize: size of the record
 * @param attributeVector:  a point to vector of attributeType(which would change)
 * @param blockOffset: the block offset num
 */
void API::recordIndexDelete(char* recordBegin,int recordSize, vector<Attribute>* attributeVector, int blockOffset)
{
    char* contentBegin = recordBegin;
    for (int i = 0; i < (*attributeVector).size() ; i++)
    {
        int type = (*attributeVector)[i].type;
        int typeSize = typeSizeGet(type);
        
        string content= "";
        stringstream tmp;
        
        if ((*attributeVector)[i].index != "")
        {
            //if the attribute has index
            ///这里传*attributeVector)[i].index这个index的名字, blockOffset,还有值
            if (type == TYPE_INT)
            {
                int value = *((int*)contentBegin);
                tmp << value;
            }
            else if (type == TYPE_FLOAT)
            {
                float value = *((float* )contentBegin);
                tmp << value;
            }
            else
            {
                char value[255];
                memset(value, 0, 255);
                memcpy(value, contentBegin, sizeof(type));
                string stringTmp = value;
                tmp << stringTmp;
            }
            
            tmp >> content;
            im->deleteIndexByKey("INDEX_FILE_" + (*attributeVector)[i].index, content, type);

        }
        contentBegin += typeSize;
    }

}

int API::recordSizeGet(string tableName)
{
	if (!cm->tableExist(tableName)) return 0;

	return cm->calcuteRecordLenth(tableName);
}