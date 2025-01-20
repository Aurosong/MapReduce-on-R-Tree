#ifndef MY_NODE
#define MY_NODE

#include "./config.h"

class Node;

class Rtree {
    friend class Node;
public:
    const int MAX_NODE_SPACE; // maximum node capacity
    int PAGE_COUNTER; // counter for page, everytime a new node constructed, this variable add 1
    std::map<int, Node*> nodeMap; // use std::map to store nodes in the tree

    Rtree();
    Rtree(const Rtree& other);
    Rtree(Rtree&& other) noexcept ;
    Rtree& operator=(const Rtree& other);
    Rtree& operator=(const Rtree&& other);
    // ~Rtree();
    void initite(int parent, int pageId, int level, int nodeSpace);
    void insertNode(Rectangle rect, int page);
    Node* getRoot();
    Node* chooseLeaf(Rectangle rect, Node* node);
    int findLeastGrowth(Rectangle rect, Node* node);
    Node* nextPageNumber(Node* node);
    Node** leafSplit(Node* leaf, Rectangle rect, int page);
    std::vector<std::vector<int>> QuadraticSplit(Node* leaf, Rectangle rect, int page);
    // std::vector<Rectangle> queryRect(Rectangle queryRect);
    int* QuadraticPickSeeds(Node* node);
    // int deleteNode(Rectangle rect);
    // void mergeTree(std::list<Node> list, Node node);
    Rectangle getFinalRect();
    // std::vector<Node> postOrder(Node root);
};

class Node {
public:
    int level; // level of a node, 
    int pageId; // unique mark of a node, 0 for root
    int rectNums; // counter, represent the number of entries in the node
    std::vector<Rectangle> data;
    std::vector<int> childId;
    int parent; // parent node's pageId
    Rtree* tree; // call back pointer to the tree the node belongs to

    Node();
    Node(int parent, int pageId, int level, int nodeSpace, Rtree* _tree);
    Node(const Node& other);
    Node(Node&& other) noexcept ;
    Node& operator = (const Node& other);
    Rectangle getNodeRectangle();
    void addData(Rectangle rect, int pageId);
    Node* getParent();
    Node* getChild(int index);
    Node* findLeaf(Rectangle rect);
    void adjustTree(Node* node1, Node* node2);
    bool insert(Node* node);
    bool isRoot();
    bool isLeaf();
    Node** splitIndex(Node* node);
    std::vector<Rectangle> queryRect(Rectangle rect);
    void printNode();
};

std::map<int, Node*> nodeMap;
int PAGE_COUNTER = 0;
const int MAX_NODE_SPACE = 10;

Node :: Node() {}

Node :: Node(int _parent, int _pageNumber, int _level, int _nodeSpace, Rtree* _tree) {
    this -> parent = _parent;
    this -> pageId = _pageNumber;
    this -> level = _level;
    this -> rectNums = 0;
    this -> tree = _tree;
    this -> data.assign(_nodeSpace, Rectangle(Point(), Point()));
    this -> childId.assign(_nodeSpace, -1);
}

Node :: Node(const Node& other) : level(other.level), pageId(other.pageId), rectNums(other.rectNums),
                          data(other.data), childId(other.childId), parent(other.parent),
                          tree(other.tree) {}

Node :: Node(Node&& other) noexcept : level(other.level), pageId(other.pageId), 
                                      rectNums(other.rectNums), data(std::move(other.data)), 
                                      childId(std::move(other.childId)), parent(other.parent), tree(other.tree) {
                                        other.tree = nullptr;
                                      }

Node& Node :: operator = (const Node& other) {
    if (this != &other) {
        level = other.level;
        pageId = other.pageId;
        rectNums = other.rectNums;
        data = other.data;
        childId = other.childId;
        parent = other.parent;
        tree = other.tree;
    }
    return *this;
}

// return the minimum bounding rectangle (MBR) of the invoking node
Rectangle Node :: getNodeRectangle() {
    if(this -> rectNums > 0) {
        // Rectangle result = Rectangle::unionRects(this -> data);
        std::vector<Rectangle> subset(this -> data.begin(), this -> data.begin() + this -> rectNums);
        Rectangle result = Rectangle::unionRects(subset);
        return result;
    } else {
        Point emptyPoint(0.0, 0.0);
        Rectangle emptyRect(emptyPoint, emptyPoint);
        return emptyRect;
    }
}

