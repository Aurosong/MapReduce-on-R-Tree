#include <fstream>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include "Rtree_on_disk/Rtree.h"
// #include "MapReduce/master.h"

void readFileToTree(const std::string& filePath) {

    std::string fileName = "./data/tree.txt";
    FileManager fm;
    FileHandler fh;

    try{
        fh = fm.createFile(fileName.c_str());
    }
    catch(...){
        std::remove(fileName.c_str());
        fh = fm.createFile(fileName.c_str()); 
    }

    RTree rt = RTree(10, fh);

    std::ifstream inFile(filePath); // 打开文件
    if (!inFile) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }
    std::string line;
    while (std::getline(inFile, line)) { // 逐行读取
        std::istringstream iss(line); // 使用字符串流解析每一行
        std::vector<int> numbers;
        int number;
        while (iss >> number) { // 读取每个整数
            numbers.push_back(number);
        }
        rt.insert(numbers, fh);
    }

    fm.closeFile(fh);
    fm.destroyFile(fileName.c_str());
    inFile.close(); // 关闭文件
}

// input, maxCap, dimension, output
// void rTreesCreator(const std::string& folderPath, int amount, int maxCap) {
//     std::string fileName = folderPath + "RTree_" + std::to_string(amount) + ".txt";
//     FileManager fm;
//     FileHandler fh;

//     try{
//         fh = fm.createFile(fileName.c_str());
//     }
//     catch(...){
//         std::remove(fileName.c_str());
//         fh = fm.createFile(fileName.c_str()); 
//     }

//     RTree rt = RTree(maxCap, fh);
//     std::vector<int> pt = {15, 25, 85, 100};
//     rt.insert(pt, fh);

//     fm.closeFile(fh);
//     fm.destroyFile(fileName.c_str());
// }

void generateFiles(const std::string& filePath, int n) {
    std::ofstream outFile(filePath);
    if (!outFile) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = 0; i < n; ++i) {
        int num1 = std::rand() % 10000;
        int num2 = std::rand() % 10000;
        int num3 = std::rand() % 10000;
        int num4 = std::rand() % 10000;
        outFile << num1 << " " << num2 << " " << num3 << " " << num4 << std::endl;
    }
    outFile.close();
}

int main(){
    std::string filePath = "./data/data0.txt";
    readFileToTree(filePath.c_str());

    // generateFiles(folderPath.c_str(), 100);


    // for(int i = 0; i < 10; i++) {
    //     std::string filePath = folderPath + "data" + to_string(i) + ".txt";
    //     generateFiles(filePath, 200);
    // }

    // rTreesCreator(folderPath, 47, 10);

    return 0;
}