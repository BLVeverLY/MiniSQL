#pragma once

#include <string>
#include <sstream>
using namespace std;

class Condition
{
    
public:
    const static int OP_eq = 0; // "="
    const static int OP_neq = 1; // "<>"
    const static int OP_less = 2; // "<"
    const static int OP_greater = 3; // ">"
    const static int OP_eq_less = 4; // "<="
    const static int OP_eq_greater = 5; // ">="
    
    Condition(string a,string v,int o)
	{
		attributeName = a;
		value = v;
		operate = o;
	}
    
    string attributeName;
    string value;           // the value to be compared
    int operate;            // the type to be compared
    
    bool ifRight(int content);
    bool ifRight(float content);
    bool ifRight(string content);
};