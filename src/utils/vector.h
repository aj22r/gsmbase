#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

    void push_back(const T& elem) {
        auto new_data = m_size ? (T*)realloc(m_data, (m_size + 1) * sizeof(T)) : (T*)malloc(sizeof(T));
        if(!new_data) return;
        m_data = new_data;

        memcpy(m_data + m_size, &elem, sizeof(T));
        m_size++;
    }

    T& operator[](size_t idx) {
        if(idx >= m_size) return {};
        return *(m_data + idx);
    }

    T* begin() {
        return m_data;
    }

    T* end() {
        return m_data + m_size;
    }

    void erase(T* it) {
        if(!m_size || it < begin() || it >= end()) return;

        memcpy(it, it + 1, m_size * sizeof(T) - (it - begin()) - 1 * sizeof(T));

        m_size -= 1;
        if(!m_size) {
            free(m_data);
            return;
        }

        auto new_data = (T*)realloc(m_data, m_size * sizeof(T));
        if(!new_data) return;
        m_data = new_data;
    }
};