#include <fstream>
#include <cstdlib>
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
    const int insertNum = 500; // data inserted into single tree
    const int workerNum = 5; // number of workers in MapReduce

    // query range
    const int queryMinX = 0;
    const int queryMinY = 0;
    const int queryMaxX = 5000;
    const int queryMaxY = 10000;

    std::vector<Rtree> dataset = dataGenerator(treeNum, insertNum);
    Rectangle myQuery(Point(queryMinX, queryMinY), Point(queryMaxX, queryMaxY));
    Job myJob(myQuery, dataset);
    Master master;
    std::vector<Rectangle> result = master.excutor(myJob, 5);


    std::string result_toStr = "====== Query Result ======\n";
    result_toStr = result_toStr + "Your Query: " + myQuery.printRect();
    if(result.size() == 0) {
        result_toStr = result_toStr + "No Result Queried\n";
    }
    for(int i = 0; i < result.size(); i++) {
        result_toStr = result_toStr + "Answer " + std::to_string(i) + " : " + 
                       result.at(i).printRect();
    }

    std::cout << result_toStr << std::endl;
    return 0;
}