#include <iostream>
#include <fstream>
#include <list>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <atomic>
#include <algorithm>
#include <thread>

#include "blimit.hpp"

using std::string;
using std::cout;
using std::endl;

std::vector<long> b;
std::atomic<long> *db;
std::vector<long> decode;
std::atomic<bool> *is_in_P;
std::mutex *mut;

struct Edge {
    long id;
    long weigh;

    bool operator < (Edge e) {
        if (this->weigh == e.weigh) return decode.at(this->id) < decode.at(e.id);
        return this->weigh < e.weigh;
    }

    bool operator == (Edge e) {
        return id == e.id && weigh == e.weigh;
    }

    Edge(long id, long weigh) : id(id), weigh(weigh) {}
};



template<typename T>
class atom_queue {
public:
    void push(const T& value ) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queque.push(value);
    }

    long pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        long u = m_queque.front();
        m_queque.pop();
        return u;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queque.empty();
    }

    void swap(atom_queue &Q) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(Q.m_queque, m_queque);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queque.empty()) {
            m_queque.pop();
        }
    }

private:
    std::queue<T> m_queque;
    mutable std::mutex m_mutex;
};

atom_queue<long> Q;
atom_queue<long> P;
std::vector<std::list<Edge>> N;

class Compare {
public:
    bool operator() (Edge a, Edge b) {
        return b < a;
    }
};
std::vector<std::priority_queue<Edge, std::vector<Edge>, Compare>> proposals; // adorators of v

void split(string line, long &idx1, long &idx2, long &weigh) {
    size_t it1 = line.find(' ');
    idx1 = std::stol(line.substr(0, it1));
    size_t it2 = line.substr(it1 + 1).find(' ');
    idx2 = std::stol(line.substr(it1 + 1, it2));
    weigh = std::stol(line.substr(it1 + it2 + 1));
}

void read_graph(std::fstream &graph,
                std::vector<std::list<Edge>> &neighb,
                long &nodes_numb) {
    std::string line;
    std::unordered_map<long, long> code;
    long i = -1;
    while (std::getline(graph, line)) {
        if (line[0] != '#') {
            long idx1, idx2, weigh;
            split(line, idx1, idx2, weigh);
            if (code.find(idx1) == code.end()) {
                code[idx1] = ++i;
                decode.push_back(idx1);
                neighb.push_back(std::list<Edge>());
                ++nodes_numb;
            }
            if (code.find(idx2) == code.end()) {
                code[idx2] = ++i;
                decode.push_back(idx2);
                neighb.push_back(std::list<Edge>());
                ++nodes_numb;
            }
            neighb[code[idx1]].push_back(Edge{code[idx2], weigh});
            neighb[code[idx2]].push_back(Edge{code[idx1], weigh});
        }
    }
}
bool is_place(long id, long method) {
    return proposals.at(id).size() < bvalue(method, decode.at(id));
}

Edge find_eligible(long u, long method, bool &found) {
    Edge eligible = N.at(u).back();
    found = true;
//    Edge eligible = {-1, -1};
//    for (Edge p : N.at(u)) {
//        if (eligible < p) {
//            eligible = p;
//        }
//    }

    if (is_place(eligible.id, method)) {
        return eligible;
    }
    if (bvalue(method, decode.at(eligible.id)) == 0) {
        found = false;
        return eligible;
    }
    Edge lowest_prop = proposals.at(eligible.id).top();
    if (lowest_prop < Edge{u, eligible.weigh}) return eligible;
    found = false;
    return eligible;
}

void make_suitor(long u, Edge p, long method) {
    --b.at(u);
    if (proposals.at(p.id).size() == bvalue(method, decode.at(p.id))) {
        Edge v = proposals.at(p.id).top();
        proposals.at(p.id).pop();
        ++db[v.id];
        if (!is_in_P[v.id]) {
            is_in_P[v.id] = true;
            P.push(v.id);
        }
    }
    proposals.at(p.id).push(Edge{u, p.weigh});
}

void run(long method) {
    while (!Q.empty()) {
        long u = Q.pop();
        while (b.at(u) > 0 && !N.at(u).empty()) {
            bool found = true;
            Edge p = find_eligible(u, method, found);
            if (found) {
                mut[p.id].lock();
                Edge q = find_eligible(u, method, found);
                if ((q.id == p.id) && found) {
                    make_suitor(u, p, method);
                }
                mut[p.id].unlock();
            }
            N.at(u).pop_back();
        }
    }
}

void reset_containers(std::vector<std::list<Edge>> &neighb, long nodes_numb) {
    N = std::vector<std::list<Edge>>(neighb);
    b.clear();
    Q.clear();
    P.clear();
    proposals = std::vector<std::priority_queue<Edge,
            std::vector<Edge>, Compare>>(nodes_numb, std::priority_queue<Edge,
            std::vector<Edge>, Compare>());
}

void set_containers(long nodes_numb, std::vector<std::list<Edge>> &neighb) {
    proposals = std::vector<std::priority_queue<Edge,
            std::vector<Edge>, Compare>>(nodes_numb, std::priority_queue<Edge,
            std::vector<Edge>, Compare>());
    mut = new std::mutex[nodes_numb];
    db = new std::atomic<long>[nodes_numb];
    for (long i = 0; i < nodes_numb; ++i) {
        db[i] = 0;
    }
    is_in_P = new std::atomic<bool>[nodes_numb];
    for (long i = 0; i < nodes_numb; ++i) {
        is_in_P[i] = false;
    }
    N = std::vector<std::list<Edge>>(neighb);
}

long count_sum() {
    long sum = 0;
    for (auto a : proposals) {
        while (!a.empty()) {
            sum += a.top().weigh;
            a.pop();
        }
    }
    return sum / 2;
}

void update_b(long nodes_numb) {
    for (long i = 0; i < nodes_numb; ++i) {
        b.at(i) += db[i];
        db[i] = 0;
    }
    for (long i = 0; i < nodes_numb; ++i) {
        is_in_P[i] = false;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "usage: "<<argv[0]<<" thread-count inputfile b-limit"<< std::endl;
        return 1;
    }

    int thread_count = std::stoi(argv[1]);
    int b_limit = std::stoi(argv[3]);
    std::string input_filename{argv[2]};

    std::fstream graph;
    graph.open(input_filename, std::ios::in);

    if (!graph.is_open()) {
        std::cerr << "Could not open the file.";
        return 1;
    }

    long nodes_numb = 0;

    std::vector<std::list<Edge>> neighb;
    read_graph(graph, neighb, nodes_numb);

    for (auto it = neighb.begin(), endit = neighb.end(); it != endit; ++it) {
        it->sort();
    }
    set_containers(nodes_numb, neighb);

    for (long i = 0; i <= b_limit; ++i) {

        for (long j = 0; j < nodes_numb; ++j) {
            Q.push(j);
            b.push_back(bvalue(i, decode.at(j)));
        }
        while (!Q.empty()) {
            std::thread *th = NULL;
            if (thread_count > 1) {
                th = new std::thread[thread_count - 1];
            }
            for (long j = 0; j < thread_count - 1; ++j) {
                th[j] = std::thread(run, i);
            }
            run(i);
            for (long j = 0; j < thread_count - 1; ++j) {
                th[j].join();
            }

            Q.swap(P);
            update_b(nodes_numb);
            delete[] th;
        }
        std::cout << count_sum() << std::endl;
        reset_containers(neighb, nodes_numb);
    }
    delete[] mut;
    delete[] db;
    delete[] is_in_P;

    return 0;
}