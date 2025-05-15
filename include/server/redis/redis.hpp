#ifndef REDIS_HPP
#define REDIS_HPP

#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <functional>
using namespace std;


class Redis {
public:
  Redis();
  ~Redis();
  
  //连接redis服务器
  bool connect();

  //向redis指定的通道channel发布消息
  bool publish(int channel, string message);

  //向redis指定的通道subscribe订阅消息
  bool subscribe(int channel);

  //向redis指定的通道unsubscribe取消订阅消息
  bool unsubscribe(int channel);
  
  //在独立线程中接收订阅通道中的消息
  void observer_channel_message();

  //初始化向业务层上报通道消息的回调对象
  void init_notify_handler(function<void(int, string)> fn);

private:
    //负责push
    redisContext *_redisContext;
    //负责订阅
    redisContext *_subContext;
    //回调操作，收到订阅的消息，给service层上报
    function<void(int, string)> _notify_message_handler;

};
#endif