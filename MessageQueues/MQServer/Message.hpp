#ifndef __M_MESSAGE_H__
#define __M_MESSAGE_H__
#include "../MQCommon/Helper.hpp"
#include "../MQCommon/Logger.hpp"
#include "../MQCommon/message.pb.h"
#include <google/protobuf/map.h>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace MQ
{
#define DATAFILE_SUBFIX ".message_data"
#define TMPFILE_SUBFIX ".message_data.tmp"

  using MessagePtr = std::shared_ptr<MQ::Message>;
  class MessageMapper
  {
  public:
    MessageMapper(std::string &path, const std::string &queue_name)
        : _name_queue(queue_name)
    {
      if (path.back() != '/')
        path += '/';

      _name_data_file = path + queue_name + DATAFILE_SUBFIX;
      _name_temp_file = path + queue_name + TMPFILE_SUBFIX;
      if (!FileHelper(path).exists())
      {
        assert(FileHelper::createDirectory(path));
      }

      if (!FileHelper(_name_data_file).exists())
        creatFile();
    }

    bool insertDataFile(MessagePtr &message)
    {
      return insert(_name_data_file, message);
    }

    bool insertTempFile(MessagePtr &message)
    {
      return insert(_name_temp_file, message);
    }

    bool remove(MessagePtr &message)
    {
      std::string filename = _name_data_file;
      size_t offset = message->offset();
      size_t length = message->length();
      // 修改文件的有效标志
      message->mutable_payload()->set_valid("0");
      std::string new_load = message->payload().SerializeAsString();
      // 判断新的载荷与原载荷长度是否相同，来防止覆盖掉别的消息
      if (new_load.size() != length)
      {
        ELOG("消息载荷长度与原载荷长度不同");
        return false;
      }
      // 将修改合法标志的数据写入文件(覆盖原载荷的位置)
      FileHelper file_helper(filename);
      file_helper.write(new_load.c_str(), offset, new_load.size());
      return true;
    }

    std::list<MessagePtr> garbageCollection()
    {
      bool ret;
      std::list<MessagePtr> result;
      ret = load(result);
      if (ret == false)
      {
        DLOG("加载有效数据失败！\n");
        return result;
      }
      // DLOG("垃圾回收，得到有效消息数量：%d", result.size());
      // 2. 将有效数据，进行序列化存储到临时文件中
      FileHelper::createFile(_name_temp_file);
      for (auto &msg : result)
      {
        DLOG("向临时文件写入数据: %s", msg->payload().body().c_str());
        ret = insert(_name_temp_file, msg);
        if (ret == false)
        {
          DLOG("向临时文件写入消息数据失败！！");
          return result;
        }
      }
      // DLOG("垃圾回收后，向临时文件写入数据完毕，临时文件大小: %ld", FileHelper(_name_temp_file).size());
      // 3. 删除源文件
      ret = FileHelper::removeFile(_name_data_file);
      if (ret == false)
      {
        DLOG("删除源文件失败！");
        return result;
      }
      // 4. 修改临时文件名，为源文件名称
      ret = FileHelper(_name_temp_file).rename(_name_data_file);
      if (ret == false)
      {
        DLOG("修改临时文件名称失败！");
        return result;
      }
      // 5. 返回新的有效数据
      return result;
    }

    void removeFile()
    {
      FileHelper::removeFile(_name_data_file);
      FileHelper::removeFile(_name_temp_file);
    }

  private:
    bool load(std::list<MessagePtr> &result)
    {

      // 挑选出有效信息
      FileHelper file_helper(_name_data_file);
      size_t offset = 0, length = 0;
      while (offset < file_helper.size())
      {
        // 读取消息长度
        file_helper.read((char *)&length, offset, sizeof(size_t));
        offset += sizeof(size_t);
        // 读取消息
        std::string load_str(length, '\0');
        file_helper.read(&load_str[0], offset, length);
        offset += length;

        // 反序列化消息
        MessagePtr message = std::make_shared<MQ::Message>();
        message->mutable_payload()->ParseFromString(load_str);
        // 判断消息是否有效
        if (message->payload().valid() == std::string("0"))
        {
          DLOG("该消息无效，不用插入队列");
          continue;
        }
        // 将有效消息插入队列
        result.push_back(message);
      }
      return true;
    }

    bool creatFile()
    {
      if (FileHelper(_name_data_file).exists())
      {
        return true;
      }
      return FileHelper::createFile(_name_data_file);
    }

    bool insert(const std::string &filename, MessagePtr &message)
    {
      bool ret = true;
      // 将消息中的消息载荷序列化
      std::string load = message->payload().SerializeAsString();
      // 找到消息位置的偏移量
      FileHelper file_helper(filename);
      // 偏移量
      size_t offset = file_helper.size();
      // 消息的长度
      size_t length = load.size();
      // 先将消息的长度写入文件
      ret = file_helper.write((char *)&length, offset, sizeof(size_t));
      if (ret == false)
      {
        ELOG("写入消息长度失败");
        return false;
      }
      // 将消息插入文件
      ret = file_helper.write(load.c_str(), offset + sizeof(size_t), length);
      if (ret == false)
      {
        ELOG("写入消息失败");
        return false;
      }
      // 设置message的偏移量和长度
      message->set_offset(offset + sizeof(size_t));
      message->set_length(length);

      return true;
    }

  private:
    std::string _name_data_file;
    std::string _name_temp_file;
    std::string _name_queue;
  };

  // 队列消息类，主要是负责消息与队列之间的关系
  class QueueMessage
  {
  public:
    using ptr = std::shared_ptr<QueueMessage>;

    QueueMessage(std::string &path, const std::string &qname) : _qname(qname), _valid_count(0), _total_count(0), _mapper(path, qname)
    {
    }

    ~QueueMessage() {}
    // 传入队列消息的属性、消息体、是否持久化
    bool insert(const BasicProperties *bp, const std::string &body, bool queue_is_durable)
    {
      // 1. 构造消息对象
      MessagePtr msg = std::make_shared<MQ::Message>();
      msg->mutable_payload()->set_body(body);
      // 如果消息属性不为空，则使用传入的属性，设置，否则使用默认属性
      if (bp != nullptr)
      {
        DeliveryMode mode = queue_is_durable ? bp->delivery_mode() : DeliveryMode::UNDURABLE;
        msg->mutable_payload()->mutable_properties()->set_id(bp->id());
        msg->mutable_payload()->mutable_properties()->set_delivery_mode(mode);
        msg->mutable_payload()->mutable_properties()->set_routing_key(bp->routing_key());
      }
      else
      {
        DeliveryMode mode = queue_is_durable ? DeliveryMode::DURABLE : DeliveryMode::UNDURABLE;
        msg->mutable_payload()->mutable_properties()->set_id(UUIDHelper::uuid());
        msg->mutable_payload()->mutable_properties()->set_delivery_mode(mode);
        msg->mutable_payload()->mutable_properties()->set_routing_key("");
      }
      std::unique_lock<std::mutex> lock(_mutex);
      // 2. 判断消息是否需要持久化
      if (msg->payload().properties().delivery_mode() == DeliveryMode::DURABLE)
      {
        msg->mutable_payload()->set_valid("1"); // 在持久化存储中表示数据有效
        // 3. 进行持久化存储
        bool ret = _mapper.insertDataFile(msg);
        if (ret == false)
        {
          DLOG("持久化存储消息：%s 失败了！", body.c_str());
          return false;
        }
        _valid_count += 1; // 持久化信息中的数据量+1
        _total_count += 1;
        _durable_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
      }
      // 4. 内存的管理
      _msgs.push_back(msg);
      return true;
    }

    bool remove(const std::string &msg_id)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      // 1. 从待确认队列中查找消息
      auto it = _waitack_msgs.find(msg_id);
      if (it == _waitack_msgs.end())
      {
        DLOG("没有找到要删除的消息：%s!", msg_id.c_str());
        return true;
      }
      // 2. 根据消息的持久化模式，决定是否删除持久化信息
      if (it->second->payload().properties().delivery_mode() == DeliveryMode::DURABLE)
      {
        // 3. 删除持久化信息
        _mapper.remove(it->second);
        _durable_msgs.erase(msg_id);
        _valid_count -= 1;   // 持久化文件中有效消息数量 -1
        garbageCollection(); // 内部判断是否需要垃圾回收，需要的话则回收一下
      }
      // 4. 删除内存中的信息
      _waitack_msgs.erase(msg_id);
      // DLOG("确认消息后，删除消息的管理成功：%s", it->second->payload().body().c_str());
      return true;
    }

    bool recovery()
    {
      // 恢复历史消息
      std::unique_lock<std::mutex> lock(_mutex);
      _msgs = _mapper.garbageCollection();
      for (auto &msg : _msgs)
      {
        _durable_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
      }
      _valid_count = _total_count = _msgs.size();
      return true;
    }
    // 从队首取出消息
    MessagePtr front()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      if (_msgs.empty())
      {
        return MessagePtr();
      }
      // 获取一条队首消息：从_msgs中取出数据
      MessagePtr msg = _msgs.front();
      _msgs.pop_front();
      // 将该消息对象，向待确认的hash表中添加一份，等到收到消息确认后进行删除
      _waitack_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
      return msg;
    }

    size_t getAbleCount()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _msgs.size();
    }
    size_t getTotalCount()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _total_count;
    }
    size_t getDurableCount()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _durable_msgs.size();
    }
    size_t getWaitackCount()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _waitack_msgs.size();
    }

    void clear()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _mapper.removeFile();
      _msgs.clear();
      _durable_msgs.clear();
      _waitack_msgs.clear();
      _valid_count = 0;
      _total_count = 0;
    }

  private:
    bool garbageCollectionCheck()
    {
      // 持久化的消息总量大于2000， 且其中有效比例低于50%则需要持久化
      if (_total_count > 2000 && _valid_count * 10 / _total_count < 5)
      {
        return true;
      }
      return false;
    }
    void garbageCollection()
    {
      // 1. 进行垃圾回收，获取到垃圾回收后，有效的消息信息链表
      if (garbageCollectionCheck() == false)
        return;
      std::list<MessagePtr> msgs = _mapper.garbageCollection();
      for (auto &msg : msgs)
      {
        auto it = _durable_msgs.find(msg->payload().properties().id());
        if (it == _durable_msgs.end())
        {
          DLOG("垃圾回收后，有一条持久化消息，在内存中没有进行管理!");
          _msgs.push_back(msg); /// 做法：重新添加到推送链表的末尾
          _durable_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
          continue;
        }
        // 2. 更新每一条消息的实际存储位置
        it->second->set_offset(msg->offset());
        it->second->set_length(msg->length());
      }
      // 3. 更新当前的有效消息数量 & 总的持久化消息数量
      _valid_count = _total_count = msgs.size();
    }

  private:
    std::mutex _mutex;
    std::string _qname;
    size_t _valid_count;
    size_t _total_count;//文件里的总数据量，内存里的不计入其中
    MessageMapper _mapper;
    std::list<MessagePtr> _msgs;                               // 待推送消息
    std::unordered_map<std::string, MessagePtr> _durable_msgs; // 持久化消息hash
    std::unordered_map<std::string, MessagePtr> _waitack_msgs; // 待确认消息hash
  };

  class MessageManager
  {
  public:
    using ptr = std::shared_ptr<MessageManager>;
    MessageManager(const std::string &basedir) : _basedir(basedir) {}
    ~MessageManager() {}
    void initQueueMessage(const std::string &qname)
    {
      QueueMessage::ptr qmp;
      {
        // 查找是否已经存在该队列的消息管理对象
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it != _queue_msgs.end())
        {
          return;
        }
        // 如果没找到，说明要新增
        qmp = std::make_shared<QueueMessage>(_basedir, qname);
        _queue_msgs.insert(std::make_pair(qname, qmp));
      }
      // 恢复历史消息
      qmp->recovery();
    }

    void clear()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      for (auto &qmsg : _queue_msgs)
      {
        qmsg.second->clear();
      }
    }

    void destroyQueueMessage(const std::string &qname)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        // 没找到，说明无需删除
        if (it == _queue_msgs.end())
        {
          return;
        }
        qmp = it->second;
        _queue_msgs.erase(it);
      }
      // 销毁消息管理对象
      qmp->clear();
    }

    bool insert(const std::string &qname, BasicProperties *bp, const std::string &body, bool queue_is_durable)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it == _queue_msgs.end())
        {
          ELOG("向队列%s新增消息失败：没有找到消息管理句柄!", qname.c_str());
          return false;
        }
        qmp = it->second;
      }
      return qmp->insert(bp, body, queue_is_durable);
    }
    MessagePtr front(const std::string &qname)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it == _queue_msgs.end())
        {
          ELOG("获取队列%s队首消息失败：没有找到消息管理句柄!", qname.c_str());
          return MessagePtr();
        }
        qmp = it->second;
      }
      return qmp->front();
    }
    // 确认消息，实际上就是确认消息之后，删除待确认消息里的对应消息
    void ack(const std::string &qname, const std::string &msg_id)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it == _queue_msgs.end())
        {
          ELOG("确认队列%s消息%s失败：没有找到消息管理句柄!", qname.c_str(), msg_id.c_str());
          return;
        }
        qmp = it->second;
      }
      qmp->remove(msg_id);
      return;
    }

    size_t getAbleCount(const std::string &qname)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it == _queue_msgs.end())
        {
          DLOG("获取队列%s待推送消息数量失败：没有找到消息管理句柄!", qname.c_str());
          return 0;
        }
        qmp = it->second;
      }
      return qmp->getAbleCount();
    }
    size_t getTotalCount(const std::string &qname)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it == _queue_msgs.end())
        {
          DLOG("获取队列%s总持久化消息数量失败：没有找到消息管理句柄!", qname.c_str());
          return 0;
        }
        qmp = it->second;
      }
      return qmp->getTotalCount();
    }
    size_t getDurableCount(const std::string &qname)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it == _queue_msgs.end())
        {
          DLOG("获取队列%s有效持久化消息数量失败：没有找到消息管理句柄!", qname.c_str());
          return 0;
        }
        qmp = it->second;
      }
      return qmp->getDurableCount();
    }
    size_t getWaitAckCount(const std::string &qname)
    {
      QueueMessage::ptr qmp;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _queue_msgs.find(qname);
        if (it == _queue_msgs.end())
        {
          DLOG("获取队列%s待确认消息数量失败：没有找到消息管理句柄!", qname.c_str());
          return 0;
        }
        qmp = it->second;
      }
      return qmp->getWaitackCount();
    }

  private:
    std::mutex _mutex;
    std::string _basedir;
    std::unordered_map<std::string, QueueMessage::ptr> _queue_msgs;
  };
}

#endif