#pragma  once 

#include <glog/logging.h>

inline void InitGlog(const char *app_name,const char *log_dir = nullptr) {

  google::InitGoogleLogging(app_name);
  FLAGS_alsologtostderr = true;
  FLAGS_colorlogtostderr = true;
  FLAGS_max_log_size = 10;
  if (log_dir)
    FLAGS_log_dir = log_dir;
  google::SetStderrLogging(google::GLOG_INFO);
}