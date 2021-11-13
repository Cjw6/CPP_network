#include "Thread.h"
#include <sys/syscall.h>
#include <unistd.h>

static thread_local pid_t t_thread_id = 0;

/**__builtin_expect() 是 GCC (version
>= 2.96）提供给程序员使用的，目的是将“分支转移”的信息提供给编译器，
 * 这样编译器可以对代码进行优化，以减少指令跳转带来的性能下降。

作者：大明白
链接：https://www.jianshu.com/p/2684613a300f
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。*/

pid_t GetThreadId()
{
    if (__builtin_expect(t_thread_id == 0, 0))
    {
        t_thread_id = static_cast<pid_t>(::syscall(SYS_gettid));
    }
    return t_thread_id;
}

