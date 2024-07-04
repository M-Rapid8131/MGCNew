#ifndef __SINGLETON_H__
#define __SINGLETON_H__

// class >> [Singleton] テンプレートクラス
template<typename Temp>
class Singleton
{
public:
	// public:static関数
	static Temp* GetInstance()
	{
		static Temp instance;
		return &instance;
	}

private:
	// private:変数
	Temp*	instance;
};

#endif // __SINGLETON_H__