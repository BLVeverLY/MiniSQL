//#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "Interpreter.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include "API.h"


clock_t start;
clock_t finish;

int main(int argc,char * argv[])
{   
    API api;

    CatalogManager cm;
	api.cm = &cm;

    RecordManager rm;
    api.rm = &rm;
	rm.api = &api;
    
    IndexManager im(&api);
    api.im = &im;
    
	FILE *fp;
	fp = fopen("Indexs", "r");
	fclose(fp);

    int fileRead = 0;
    //string fileName ="";
    ifstream file;
    Interpreter in;
    in.ap = &api;
    string s;
    int status = 0;
    while(1)
    {
        if(fileRead)
        {

            file.open(in.fileName.c_str());
            if(!file.is_open())
            {
                cout<<"Fail to open "<<in.fileName<<endl;
                fileRead = 0;
                continue;
            }
			start = clock();
            while(getline(file,s,';'))
            {
                in.interpreter(s);
            }
            file.close();
            fileRead = 0;
            finish = clock();
            double duration = (double)(finish - start) / CLOCKS_PER_SEC;
            duration *= 1000;
            printf( "%2.1f milliseconds\n", duration );
        }
        else
        {

            cout<<"minisql>>";
            getline(cin,s,';');
            start = clock();
            status =  in.interpreter(s);
            if(status==2)
            {
                fileRead = 1;
            }
            else if(status==587)
            {
                break;
            }
            else{
                finish = clock();
                double duration = (double)(finish - start) / CLOCKS_PER_SEC;
                duration *= 1000;
                printf( "The duration is %2.1f milliseconds\n", duration);
            }
        }

    }
    
    return 0;
}

