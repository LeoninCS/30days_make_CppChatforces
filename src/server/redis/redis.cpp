#include "redis.hpp"
#include <thread>

Redis::Redis() : _redisContext(nullptr), _subContext(nullptr){
}
Redis::~Redis() {
  if(_redisContext != nullptr) {
    redisFree(_redisContext);
  }
  if(_subContext != nullptr) {
    redisFree(_subContext);
  }
}

bool Redis::connect() {
  //负责publish发布消息的上下文连接
  _redisContext = redisConnect("127.0.0.1", 6379);
  if (_redisContext == nullptr) {
    cerr << "connect redis failed!" << endl;
    return false;
  }
  //负责subscribe订阅消息的上下文连接
  _subContext = redisConnect("127.0.0.1", 6379);
  if (_subContext == nullptr) {
    cerr << "connect redis failed!" << endl;
    return false;
  }

  thread t([&]() {
    observer_channel_message();
  });
  t.detach();

  cout << "connect redis-server success!" << endl;
  return true;
}

  //向redis指定的通道channel发布消息
bool Redis::publish(int channel, string message) {
    redisReply *reply = (redisReply *)redisCommand(_redisContext, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr) {
      cerr << "publish command failed!" << endl;
      return false;
    }
    freeReplyObject(reply);
    return true;
}

  //向redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel) {
  if(REDIS_ERR == redisAppendCommand(this->_subContext, "SUBSCRIBE %d", channel)) {
    cerr << "subscribe command failed!" << endl;
    return false;
  }
  // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
  int done = 0;
  while(!done) {
    if(REDIS_ERR == redisBufferWrite(this->_subContext, &done)) {
      cerr << "subscribe command failed!" << endl;
      return false;
    }
  }
  return true;
}

  //向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel) {
  if(REDIS_ERR == redisAppendCommand(this->_subContext, "UNSUBSCRIBE %d", channel)) {
    cerr << "unsubscribe command failed!" << endl;
    return false;
  }
  // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
  int done = 0;
  while(!done) {
    if(REDIS_ERR == redisBufferWrite(this->_subContext, &done)) {
      cerr << "unsubscribe command failed!" << endl;
      return false;
    }
  }
  return true;
}
  
  //在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message() {
  redisReply *reply = nullptr;
  while(REDIS_OK == redisGetReply(this->_subContext, (void**)&reply)) {
    //订阅收到的消息是一个带三元素的数组
    if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
      //给业务层上报通道上发生的消息
      _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
    }
    //释放资源
    freeReplyObject(reply);
  }
  cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

  //初始化向业务层上报通道消息的回调对象
void Redis::init_notify_handler(function<void(int, string)> fn) {
  this->_notify_message_handler = fn;
}