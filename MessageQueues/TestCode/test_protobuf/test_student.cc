#include<iostream>
#include"student.pb.h"

int main()
{
  Student::Student s1;
  s1.set_studentname("张三");
  s1.set_studentnumber(1000);

  std::string str;
  str = s1.SerializeAsString();


  Student::Student ret;
  ret.ParseFromString(str);

  std::cout<<ret.studentname()<<std::endl<<ret.studentnumber()<<std::endl;
  return 0;
}