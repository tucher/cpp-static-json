#ifndef UTILS_H
#define UTILS_H
#include <iostream>
class Iter {
    char *m_d = 0;
    std::size_t m_size = 0;
    std::size_t pos = 0;
    char * current;
public:

    Iter( char *d, std::size_t size, std::size_t offs = 0):m_d(d), m_size(size), pos(offs) {
        (void)m_size;
    }
    void operator++() {
        pos++;
        current = ( m_d + pos);
    }
//    Iter& operator+=(std::size_t offs) {
//        pos+=offs;
//        return *this;
//    }
    inline const char & operator*() const {
//        std::cout << "Read at " << pos << std::endl;
        return *( m_d + pos);
    }
    inline char & operator*() {
//        std::cout << "Write at " << pos << std::endl;
        return *( m_d + pos);
    }
    bool operator==(Iter other) {return other.m_d==m_d&&other.pos == pos;}
    bool operator!=(Iter other) {return other.m_d!=m_d||other.pos != pos;}
    std::size_t operator-(const Iter& other) {return pos - other.pos;}

};
#endif // UTILS_H