// add an entry into the node
void Node :: addData(Rectangle rect, int pageId) {
    this -> data[rectNums] = rect;
    this -> childId[rectNums] = pageId;
    this -> rectNums++;
}

// return the parent of the invoker
Node* Node :: getParent() {
    if(isRoot()) return nullptr;
    else return tree -> nodeMap[this -> parent];
    // else return nodeMap.at(this -> parent);
}

// when a node splits into two, its parent invoke this funtion to adjust the tree's structure
void Node :: adjustTree(Node* node1, Node* node2) {
    for(int i = 0; i < this -> rectNums; i++) {
        if(this -> childId[i] == node1 -> pageId) {
            this -> data[i] = node1 -> getNodeRectangle();
            tree -> nodeMap.insert_or_assign(this -> pageId, this);
            break;
        }
    }
    if(node2 == nullptr) {
        tree -> nextPageNumber(this);
    }
    if(node2 != nullptr) {
        insert(node2);
    } else if(!isRoot()) {
        Node* parent = tree -> nodeMap.at(this -> parent);
        parent -> adjustTree(this, nullptr);
    }
}

// insert a node entry into invoker
bool Node :: insert(Node* node) {
    if(rectNums < tree -> MAX_NODE_SPACE) {
        data[rectNums] = node -> getNodeRectangle();
        childId[rectNums] = node -> pageId;
        rectNums += 1;
        node -> parent = pageId;
        tree -> nextPageNumber(node);
        tree -> nextPageNumber(this);

        Node* parent = getParent();
        if(parent != nullptr) {
            parent -> adjustTree(this, nullptr);
        }
        return false;
    } else {
        Node** splitedIndex = splitIndex(node);
        Node* n1 = splitedIndex[0];
        Node* n2 = splitedIndex[1];

        if(isRoot()) {
            n1 -> parent = 0;
            n1 -> pageId = -1;
            n2 -> parent = 0;
            n2 -> pageId = -1;

            int p = tree -> nextPageNumber(n1) -> pageId;
            for(int i = 0; i < n1 -> rectNums; i++) {
                Node* ch = n1 -> getChild(i);
                ch -> parent = p;
                tree -> nextPageNumber(ch);
            }
            p = tree -> nextPageNumber(n2) -> pageId;
            for(int i = 0; i < n2 -> rectNums; i++) {
                Node* ch = n2 -> getChild(i);
                ch -> parent = p;
                tree -> nextPageNumber(ch);
            }

            Node* newRoot = new Node(-1, 0, level + 1, tree -> MAX_NODE_SPACE, tree);
            newRoot -> addData(n1 -> getNodeRectangle(), n1 -> pageId);
            newRoot -> addData(n2 -> getNodeRectangle(), n2 -> pageId);
            tree -> nextPageNumber(newRoot);
        } else {
            n1 -> pageId = pageId;
            n1 -> parent = parent;
            n2 -> pageId = -1;
            n2 -> parent = parent;
            tree -> nextPageNumber(n1);
            int j = tree -> nextPageNumber(n2) -> pageId;
            for(int i = 0; i < n2 -> rectNums; i++) {
                Node* ch = n2 -> getChild(i);
                ch -> parent = j;
                tree -> nextPageNumber(ch);
            }
            Node* p = getParent();
            p -> adjustTree(n1, n2);
        }
    }
    return true;
}

// used to split index
Node** Node :: splitIndex(Node* node) {
    std::vector<std::vector<int>> group = (new Rtree()) -> QuadraticSplit(this, node -> getNodeRectangle(), node ->pageId);
    Node* index1 = new Node(parent, pageId, level, tree -> MAX_NODE_SPACE, tree);
    Node* index2 = new Node(parent, -1, level, tree -> MAX_NODE_SPACE, tree);
    std::vector<int> group1 = group[0];
    std::vector<int> group2 = group[1];

    for(int i = 0; i < group1.size(); i++) {
        index1 -> addData(data[group1[i]], childId[group1[i]]);
    }
    for(int i = 0; i < group2.size(); i++) {
        index2 -> addData(data[group2[i]], childId[group2[i]]);
    }

    Node** result = new Node*[2];
    result[0] = index1;
    result[1] = index2;

    return result;
}

