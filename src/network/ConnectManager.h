#pragma once

#include "TcpConnect.h"
#include <map>
#include <mutex>

class ConnectManager {
public:
  ConnectManager()=default;
  ~ConnectManager()=default;

  void AddConnectToManager(const std::string& name,TcpConnect::Ptr conn);
  void RemoveConnectToManager(const std::string& name);

private:
  std::mutex conn_map_mutex_; 
  std::map<std::string , TcpConnect::Ptr> conn_map_;
};