#ifndef SINGLETON_H
#define SINGLETON_H

#define Singleton(x)   \
private:  \
	x(){} \
	~x(){} \
public: \
	static x* GetInstance(){static x instance; return &instance;}

#define Singleton2(x,construct,destruct) \
	x(){ construct();} \
	~x(){destruct();} \
public: \
	static x* GetInstance(){static x instance; return &instance;}

#endif


	
