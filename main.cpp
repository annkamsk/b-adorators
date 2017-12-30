#include "blimit.hpp"
#include "adorate_struct.h"

using std::string;

std::vector<long> b;
std::atomic<long> *db;
std::recursive_mutex *mut;
atom_queue<long> Q;
atom_queue<long> P;
std::vector<neighbours<Edge>> N;
std::vector<std::priority_queue<Edge, std::vector<Edge>, Compare>> proposals;

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
    std::lock_guard<std::recursive_mutex> lock(mut[id]);
    return proposals.at(id).size() < bvalue(method, decode.at(id));
}

Edge find_eligible(long u, long method, bool &found) {
    Edge eligible = N.at(u).back();
    found = true;

    if (is_place(eligible.id, method)) {
        return eligible;
    }
    if (bvalue(method, decode.at(eligible.id)) == 0) {
        found = false;
        return eligible;
    }
    mut[eligible.id].lock();
    Edge lowest_prop = proposals.at(eligible.id).top();
    mut[eligible.id].unlock();
    if (Edge{u, eligible.weigh} < lowest_prop) found = false;
    return eligible;
}

void make_suitor(long u, Edge p, long method) {
    --b.at(u);
    proposals.at(p.id).push(Edge{u, p.weigh});
    if (proposals.at(p.id).size() > bvalue(method, decode.at(p.id))) {
        Edge v = proposals.at(p.id).top();
        proposals.at(p.id).pop();
        if (++db[v.id] == 1) {
            P.push(v.id);
        }
    }
}

void run(long method) {
    while (!Q.empty()) {
        long u = Q.pop();
        if (u == -1) {
            break;
        }
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

void reset_containers() {
    for (auto it = N.begin(), itend = N.end(); it != itend; ++it) it->reload();
    b.clear();
}

void set_containers(long nodes_numb, std::vector<std::list<Edge>> &neighb) {
    for (auto it = neighb.begin(), endit = neighb.end(); it != endit; ++it) {
        it->sort();
    }

    proposals = std::vector<std::priority_queue<Edge,
            std::vector<Edge>, Compare>>(nodes_numb, std::priority_queue<Edge,
            std::vector<Edge>, Compare>());
    mut = new std::recursive_mutex[nodes_numb];
    db = new std::atomic<long>[nodes_numb];
    for (long i = 0; i < nodes_numb; ++i) {
        db[i] = 0;
    }
    for (auto it = neighb.begin(), itend = neighb.end(); it != itend; ++it) {
        N.emplace_back(*it);
    }
}

long count_sum() {
    long sum = 0;
    for (auto &a : proposals) {
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

    set_containers(nodes_numb, neighb);

    std::thread *th = nullptr;
    if (thread_count > 1) {
        th = new std::thread[thread_count - 1];
    }
    for (long i = 0; i <= b_limit; ++i) {

        for (long j = 0; j < nodes_numb; ++j) {
            Q.push(j);
            b.push_back(bvalue(i, decode.at(j)));
        }
        while (!Q.empty()) {
            for (long j = 0; j < thread_count - 1; ++j) {
                th[j] = std::thread(run, i);
            }
            run(i);
            for (long j = 0; j < thread_count - 1; ++j) {
                th[j].join();
            }

            Q.swap(P);
            update_b(nodes_numb);
        }
        std::cout << count_sum() << std::endl;
        reset_containers();
    }
    delete[] mut;
    delete[] db;
    delete[] th;
    return 0;
}