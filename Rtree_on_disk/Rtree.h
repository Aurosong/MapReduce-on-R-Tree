#ifndef RTREE_H
#define RTREE_H

#include <limits>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <cmath>
#include "diskManager.h"
#include "errors.h"

// #define INT_MIN std::numeric_limits<int>::min()
// #define INT_MAX std::numeric_limits<int>::max()

#define DOUBLE_MIN std::numeric_limits<double>::min()
#define DOUBLE_MAX std::numeric_limits<double>::max()

class Node{
public:
    int pageId;                 // page no. in which node resides
    int parentId;               // page no. of parent node
    std::vector<int> MBR;
    std::vector<std::vector<int>> childMBR;
    std::vector<int> childptr;
    bool leaf;                  // leaf node or internal node
    int size;                   // no. of children in a node( current size)
    // Node(int d, int maxCap);
    Node(int maxCap);
    Node() {return;}
};

// Node::Node(int d, int maxCap) {
//     MBR = std::vector<int>(2 * d, INT_MIN);
//     childMBR = std::vector<std::vector<int>>(maxCap, std::vector<int>(2 * d, INT_MIN));
//     childptr = std::vector<int>(maxCap, -1);
//     size = 0;
// }

Node::Node(int maxCap) {
    MBR = std::vector<int>(4, INT_MIN);
    childMBR = std::vector<std::vector<int>>(maxCap, std::vector<int>(4, INT_MIN));
    childptr = std::vector<int>(maxCap, -1);
    size = 0;
}

class RTree{
public:
    // int d;        // dimension of points in R tree
    int maxCap;   // maximum no. of children in a node
    int m;        // minimum no. of children in a node
    // int M;        // maximum no. of nodes in a Page
    int rootPageId;
    int height;
    int noOfElement;
    // RTree(int dim, int maxChildren, FileHandler& fh);
    RTree(int maxChildren, FileHandler& fh);
    Node diskRead(int id,FileHandler& fh);              // read the page corresponding to the node to disk
    Node diskWrite(Node& n,FileHandler& fh);            // write the page corresponding to the node to disk
    Node allocateNode(FileHandler&,int parentid);       // Allocate page for the node
    bool equal(Node& n1,Node& n2);                      //check if two Node are equal or not just for debugging purpose
    bool deleteNode(Node& n, FileHandler& fh);          // delete the node from disk
    bool freeNode(const Node& n,FileHandler& fh);              // free node from memory still remains on disk
    void splitChild(int k,Node& n,FileHandler& fh);     // split kth child of node
    void insert(const std::vector<int>& p, FileHandler& fh);
    void insertNonFull(const std::vector<int>& p, Node& n, FileHandler& fh);
    std::vector< Node > quadraticSplit(const Node& n, FileHandler& fh);  // split a node into two and return the nodes as vector
    bool search(const std::vector<int>& p, int nodeid, FileHandler& fh);
    void bulk_load(FileHandler& fh_1, FileHandler& fh, int N);
    void assignParents(int startPageId,int lastPageId, FileHandler& fh);
    // helper functions
    // return the index of the MBRs which expands the least when p is included in it
    int leastIncreasingMBR( const std::vector<int>& p ,const std::vector<std::vector<int>>& possMBRs, int nsize);
    std::vector<int> seed(const Node& n);             // seed the QudraticSplit Algo
    bool contains(const std::vector<int>& p, const std::vector<int>& MBR);        // check if MBR contains p
    double volMBR(const std::vector<int>&);                                //volume of single MBR
    double volMBRS(const std::vector<std::vector<int>>& MBRs, int nsize); // sum of volume of all MBRs
    double deadSpace(int nsize, const std::vector<std::vector<int>>& Elist , const std::vector<int>& MBR);// wasted space in MBR containing E list MBRs
    std::vector<int> minBoundingRegion(const std::vector< std::vector<int>>& Elist, int nsize);    //  minimum bounding region of a list of MBR

    // For Debugging
    void printTree(FileHandler& fh);
    void printNode(const Node& n);
};

// RTree::RTree(int dim, int maxChildren, FileHandler &fh) {
//     d = dim;
//     maxCap = (PAGE_CONTENT_SIZE - 8 * d - 16) / (8 * d + 4);
//     maxCap = std::min(maxChildren, maxCap);
//     maxCap = std::max(3, maxCap);
//     m = (int)ceil(maxCap / 2.0);
//     height = 0;
//     Node root = allocateNode(fh, -1);
//     rootPageId = root.pageId;
//     noOfElement = 4 + 2 * d + (2 * d + 1) * maxCap;
//     root.leaf = true;
//     root.size = 0;
//     diskWrite(root, fh);
// }

