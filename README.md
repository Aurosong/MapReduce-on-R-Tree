# MapReduce-on-R-Tree

Simulate MapReduce R-Tree query

---

### 1 R-Tree

_Introduction_. R-Tree is a data structure used primarily for indexing multi-dimensional datasets, such as grographical coordinates, hyperrectangle and polygons. It is a balanced tree structure where each node can contain multiple entries, and each entries consists of bounding rectangle, which represents the minimum bounding rectangle (MBR) that encloses all child nodes, and child pointer. It support fundational operations in a tree like searching, insertion and deletion.

_Inplement details_. R-Tree in memory is implement in Rtree folder, including file RTree.h and config.h. File config.h sets constants of R-Tree, and construct _Point_ and _Rectangle_ structure as entries in R-Tree. File RTree.h is the realization of R-Tree, including class Node and Rtree.

Every Node has the following attributes: a _level_ denote its level in R-Tree with leaf node as level 0, a _pageId_ as the unique identifier of the node, which can be more important in R-Tree on disk, a counter to count the entries in the node, and two vectors, one is used to store entries (Rectangles or MBR) in the node, and another lists child node's pageId of this node. A call back pointer _*tree_ is also included in the node, which points to the tree it belongs to.

In class _RTree_, a hashmap is maintained to store the nodes of the tree. The key is the pageId of the node and the value is a pointer to the node respectively. Besides, two constants which declare the node's capacity and the next pageId to be allocated.

_Optimizations on index construction_. R-Tree's structure is built during the process of insertion. There are some strategies can be used to optimize the building process.

- Quadratic Split

- Linear Split

- SL Split Algorithm, Rectangle Split, etc.

- Bulk Loading

- R* Tree

Among all of them, quadratic split and linear split are two strategies that most commonly used in building a R-Tree. Quadratic split choose two entries that create as much empty space as possible, it _1_. peek two entries in the node, which maximum the difference of the MBR of the node and the sum area of MBRs of the two entries. These entries are denoted as _seeds_. For the left entries, we insert it into one of seed, which minimize the increased MBR's before and after it is insert. Instead, linear split choose two entries that are farthest apart and split entries according to their orders on a specific dimension.

In Quadratic split, we must compare entreis in the node one by one to find seeds, this time complexity easily reaches $O(n^2)$ and n is the capacity of a node. However in linear split, this is much more simple for only sort and split is needed. Time complexity in linear search is usually $O(nlogn)$. But quadratic split promises a better space usage than linear split, which makes it suitable in unevenness conditions.

although, compared to Linear Split and other split strategies, this algorithm's time complexity increases with $O(n^2)$, n denotes the number of entries to be insert, but the balance of the R-Tree is maintained, when the data's amount is huge, it is crucial to the optimization of query in the tree.

In this project, we mainly use _Quadratic Split_ to optimize the building of R-Tree. Every time a node overflows and need a split, we _1_. peek two entries in the node, which maximum the difference of the MBR of the node and the sum area of MBRs of the two entries. These entries are denoted as _seeds_. For the left entries, we insert it into one of seed, which minimize the increased MBR's before and after it is insert. Following this strategies, a node splits into two __balanced__ nodes. although, compared to Linear Split and other split strategies, this algorithm's time complexity increases with $O(n^2)$, n denotes the number of entries to be insert, but the balance of the R-Tree is maintained, when the data's amount is huge, it is crucial to the optimization of query in the tree.

---

### 2 MapReduce and Query

_Introduction_. MapReduce is a programming model and an associated implementation for processing large datasets that can be parallelized across a distributed cluster of computers. A map function takes an input dataset and processses it into a set of intermediate key-value pairs, then they are shuffled and sorted according to their keys, grouping all values associated with the same key together. The reduce function then processes them to produce the final result. Basically, MapReduce is deploied on distributed system, but we simulate its working process in folder _MapReduce_. File _job.h_ packaing the dataset and query into a job, _master.h_ and _worker.h_ simulate the master-slave architecture of MapReduce, simulate the process of _map - sort and shuffle - reduce_.

_Preprocessing_. Assuming that a query for a R-Tree is a range represented by a rectangle, we do the following operations as the preprocessing to optimize the MapReduce algorithm.

- Resize the query's rectangle by intersect it with the MBR of R-Tree's root node. Because the root's MBR represents the total range of this tree, queries beyond this range can not be found in this tree so they can be omitted.

- Split the query's rectangle into sub-jobs. These sub-jobs are distributed to workers in mapping. The strategies of split are various: split by longer axis, random split or, considering there are more than one R-Tree in the data, with the reference of MBR of each tree.

_Query by MapReduce_. The logic of execuating a query, represented by a rectangle,  in a single tree is recursive. Function _queryRect_ in _Rtree.h_ realize this by recursively call itself in the suitable child nodes until leaf node. In MapReduce process, every worker executes its sub-query, get the result and form key - value pattern. Then some of them work as reducer, collect the mapping result in other workers and output the final result.

---

### 3 Efficiency Analysis

Recall that we use quadratic split strategy in orgnizing R-Tree. This algorithm demand search two _seeds_ by compare entries in a node one by one so the time complexity reaches $O(n^2)$. Accordingly, this strategy promise a better space efficiency. On the other hand, linear split is simpler and faster in split, but might not perform well in some certain circumstances. In our experience, we compare these two strategies by construct R-Tree with the same data and execute the same query by MapReduce simulator. Ensuring the accuracy of the experiment by controlling the variables. The result are shown as follows.

| Data Amount | Quadratic Split Index Construction Time | Quadratic Split Query Time | Linear Split Index Construction Time | Linear Split Query Time |
|:-----------:| --------------------------------------- | -------------------------- | ------------------------------------ | ----------------------- |
| 1000        | 0.006812                                | 0.006426                   | 0.011157                             | 0.011157                |
| 5000        | 0.051807                                | 0.069084                   | 0.073307                             | 0.073309                |
| 10000       | 0.138739                                | 0.26391                    | 0.159564                             | 0.159569                |
| 50000       | 1.790808                                | 4.272801                   | 0.941505                             | 0.941532                |
| 100000      | 4.867134                                | 9.385408                   | 2.004629                             | 2.004687                |
| 500000      | 62.439416                               | 75.232261                  | 11.428443                            | 11.428768               |

For quadratic split, the quadratic regression functions fitting results of its build time and query time are $T(Construction) = 1.89594 \times 10^{-10} x^2 + 3.13133\times10^{-5}x-0.1143$ and $T(Query)=1.98592\times10^{-10}x^2+3.03152\times10^{-5}x-0.1139$. For linear split, the regression functions are $T(Construction)=1.74196\times10^{-6}x\ln(x)-8.76303\times10^{-4}$ and $T(Construction)=1.74201\times10^{-6}x\ln(x)-8.76308\times10^{-4}$.
