#ifndef MYDIRECT_SOURCE_COMMON_SINGLETON_H
#define MYDIRECT_SOURCE_COMMON_SINGLETON_H

template <typename T>
class Singleton {
private:
    static T* m_instance;

protected:
    Singleton() {}
    virtual ~Singleton() 
    {
        if (m_instance != nullptr)
            delete m_instance;
    }

public:
    // 복사 생성자 및 대입 연산자를 삭제하여 복사 방지
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // 정적 메서드로 싱글톤 인스턴스를 반환
    static T* Instance() {
        if (m_instance == nullptr) {
            m_instance = new T();  // 최초 호출 시에만 인스턴스 생성
        }
        return m_instance;
    }
};

template <typename T>
T* Singleton<T>::m_instance = nullptr;

#endif  // MYDIRECT_SOURCE_COMMON_SINGLETON_H