#define BUILTIN_LIKELY(expr) __builtin_expect(!!(expr), true)
#define BUILTIN_UNLIKELY(expr) __builtin_expect(!!(expr), false)

//这里是开启分支预测