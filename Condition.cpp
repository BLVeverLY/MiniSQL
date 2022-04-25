#include "Condition.h"

bool Condition::ifRight(int content)
{
    stringstream ss;
    ss << value;
    int myContent;
    ss >> myContent;
    
    switch (operate)
    {
        case Condition::OP_eq:
            return content == myContent;
            break;
        case Condition::OP_neq:
            return content != myContent;
            break;
        case Condition::OP_less:
            return content < myContent;
            break;
        case Condition::OP_greater:
            return content > myContent;
            break;
        case Condition::OP_eq_less:
            return content <= myContent;
            break;
        case Condition::OP_eq_greater:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

bool Condition::ifRight(float content)
{
    stringstream ss;
    ss << value;
    float myContent;
    ss >> myContent;
    
    switch (operate)
    {
        case Condition::OP_eq:
            return content == myContent;
            break;
        case Condition::OP_neq:
            return content != myContent;
            break;
        case Condition::OP_less:
            return content < myContent;
            break;
        case Condition::OP_greater:
            return content > myContent;
            break;
        case Condition::OP_eq_less:
            return content <= myContent;
            break;
        case Condition::OP_eq_greater:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

bool Condition::ifRight(string content)
{
    string myContent = value;
    switch (operate)
    {
        case Condition::OP_eq:
            return content == myContent;
            break;
        case Condition::OP_neq:
            return content != myContent;
            break;
        case Condition::OP_less:
            return content < myContent;
            break;
        case Condition::OP_greater:
            return content > myContent;
            break;
        case Condition::OP_eq_less:
            return content <= myContent;
            break;
        case Condition::OP_eq_greater:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}