// used to issuing a query in current node, recursively find the result
std::vector<Rectangle> Node :: queryRect(Rectangle queryRect) {
    // this -> printNode();
    std::vector<Rectangle> result;
    if(this -> isLeaf()) {
        for(Rectangle dataRect : this -> data) {
            if(queryRect.cover(dataRect)) {
                result.push_back(dataRect);
            }
        }
    } else {
        for(int i = 0; i < this -> data.size(); i++) {
            if(this -> childId.at(i) >= 0) {
                if(queryRect.isIntersection(data.at(i))) {
                    Node* next = this -> tree -> nodeMap.at(this -> childId.at(i));
                    std::vector<Rectangle> nextResult = next -> queryRect(queryRect);
                    result.insert(result.end(), nextResult.begin(), nextResult.end());
                }
            }
        }
    }
    return result;
}

// return the child of a specific index
Node* Node :: getChild(int index) {
    return tree -> nodeMap.at(childId[index]);
}

// given a rect, find the leaf it belongs to
Node* Node :: findLeaf(Rectangle rect) {
    for(int i = 0; i < rectNums; i++) {
        if(data[i].cover(rect)) {
            if(this -> level == 0) {
                if(data[i] == rect) return this;
            } else {
                Node* leaf = getChild(i) -> findLeaf(rect);
                if(leaf != nullptr) return leaf;
            }
        }
    }
    return nullptr;
}

bool Node :: isRoot() {
    if(parent == -1) return true;
    else return false;
}

bool Node :: isLeaf() {
    if(level == 0) return true;
    else return false;
}

void Node :: printNode() {
    std::string baseStr = "Node msg : \n";
    baseStr = baseStr + "Level: " + std::to_string(this -> level) +"\n";
    baseStr = baseStr + "Page id: " + std::to_string(this -> pageId) +"\n";
    baseStr = baseStr + "Number of Entries: " + std::to_string(this -> rectNums) +"\n";
    baseStr = baseStr + "Parent Page id: " + std::to_string(this -> parent) +"\n"; 
    baseStr = baseStr + "Child Node's Rectangle: " + "\n";
    for(int i = 0; i < MAX_NODE_SPACE; i++) {
        baseStr = baseStr + "  child-" + std::to_string(i) + " " + this -> data.at(i).printRect();
    }
    baseStr += "\n\n";
    std::cout << baseStr << std::endl;
}

Rtree :: Rtree() : MAX_NODE_SPACE(10), PAGE_COUNTER(1000) {}


Rtree :: Rtree(const Rtree& other) : MAX_NODE_SPACE(other.MAX_NODE_SPACE), 
                                     PAGE_COUNTER(other.PAGE_COUNTER) {
    nodeMap = other.nodeMap;
}

Rtree :: Rtree(Rtree&& other) noexcept : MAX_NODE_SPACE(other.MAX_NODE_SPACE), 
                                         PAGE_COUNTER(other.PAGE_COUNTER),
                                         nodeMap(other.nodeMap) {}

Rtree& Rtree :: operator = (const Rtree& other) {
    if (this != &other) {
        for (auto& pair : nodeMap) {
            delete pair.second;
        }
        nodeMap.clear();
        PAGE_COUNTER = other.PAGE_COUNTER;
        for (const auto& pair : other.nodeMap) {
            nodeMap[pair.first] = new Node(*pair.second);
        }
    }
    return *this;
}

Rtree& Rtree :: operator = (const Rtree&& other) {
    if(this != &other) {
        PAGE_COUNTER = other.PAGE_COUNTER;
        nodeMap = std::move(other.nodeMap);
    }
    return *this;
}

// Rtree :: ~Rtree () {
//     for(auto& pair : nodeMap) {
//         delete pair.second;
//     }
// }

// initialize a tree
void Rtree :: initite(int parent, int pageId, int level, int nodeSpace) {
    Node *node = new Node(parent, pageId, level, nodeSpace, this);
    this -> nodeMap.emplace(0, node);
}

// return the root of current tree
Node* Rtree :: getRoot() {
    return nodeMap.at(0);
}


