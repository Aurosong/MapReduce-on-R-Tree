#ifndef MY_MASTER
#define MY_MASTER

#include "worker.h"

class Master {
public:
    Job preProcessor(Job job);
    std::vector<Rectangle> excutor(Job& job, int workerNums);
    std::vector<Job> splitJob(Job job, int workerNums);
};

std::vector<Rectangle> Master :: excutor(Job& job, int workerNums) {
    // 1. pre-process job
    Job optJob = preProcessor(job);
    // 2. split the job into sub-jobs
    std::vector<Job> subJobs = splitJob(optJob, workerNums);
    // 3. workers receive sub-job and start mapping
    std::vector<Worker> workers(workerNums);
    for(int i = 0; i < workerNums; i++) {
        workers.at(i).mapper(subJobs.at(i));
    }
    // 4. reducer start shuffle and reduce, workers[0] as reducer;
    workers[0].shuffle_and_reduce(workers, 1);
    return workers[0].reducing_result;
}

// preprocess the job
Job Master :: preProcessor(Job job) {
    std::vector<Rectangle> finalRectSet;
    for(Rtree tree : job.data) {
        finalRectSet.push_back(tree.getFinalRect());
    }
    Rectangle finalUnionRect = Rectangle::unionRects(finalRectSet);
    job.query = job.query.intersectRect(finalUnionRect);
    return job;
}

// divide the job into sub-jobs, which is done in the first step of MapReduce
std::vector<Job> Master :: splitJob(Job job, int workerNums) {
    std::vector<Job> subJobs;
    Rectangle baseQuery = job.query;
    if(baseQuery.splitAxis()) {
        int interval = (baseQuery.high.x - baseQuery.low.x) / workerNums;
        Point currLow = baseQuery.low;
        for(int i = 0; i < workerNums; i++) {
            Point currHigh(currLow.x + interval, baseQuery.high.y);
            Rectangle currRect(currLow, currHigh);
            subJobs.push_back(Job(currRect, job.data));
            currLow.x += interval;
        }
    } else {
        int interval = (baseQuery.high.y - baseQuery.low.y) / workerNums;
        Point currLow = baseQuery.low;
        for(int i = 0; i < workerNums; i++) {
            Point currHigh(baseQuery.high.x, currLow.y + interval);
            Rectangle currRect(currLow, currHigh);
            subJobs.push_back(Job(currRect, job.data));
            currLow.y += interval;
        }
    }
    return subJobs;
}

#endif