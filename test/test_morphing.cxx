//Base Class H
#include <iostream>
#include <stdio.h>
using namespace std;
class BaseClass
{
    public:
        BaseClass();
        virtual ~BaseClass();

        virtual void functionA();
        virtual void functionB();
};

//Derived Class H
class DerivedClass : public BaseClass
{
    public:
        DerivedClass();
        virtual ~DerivedClass();

        void functionB(); // Overridden function
};

//Base Class CPP
BaseClass::BaseClass()
{
    this->functionA();
}

BaseClass::~BaseClass()
{
}

void BaseClass::functionA()
{
    printf("BaseClass A\n");
    this->functionB();
}

void BaseClass::functionB()
{
    printf("BaseClass B\n");
}


//Derived Class CPP
DerivedClass::DerivedClass():BaseClass()
{
    printf("DerivedClass generated\n");
}

DerivedClass::~DerivedClass()
{
}

void DerivedClass::functionB()
{
    printf("Derived B\n");
}

// In Main
int main(int argc, char** argv)
{
    DerivedClass* newObj = new DerivedClass();
    BaseClass* base = newObj;
    base->functionA();
    delete newObj;
    return 0;
}
