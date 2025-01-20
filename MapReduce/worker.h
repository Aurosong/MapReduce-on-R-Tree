#ifndef MY_WORKER
#define MY_WORKER

#include "job.h"

class Worker {
public:
    Worker() {};
    std::map<int, Rectangle> mapping_result;
    std::vector<Rectangle> reducing_result;
    void mapper(const Job job);
    void shuffle_and_reduce(std::vector<Worker> workers, int queriedKey);
};

// worker work as mapper, deal with their sub-job and create key-value
void Worker :: mapper(const Job job) {
    Rectangle subQuery = job.query;
    // std::vector<std::vector<Rectangle>> query_result;
    std::vector<Rectangle> query_result;
    for(Rtree tree : job.data) {
        Node* root = tree.getRoot();
        // std::map<int, Node*> abc = tree.nodeMap;
        std::vector<Rectangle> subResult = root -> queryRect(subQuery);
        // query_result.push_back(subResult);
        query_result.insert(query_result.end(), subResult.begin(), subResult.end());
    }
    // add key to the result
    for(Rectangle queryRect : query_result) {
        this -> mapping_result.emplace(std::make_pair(1, queryRect));
    }
}

// worker work as reducer, shuffle the result of mapper according to their key, then reduce
void Worker :: shuffle_and_reduce(std::vector<Worker> workers, int queriedKey) {
    for(Worker worker : workers) {
        for(const auto pair : worker.mapping_result) {
            if(pair.first == queriedKey) this -> reducing_result.push_back(pair.second);
        }
    }
}

#endif