RTree::RTree(int maxChildren, FileHandler &fh) {
    maxCap = (PAGE_CONTENT_SIZE - 8 * 2 - 16) / (8 * 2 + 4);
    maxCap = std::min(maxChildren, maxCap);
    maxCap = std::max(3, maxCap);
    m = (int)ceil(maxCap / 2.0);
    height = 0;
    Node root = allocateNode(fh, -1);
    rootPageId = root.pageId;
    noOfElement = 4 + 2 * 2 + (2 * 2 + 1) * maxCap;
    root.leaf = true;
    root.size = 0;
    diskWrite(root, fh);
}

Node RTree::allocateNode(FileHandler &fh, int parentId) {
    PageHandler ph = fh.newPage();
    Node n = Node(maxCap);
    n.pageId = ph.getPageNum();
    // std::cout <<"Allocated " << n.pageId << "\n";
    n.parentId = parentId;
    return n;
}

Node RTree::diskWrite(Node &n, FileHandler &fh) {
    PageHandler ph = fh.pageAt(n.pageId); // going to disk or buffer check?
    char *data = ph.getData();
    std::vector<int> v;
    v.push_back(n.pageId);
    v.push_back(n.parentId);
    v.insert(v.end(), n.MBR.begin(), n.MBR.end());
    for (int i = 0; i < maxCap; i++){
      v.insert(v.end(), n.childMBR[i].begin(), n.childMBR[i].end());
      v.insert(v.end(), n.childptr[i]);
    }
    v.push_back(n.leaf);
    v.push_back(n.size);
    memcpy(&data[0], &v[0], v.size() * sizeof(int));
    fh.markDirty(n.pageId);
    fh.unpinPage(n.pageId);
    fh.flushPage(n.pageId);
    return n;
}

Node RTree::diskRead(int id, FileHandler &fh) {
    PageHandler ph = fh.pageAt(id);
    char *data = ph.getData();
    std::vector<int> v(noOfElement);
    memcpy(&v[0], &data[0], noOfElement * sizeof(int));
    Node n = Node(maxCap);
    n.pageId = v[0];
    n.parentId = v[1];
    memcpy(&n.MBR[0], &v[2], 2 * 2 * sizeof(int));
    int idx = 2 + 2 * 2;
    for (int i = 0; i < maxCap; i++) {
        memcpy(&n.childMBR[i][0], &v[idx], 2 * 2 * sizeof(int));
        idx = idx + 2 * 2;
        memcpy(&n.childptr[i], &v[idx], sizeof(int));
        idx = idx + 1;
    }
    memcpy(&n.leaf, &v[idx], sizeof(int));
    idx += 1;
    memcpy(&n.size, &v[idx], sizeof(int));
    return n;
}

bool RTree::equal(Node &n1, Node &n2) {
  if (n1.pageId != n2.pageId) return false;
  if (n1.parentId != n2.parentId) return false;
  if (n1.MBR.size() != n2.MBR.size()) return false;
  for (int i = 0; i < n1.MBR.size(); i++) {
      if (n1.MBR[i] != n2.MBR[i]) return false;
  }
  if (n1.childptr.size() != n2.childptr.size()) return false;
  for (int i = 0; i < maxCap; i++){
      if (n1.childMBR[i].size() != n2.childMBR[i].size()) return false;
      if (n1.childptr[i] != n2.childptr[i]) return false;
      for (int j = 0; j < n1.childMBR[i].size(); j++) {
          if (n1.childMBR[i][j] != n2.childMBR[i][j])
              return false;
      }
  }
  if (n1.leaf != n2.leaf) return false;
  if (n1.size != n2.size) return false;
  return true;
}

bool RTree::deleteNode(Node &n, FileHandler &fh) {
    // std::cout << " Delete Node " << n.pageId <<"\n";
    // std::cout << " mark delete " << fh.MarkDirty(n.pageId) <<"\n";
    return (fh.disposePage(n.pageId));
}

bool RTree::contains(const std::vector<int> &p, const std::vector<int> &MBR) {
    for (int i = 0; i < 2; i++) {
        if (MBR[2 * i] > p[2 * i] || MBR[i * 2 + 1] < p[2 * i + 1]) return false;
    }
    return true;
}

double RTree::volMBR(const std::vector<int> &MBR) {
    double vol = 1.0;
    for (int i = 0; i < 2; i++){
        vol = vol * (MBR[2 * i + 1] - MBR[2 * i]);
    }
    return vol;
}

