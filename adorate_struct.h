
#ifndef MIMUW_ADORATE_42_ADORATE_STRUCT_H
#define MIMUW_ADORATE_42_ADORATE_STRUCT_H
#include <iostream>
#include <fstream>
#include <list>
#include <unordered_map>

#include <algorithm>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>


std::vector<long> decode;

struct Edge {
    long id;
    long weigh;

    bool operator < (Edge e) {
        if (this->weigh == e.weigh) return decode.at(this->id) < decode.at(e.id);
        return this->weigh < e.weigh;
    }
};

template<typename T>
class neighbours {
public:
    neighbours() = default;
    neighbours(std::list<T> &other) : N(other), it(N.rbegin()){}

    neighbours(const neighbours &other) : N(other.N), it(N.rbegin()) {}

    neighbours(neighbours && other) = default;

    bool empty() const {
        return it == N.rend();
    }

    T back() {
        return *it;
    }

    void pop_back() {
        ++it;
    }

    void push_back(T &el) {
        N.push_back(el);
        it = N.rbegin();
    }

    void reload() {
        it = N.rbegin();
    }

private:
    std::list<T> N;
    typename std::list<T>::reverse_iterator it;
};

template<typename T>
class atom_queue {
public:
    void push(const T& value ) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
    }

    long pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return -1;
        }
        long u = m_queue.front();
        m_queue.pop();
        return u;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void swap(atom_queue &Q) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(Q.m_queue, m_queue);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            m_queue.pop();
        }
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
};

class Compare {
public:
    bool operator() (Edge a, Edge b) {
        return b < a;
    }
};


#endif //MIMUW_ADORATE_42_ADORATE_STRUCT_H
