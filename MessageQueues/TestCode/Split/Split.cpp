#include <iostream>
#include <string>
#include <vector>

size_t split(const std::string &str, const std::string &sep, std::vector<std::string> &result)
{
  int pos = 0;
  int idx = 0;

  while (idx < str.size())
  {
    pos = str.find(sep, idx);
    if (pos == std::string::npos)//未找到
    {
      std::string tmp = str.substr(idx);
      result.push_back(tmp);
      return result.size();
    }

    if (pos == idx)//分割符之间没有数据
    {
      idx = pos + sep.size();
      continue;
    }

    std::string tmp = str.substr(idx, pos - idx);
    result.push_back(tmp);
    idx = pos + sep.size();
  }
  return result.size();
}

int main()
{
  std::string str("");
  std::vector<std::string> arr;
  std::string sep = ".";

  auto n =split(str,sep,arr);
  std::cout<<n<<"        "<<std::endl;
  for(auto a : arr)
  {
    std::cout<<a<<std::endl;
  }

}