// insert a rectangle into Rtree
void Rtree :: insertNode(Rectangle rect, int page) {
    Node* leaf;
    int currentPage;
    Node* root = nodeMap.at(0);
    if(root -> isLeaf()) {
        leaf = root;
        currentPage = 0;
    } else {
        leaf = chooseLeaf(rect, root);
    }

    if(leaf -> rectNums < MAX_NODE_SPACE) {
        int a = leaf -> data.capacity();
        int b = leaf -> data.size();
        leaf -> data.at(leaf -> rectNums) = rect;
        leaf -> childId.at(leaf -> rectNums) = page;
        leaf -> rectNums += 1;
        nodeMap.insert_or_assign(leaf -> pageId, leaf);
        Node* parent = leaf -> getParent();
        if(parent != nullptr) {
            parent -> adjustTree(leaf, nullptr);
        }
    } else {
        Node** nodes = leafSplit(leaf, rect, page);
        Node* n1 = nodes[0];
        Node* n2 = nodes[1];

        if(leaf -> isRoot()) {
            n1 -> parent = 0;
            n1 -> pageId = -1;
            n2 -> parent = 0;
            n2 -> pageId = -1;
            n1 = nextPageNumber(n1);
            n2 = nextPageNumber(n2);

            Node* node = new Node(-1, 0, 1, MAX_NODE_SPACE, this);
            node -> addData(n1 -> getNodeRectangle(), n1 -> pageId);
            node -> addData(n2 -> getNodeRectangle(), n2 -> pageId);
            // nodeMap.emplace(0, node);
            nodeMap.insert_or_assign(0, node);
        } else {
            n1 -> pageId = leaf -> pageId;
            n2 -> pageId = -1;
            n1 = nextPageNumber(n1);
            n2 = nextPageNumber(n2);
            Node* parentNode = leaf -> getParent();
            parentNode -> adjustTree(n1, n2);
        }
    }
}

/*
given a rectangle, recursively find the leaf node
where the rectangle should be insert
*/
Node* Rtree :: chooseLeaf(Rectangle rect, Node* node) {
    int index = findLeastGrowth(rect, node);
    Node* descent = node -> getChild(index);
    if(node -> level == 1) {
        return descent;
    }
    return chooseLeaf(rect, descent);
}

/*
given a rectangle, choose the index of a rectangle in the node, 
which has the least area growth
*/
int Rtree :: findLeastGrowth(Rectangle rect, Node* node) {
    double area = std::numeric_limits<double>::infinity();
    int sel = -1;

    for(int i = 0; i < node -> rectNums; i++) {
        double grow = node -> data[i].unionRect(rect).getArea() - node -> data[i].getArea();
        if(grow < area) {
            area = grow;
            sel = i;
        } else if(grow == area) {
            sel = (node -> data[sel].getArea() <= node -> data[i].getArea()) ? sel : i;
        }
    }
    return sel;
}

// split leaf when it overflow
Node** Rtree :: leafSplit(Node* leaf, Rectangle rect, int page) {
    std::vector<std::vector<int>> group = QuadraticSplit(leaf, rect, page);
    Node* n1 = new Node(leaf -> parent, -1, 0, MAX_NODE_SPACE, this);
    Node* n2 = new Node(leaf -> parent, -1, 0, MAX_NODE_SPACE, this);
    std::vector<int> group1 = group[0];
    std::vector<int> group2 = group[1];

    for(int i = 0; i < group1.size(); i++) {
        n1 -> addData(leaf -> data[group1[i]], leaf -> childId[group1[i]]);
    }

    for(int i = 0; i < group2.size(); i++) {
        n2 -> addData(leaf -> data[group2[i]], leaf -> childId[group2[i]]);
    }

    Node** result = new Node*[2];
    result[0] = n1;
    result[1] = n2;
    return result;
}

// when a new node initialized, call this function to mark and store it
Node* Rtree :: nextPageNumber(Node* node) {
    if(node -> pageId < 0) {
        node -> pageId = this -> PAGE_COUNTER + 1;
        PAGE_COUNTER += 1;
    }
    Rtree::nodeMap.insert_or_assign(node -> pageId, node);
    return node;
}

