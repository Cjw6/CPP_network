#pragma once

#include "TcpConnect.h"
#include <map>
#include <mutex>

class TcpConnectManager {
public:
  TcpConnectManager()=default;
  ~TcpConnectManager()=default;

  void AddConnectToManager(const std::string& name,TcpConnect::Ptr conn);
  void RemoveConnectToManager(const std::string& name);
  void CheckTcpConnectTimeout();

private:
  std::mutex conn_map_mutex_; 
  std::map<std::string , TcpConnect::Ptr> conn_map_;
  
};