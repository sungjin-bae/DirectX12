#ifndef MYDIRECT_SOURCE_COMMON_SINGLETON_H
#define MYDIRECT_SOURCE_COMMON_SINGLETON_H

template <typename T>
class Singleton {
private:
    static T* m_instance;

protected:
    Singleton() {}
    virtual ~Singleton() {}

public:
    // ���� ������ �� ���� �����ڸ� �����Ͽ� ���� ����
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // ���� �޼���� �̱��� �ν��Ͻ��� ��ȯ
    static T* Instance() {
        if (m_instance == nullptr) {
            m_instance = new T();  // ���� ȣ�� �ÿ��� �ν��Ͻ� ����
        }
        return m_instance;
    }
};

template <typename T>
T* Singleton<T>::m_instance = nullptr;

#endif  // MYDIRECT_SOURCE_COMMON_SINGLETON_H