double RTree::volMBRS(const std::vector<std::vector<int>> &MBRs, int nsize) {
    double vol = 0;
    for (int i = 0; i < nsize; i++) {
        vol += volMBR(MBRs[i]);
    }
    return vol;
}

double RTree::deadSpace(int nsize, const std::vector<std::vector<int>> &Elist, const std::vector<int> &MBR) {
    double v1 = volMBRS(Elist, nsize);
    double v2 = volMBR(MBR);
    return (v2 - v1);
}

std::vector<int> RTree::minBoundingRegion(const std::vector<std::vector<int>> &Elist, int nsize) {
    std::vector<int> mbr(2 * 2);
    for (int i = 0; i < 2; i++) {
        mbr[2 * i] = INT_MAX;
        mbr[2 * i + 1] = INT_MIN;
        for (int e = 0; e < nsize; e++) {
            mbr[2 * i] = std::min(Elist[e][2 * i], mbr[2 * i]);
            mbr[2 * i + 1] = std::max(Elist[e][2 * i + 1], mbr[2 * i + 1]);
        }
    }
    return mbr;
}

bool RTree::freeNode(const Node &n, FileHandler &fh) {
    bool res = fh.unpinPage(n.pageId);
    res = res && fh.flushPage(n.pageId);
    return (res);
}

int RTree::leastIncreasingMBR(const std::vector<int> &p, const std::vector<std::vector<int>> &possMBRs, int nsize) {
    double minInc = std::numeric_limits<double>::max();
    int idx = -1;
    for (int i = 0; i < nsize; i++) {
        double inc = volMBR(minBoundingRegion({p, possMBRs[i]}, 2)) - volMBR(possMBRs[i]);
        if (minInc > inc) {
            minInc = inc;
            idx = i;
        }
    }
    return idx;
}

void RTree::insert(const std::vector<int> &p, FileHandler &fh) {
    Node r = diskRead(rootPageId, fh);
    if (r.size == maxCap) {
        Node s = allocateNode(fh, -1);
        s.leaf = false;
        s.size = 1;
        s.childptr[0] = r.pageId;
        r.parentId = s.pageId;
        s.MBR = r.MBR;
        s.childMBR[0] = r.MBR;
        diskWrite(r, fh);
        splitChild(0, s, fh);
        s = diskRead(s.pageId, fh);
        rootPageId = s.pageId;
        height += 1;
        insertNonFull(p, s, fh);
    } else insertNonFull(p, r, fh);
}

void RTree::insertNonFull(const std::vector<int> &p, Node &n, FileHandler &fh) {
    if (!n.leaf) {
        int idx = leastIncreasingMBR(p, n.childMBR, n.size);
        Node ch = diskRead(n.childptr[idx], fh);
        if (ch.size == maxCap) {
            freeNode(ch, fh);
            splitChild(idx, n, fh);
            n = diskRead(n.pageId, fh);
            if (n.size == maxCap) {
                freeNode(n, fh);
                insert(p, fh);
            } else insertNonFull(p, n, fh);
            return;
        }
        n.childMBR[idx] = minBoundingRegion({p, n.childMBR[idx]}, 2);
        n.MBR = minBoundingRegion(n.childMBR, n.size);
        diskWrite(n, fh);
        insertNonFull(p, ch, fh);
        return;
    } else {
        n.childMBR[n.size] = p;
        n.size += 1;
        n.MBR = minBoundingRegion(n.childMBR, n.size);
        diskWrite(n, fh);
        return;
    }
}

void RTree::splitChild(int k, Node &n, FileHandler &fh) {
    int id = n.childptr[k];
    Node ch = diskRead(id, fh);
    std::vector<Node> div = quadraticSplit(ch, fh);
    Node n1, n2;
    n1 = div[0], n2 = div[1];
    n.childptr[k] = n1.pageId;
    n.childMBR[k] = n1.MBR;
    n.childptr[n.size] = n2.pageId;
    n.childMBR[n.size] = n2.MBR;
    n.size += 1;
    deleteNode(ch, fh);
    diskWrite(n, fh);
    diskWrite(n1, fh);
    diskWrite(n2, fh);
}

std::vector<int> RTree::seed(const Node &n) {
    double maxdiff = -1;
    int e1, e2;
    e1 = e2 = -1;
    for (int i = 0; i < n.size; i++) {
        for (int j = i + 1; j < n.size; j++) {
            double val = deadSpace(2, {n.childMBR[i], n.childMBR[j]}, minBoundingRegion({n.childMBR[i], n.childMBR[j]}, 2));
            if (maxdiff < val) {
                maxdiff = val;
                e1 = i;
                e2 = j;
            }
        }
    }
    return (std::vector<int>({e1, e2}));
}

