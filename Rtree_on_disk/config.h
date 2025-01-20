#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cassert>

const int BUFFER_SIZE = 40;
const int PAGE_SIZE = 4096;
const int PAGE_CONTENT_SIZE = PAGE_SIZE - sizeof(int);
const int END_FREE = -1;
const int NOT_FREE = -2;
const int FILE_HDR_SIZE = PAGE_SIZE;

#endif