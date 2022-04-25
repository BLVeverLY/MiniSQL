#pragma once

#include <string>
#include <iostream>
using namespace std;

#define TYPE_INT  0
#define TYPE_FLOAT -1

class Attribute
{
public:
    string name;
    int type;                  //the type of the attribute,-1 represents float, 0 represents int, other positive integer represents char and the value is the number of char)
    bool ifUnique;
	string index = "";       // default value is "", representing no index

    Attribute(string n, int t, bool i):name(n),type(t),ifUnique(i){}
	~Attribute(){}

    string indexNameGet(){return index;}
};