#ifndef MY_JOB
#define MY_JOB

#include "../Rtree/Rtree.h"

class Job {
public:
    Rectangle query; // query in the job, represented by a rectangle
    std::vector<Rtree> data; // corresponding data in the job, set of R-Tree

    Job() {};
    Job(Rectangle _query, std::vector<Rtree> _data) : query(_query), data(_data) {}
};


#endif