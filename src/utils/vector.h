#include <stdlib.h>
#include <stdint.h>

template<typename T>
class Vector {
private:
    size_t m_size;
    T* m_data;

public:
    Vector() : m_size(0), m_data(NULL) {}
    ~Vector() {
        if(m_data) free(m_data);
    }

    void push_back(T elem) {
        auto new_data = m_size ? (T*)realloc(m_data, (m_size + 1) * sizeof(T)) : (T*)malloc(sizeof(T));
        if(!new_data) return;
        m_data = new_data;

        *(m_data + m_size) = elem;
        m_size++;
    }

    T operator[](size_t idx) {
        if(idx >= m_size) return {};
        return *(m_data + idx);
    }

    T* begin() {
        return m_data;
    }

    T* end() {
        return m_data + m_size;
    }
};