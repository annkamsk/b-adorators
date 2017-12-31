
#ifndef MIMUW_ADORATE_42_ADORATE_STRUCT_H
#define MIMUW_ADORATE_42_ADORATE_STRUCT_H
#include <iostream>
#include <fstream>
#include <list>
#include <unordered_map>
#include <condition_variable>

#include <algorithm>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

const int NOT_FOUND = -1;

std::vector<long> decode;

struct Edge {
    long id;
    long weigh;

    bool operator < (Edge e) {
        if (this->weigh == e.weigh) return decode.at(this->id) < decode.at(e.id);
        return this->weigh < e.weigh;
    }
};

bool comp(Edge a, Edge b) {
    return b < a;
}

template<typename T>
class neighbours {
public:
    neighbours() : it(N.rbegin()), sorted(N.rbegin()) {}

    neighbours(const neighbours &other) : N(other.N), it(N.rbegin()),
                                          sorted(N.rbegin()) {}

    neighbours(neighbours && other) = default;

    bool empty() const {
        return it == N.rend();
    }

    void set_bvalue(unsigned int b) {
        bvalue = 2 * b;
    }

    T back() {
        if (it == sorted) {
            sort();
        }
        return *it;
    }

    void pop_back() {
        ++it;
    }

    void push_back(const T &el) {
        N.push_back(el);
        it = N.rbegin();
        sorted = N.rbegin();
    }

    void sort() {
        if (sorted + bvalue > N.rend()) {
            std::sort(it, N.rend(), comp);
            sorted = N.rend();
        } else {
            std::partial_sort(it, sorted + bvalue, N.rend(), comp);
            sorted += bvalue;
        }
    }

    void reload() {
        it = N.rbegin();
    }

private:
    std::vector<T> N;
    typename std::vector<T>::reverse_iterator it;
    typename std::vector<T>::reverse_iterator sorted;
    unsigned int bvalue;
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
            return NOT_FOUND;
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
