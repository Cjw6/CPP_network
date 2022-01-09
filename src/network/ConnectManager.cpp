#include "ConnectManager.h"
#include <mutex>

void ConnectManager::AddConnectToManager(const std::string &name, TcpConnect::Ptr conn)
{
  std::lock_guard<std::mutex>  lg(conn_map_mutex_);
  conn_map_[name]=conn;
}

void ConnectManager::RemoveConnectToManager(const std::string &name)
{
  std::lock_guard<std::mutex>  lg(conn_map_mutex_);
  conn_map_.erase(name);
}
