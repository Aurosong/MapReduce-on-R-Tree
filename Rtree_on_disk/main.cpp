#include <fstream>
#include <cstdlib>
#include "Rtree_on_disk/Rtree.h"


int main(){
  std::ifstream inp;
  std::ofstream out;
  FileManager fm;
  FileHandler fh;
  try{
    fh = fm.createFile("./data/TC_1/RTREE_INDEX.txt");
  }
  catch(...){
    std::remove("./data/TC_1/RTREE_INDEX.txt");
    fh = fm.createFile("./data/TC_1/RTREE_INDEX.txt"); 
  }

  inp.open("./data/TC_1/queries.txt");
  out.open("./data/TC_1/answer.txt");
  int maxCap = 10;
  int dimensionality = 2;
  RTree rt = RTree(maxCap,fh);
  std::string line;
  while(inp >> line){
    if( line == "INSERT"){
      std::vector< int > p(2*dimensionality);
      for(int i = 0; i < dimensionality ; i++){
        inp >> p[2*i];
        p[2*i + 1] = p[2*i];
      }
      out << "INSERT\n\n";
      rt.insert(p,fh);
    }
    else if( line == "QUERY"){
      std::vector< int > p(2*dimensionality);
      for(int i = 0; i < dimensionality ; i++){
        inp >> p[2*i];
        p[2*i + 1] = p[2*i];
      }
      if (rt.search(p,rt.rootPageId,fh)) out << "TRUE\n\n";
      else out << "FALSE\n\n";
    }
    else std::cerr <<"Input file incorrect\n";
  }
  inp.close();
  out.close();
  fm.closeFile(fh);
  fm.destroyFile("./data/TC_1/RTREE_INDEX.txt");
  // std::cout <<"Excution finished with success\n";
  return 0;
}
