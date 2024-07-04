#ifndef __SINGLETON_H__
#define __SINGLETON_H__

// class >> [Singleton] �e���v���[�g�N���X
template<typename Temp>
class Singleton
{
public:
	// public:static�֐�
	static Temp* GetInstance()
	{
		static Temp instance;
		return &instance;
	}

private:
	// private:�ϐ�
	Temp*	instance;
};

#endif // __SINGLETON_H__