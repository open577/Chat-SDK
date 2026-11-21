#include <sqlite3.h>
#include <string>
#include <iostream>

struct Student
{
    std::string name;
    std::string gender;
    int age;
    double gap;
    Student(std::string &name, std::string &gender, int &age, double &gap)
        : name(name), gender(gender), age(age), gap(gap)
    {
    }
};

class StudentDB
{
public:
private:
};

int main()
{
    return 0;
}