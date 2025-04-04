#include "../MQCommon/Helper.hpp"

int main()
{
  FileHelper helper("../MQCommon/Helper.hpp");
  DLOG("是否存在：%d", helper.exists());
  DLOG("文件大小：%ld", helper.size());

  FileHelper tmp_helper("./aaa/bbb/ccc/tmp.hpp");
  if (tmp_helper.exists() == false)
  {
    std::string path = FileHelper::parentDirectory("./aaa/bbb/ccc/tmp.hpp");
    DLOG("path:%s",path.c_str());
    if (FileHelper(path).exists() == false)
    {
      FileHelper::createDirectory(path);
    }
    FileHelper::createFile("./aaa/bbb/ccc/tmp.hpp");
  }

  std::string body;
  helper.read(body);
  DLOG("读测试，读出的内容：%s",body.c_str());
  
  tmp_helper.write(body);
  FileHelper tmp_helper1("./aaa/bbb/ccc/tmp.hpp");
  char str[16] = {0};
  tmp_helper.read(str, 8, 11);
  DLOG("[%s]", str);
  tmp_helper.write("12345678901", 8, 11);
  tmp_helper.rename("./aaa/bbb/ccc/test.hpp");

   FileHelper::removeFile("./aaa/bbb/ccc/test.hpp");
   FileHelper::removeDirectory("./aaa");
  return 0;
}