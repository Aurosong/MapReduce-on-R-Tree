#include <fstream>
#include <cstdlib>
#include <chrono>
#include "Rtree/RTree.h"
#include "MapReduce/master.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

std::vector<Rtree> dataGenerator(int dataSize, int subSize) {
    std::vector<Rtree> dataset;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for(int i = 0; i < dataSize; i++) {
        std::vector<std::vector<double>> pointSubSet(subSize, std::vector<double>(4));
        for(int j = 0; j < subSize; j++) {
            for(int k = 0; k < 4; k++) {
                pointSubSet[j][k] = static_cast<double>(std::rand() % 10001);
            }
        }
        Rtree *tree = new Rtree();
        tree -> initite(-1, 0, 0, 10);

        for(int l = 0; l < pointSubSet.size(); l++) {
            Point pt1(pointSubSet[l][0], pointSubSet[l][1]);
            Point pt2(pointSubSet[l][0] + pointSubSet[l][2], pointSubSet[l][1] + pointSubSet[l][3]);
            Rectangle rect(pt1, pt2);
            tree -> insertNode(rect, -2);
        }
        dataset.emplace_back(*tree);
    }
    return dataset;
}

int main() {

    const int treeNum = 10; // number of R-Trees
    const int insertNum = 1000; // data inserted into single tree
    const int workerNum = 5; // number of workers in MapReduce

    // query range
    const int queryMinX = 0;
    const int queryMinY = 0;
    const int queryMaxX = 5000;
    const int queryMaxY = 10000;

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Rtree> dataset = dataGenerator(treeNum, insertNum);
    auto build_end = std::chrono::high_resolution_clock::now();

    Rectangle myQuery(Point(queryMinX, queryMinY), Point(queryMaxX, queryMaxY));
    Job myJob(myQuery, dataset);
    Master master;
    std::vector<Rectangle> result = master.excutor(myJob, 5);
    auto query_end = std::chrono::high_resolution_clock::now();

    std::string result_toStr = "====== Query Result ======\n";
    result_toStr = result_toStr + "Your Query: " + myQuery.printRect();
    if(result.size() == 0) {
        result_toStr = result_toStr + "No Result Queried\n";
    }
    for(int i = 0; i < result.size(); i++) {
        result_toStr = result_toStr + "Answer " + std::to_string(i) + " : " + 
                       result.at(i).printRect();
    }

    std::string time = "Program Supervisor: \n";
    std::chrono::duration<double> build_time = build_end - start;
    std::chrono::duration<double> query_time = query_end - build_end;
    time = time + "Data Amount: " + std::to_string(insertNum) + "\n";
    time = time + "Build Time: " + std::to_string(build_time.count()) + "\n";
    time = time + "Query Time: " + std::to_string(query_time.count()) + "\n";

    std::cout << result_toStr << std::endl;
    std::cout << time << std::endl;
    return 0;
}