std::vector<Node> RTree::quadraticSplit(const Node &n, FileHandler &fh) {
    int e1, e2;
    std::vector<int> seedv = seed(n);
    e1 = seedv[0], e2 = seedv[1];
    std::vector<int> L1, L2;
    L1.push_back(e1);
    L2.push_back(e2);
    std::vector<int> E;
    for (int i = 0; i < n.size; i++) {
        if (i != e1 && i != e2) E.push_back(i);
    }
    std::vector<std::vector<int>> mbrL1, mbrL2;
    mbrL1.push_back(n.childMBR[e1]);
    mbrL2.push_back(n.childMBR[e2]);
    while (!E.empty()) {
        double maxdiff = -1;
        int idx = -1;
        int didx = -1;
        int count = 0;
        std::vector<std::vector<int>> tp1, tp2;
        for (auto i : E) {
            double d1, d2;
            std::vector<std::vector<int>> temp1, temp2;
            temp1 = mbrL1;
            temp2 = mbrL2;
            temp1.push_back(n.childMBR[i]);
            temp2.push_back(n.childMBR[i]);
            d1 = deadSpace(temp1.size(), temp1, minBoundingRegion(temp1, temp1.size()));
            d2 = deadSpace(temp2.size(), temp2, minBoundingRegion(temp2, temp2.size()));
            if (std::abs(d1 - d2) > maxdiff) {
                idx = i;
                didx = count;
                maxdiff = std::abs(d1 - d2);
                tp1 = temp1;
                tp2 = temp2;
            }
            count++;
        }
        double d1 = volMBRS({minBoundingRegion(tp1, tp1.size())}, 1);
        d1 -= volMBRS(mbrL1, mbrL1.size());
        double d2 = volMBRS({minBoundingRegion(tp2, tp2.size())}, 1);
        d2 -= volMBRS(mbrL2, mbrL2.size());
        if (d1 < d2) {
            mbrL1 = tp1;
            L1.push_back(idx);
        } else {
            mbrL2 = tp2;
            L2.push_back(idx);
        }
        E.erase(E.begin() + didx);
    }
    Node n1, n2;
    n1 = allocateNode(fh, n.parentId);
    n2 = allocateNode(fh, n.parentId);
    for (auto i : L1) {
      n1.childMBR[n1.size] = n.childMBR[i];
      n1.childptr[n1.size] = n.childptr[i];
      n1.size += 1;
    }
    for (auto i : L2) {
      n2.childMBR[n2.size] = n.childMBR[i];
      n2.childptr[n2.size] = n.childptr[i];
      n2.size += 1;
    }
    n1.leaf = n.leaf;
    n2.leaf = n.leaf;
    n1.MBR = minBoundingRegion(n1.childMBR, n1.size);
    n2.MBR = minBoundingRegion(n2.childMBR, n2.size);
    return (std::vector<Node>({n1, n2}));
}

bool RTree::search(const std::vector<int> &p, int nodeid, FileHandler &fh) {
    Node n = diskRead(nodeid, fh);
    bool find = false;
    for (int i = 0; i < n.size && !find; i++) {
        if (contains(p, n.childMBR[i])) {
            int childId = n.childptr[i];
            if (!n.leaf) find = find || search(p, childId, fh);
            else find = find || true;
        }
    }
    freeNode(n, fh);
    return find;
}

void RTree::printTree(FileHandler &fh) {
    try {
        PageHandler ph = fh.lastPage();
        ph = fh.firstPage();
        while (1) {
            int id = ph.getPageNum();
            fh.unpinPage(id);
            Node n = diskRead(id, fh);
            ph = fh.nextPage(id);
        }
    } catch (InvalidPageException) {
        std::cout << " File Ended\n";
    }
}

void RTree::printNode(const Node &n) {
    std::cout << "\n==============Start Node=============\n";
    std::cout << "PageId " << n.pageId << "\n";
    std::cout << "ParentId " << n.parentId << "\n";
    std::cout << "MBR ";
    for (auto &w : n.MBR) {
        std::cout << w << " ";
    }
    std::cout << "\n";
    for (int i = 0; i < maxCap; i++) {
        std::cout << "MBR of child " << n.childptr[i] << " -> \n";
        for (auto &w : n.childMBR[i]) {
            std::cout << w << " ";
        }
        std::cout << "\n";
    }
    std::cout << " Is Leaf " << n.leaf << "\n";
    std::cout << " Size " << n.size << "\n";
    std::cout << "==============End Node=============\n\n";
}

#endif