// realize the quadratic split ruls in R-Tree's definition
std::vector<std::vector<int>> Rtree :: QuadraticSplit(Node* leaf, Rectangle rect, int page) {
    // leaf -> data[leaf -> rectNums] = rect;
    // leaf -> childId[leaf -> rectNums] = page;
    leaf -> data.push_back(rect);
    leaf -> childId.push_back(page);
    int total = leaf -> rectNums + 1;

    int* mask = new int[total];
    // int mask[total];
    for(int i = 0; i < total; i++) {
        mask[i] = 1;
    }

    int c = total;
    int minNodeSize = Rtree::MAX_NODE_SPACE / 2;
    if(minNodeSize < 2) minNodeSize = 2;

    int uncheck = total;
    int* group1 = new int[c];
    int* group2 = new int[c];
    // int group1[c];
    // int group2[c];
    std::fill(group1, group1 + c, -1);
    std::fill(group2, group2 + c, -1);
    int i1 = 0;
    int i2 = 0;

    int* seed = QuadraticPickSeeds(leaf);
    group1[i1++] = seed[0];
    group2[i2++] = seed[1];
    uncheck -= 2;
    mask[group1[0]] = -1;
    mask[group2[0]] = -1;

    while(uncheck > 0) {
        if(minNodeSize - i1 == uncheck) {
            for(int i = 0; i < total; i++) {
                if(mask[i] != -1) {
                    group1[i1++] = i;
                    mask[i] = -1;
                    uncheck -= 1;
                }
            }
        } else if(minNodeSize - i2 == uncheck) {
            for(int i = 0; i < total; i++) {
                if(mask[i] != -1) {
                    group2[i2++] = i;
                    mask[i] = -1;
                    uncheck -= 1;
                }
            }
        } else {
            Rectangle mbr1 = leaf -> data[group1[0]];
            for(int i = 1; i < i1; i++) {
                mbr1 = mbr1.unionRect(leaf -> data[group1[i]]);
            }

            Rectangle mbr2 = leaf -> data[group2[0]];
            for(int i = 1; i < i2; i++) {
                mbr2 = mbr2.unionRect(leaf -> data[group2[i]]);
            }

            double diff = std::numeric_limits<double>::lowest();
            double diff1Area = 0, diff1AreaSel = 0, diff2Area = 0, diff2AreaSel = 0;
            int sel = -1;

            for(int i = 0; i < total; i++) {
                if(mask[i] != -1) {
                    sel = i;
                    Rectangle a = mbr1.unionRect(leaf -> data[i]);
                    diff1Area = a.getArea() - mbr1.getArea();
                    Rectangle b = mbr2.unionRect(leaf -> data[i]);
                    diff2Area = b.getArea() - mbr2.getArea();

                    if(std::abs(diff1Area - diff2Area) > diff) {
                        diff = std::abs(diff1Area - diff2Area);
                        sel = i;
                        diff1AreaSel = diff1Area;
                        diff2AreaSel = diff2Area;
                    }
                }
            }

            if(diff1AreaSel < diff2AreaSel) {
                group1[i1++] = sel;
            } else if(diff1AreaSel > diff2AreaSel) {
                group2[i2++] = sel;
            } else if(mbr1.getArea() < mbr2.getArea()) {
                group1[i1++] = sel;
            } else if(mbr1.getArea() > mbr2.getArea()) {
                group2[i2++] = sel;
            } else if(i1 < i2) {
                group1[i1++] = sel;
            } else if(i1 > i2) {
                group2[i2++] = sel;
            } else {
                group1[i1++] = sel;
            }
            mask[sel] = -1;
            uncheck -= 1;
        }
    }
    
    std::vector<int> result1;
    result1.assign(i1, -1);
    std::vector<int> result2;
    result2.assign(i2, -1);
    // int** result = new int*[2];
    // result[0] = new int[i1];
    // result[1] = new int[i2];

    // int size0 = sizeof(result);
    int size1 = result1.size();
    int size2 = result2.size();

    for(int i = 0; i < i1; i++) {
        // result0[i] = group1[i];
        result1.at(i) = group1[i];
    }
    for(int i = 0; i < i2; i++) {
        // result1[i] = group2[i];
        result2.at(i) = group2[i];
    }
    std::vector<std::vector<int>> result = {result1, result2};

    // delete[] mask;
    // delete[] group1;
    // delete[] group2;
    // delete[] seed;
    // int** result = new int*[2];
    // result[1] = result0;
    // result[2] = result1;
    return result;
}

// seek seeds in quadratic split
int* Rtree :: QuadraticPickSeeds(Node* node) {
    double inefficiency = std::numeric_limits<double>::lowest();
    int* result = new int[2];
    result[0] = 0;
    result[1] = 0;

    for(int i = 0; i < node -> rectNums; i++) {
        for(int j = 0; j < node -> rectNums; j++) {
            Rectangle cover = node -> data[i].unionRect(node -> data[j]);
            double diff = cover.getArea() - node -> data[i].getArea() - node -> data[j].getArea();
            if(diff > inefficiency) {
                inefficiency = diff;
                result[0] = i;
                result[1] = j;
            }
        }
    }
    return result;
}

// find the final MBR of the tree
Rectangle Rtree :: getFinalRect() {
    return nodeMap.at(0) -> getNodeRectangle();
}


#endif