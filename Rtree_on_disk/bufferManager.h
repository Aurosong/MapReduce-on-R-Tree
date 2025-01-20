#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "config.h"
#include "diskManager.h"
#include "errors.h"
#include <unordered_map>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <list>

using namespace std;

struct PageDescriptor {
    int fd;
    int pagenum;
    PageDescriptor() {fd = pagenum = -1;}
    PageDescriptor(int f, int pno) : fd(f), pagenum(pno) {}
    inline bool operator == (const PageDescriptor& pd1) const {
        return (pd1.fd == fd && pd1.pagenum == pagenum);
    }
    inline PageDescriptor& operator = (const PageDescriptor& pd1) {
        fd = pd1.fd;
        pagenum = pd1.pagenum;
        return *this;
    }
};

namespace std {
    template <>
    struct hash<PageDescriptor> {
        size_t operator()(const PageDescriptor& pd) const {
            return hash<int>()(pd.fd) ^ hash<int>()(pd.pagenum);
        }
    };
}

struct Frame {
    PageDescriptor pageDescriptor;
    bool dirty;
    bool pinned;
    char* data;
};

class BufferManager {
public:
    BufferManager(int num_pages);
    ~BufferManager();
    char* getPage(PageDescriptor pd);
    char* allocatePage(PageDescriptor pd);
    bool markDirty(PageDescriptor pd);
    bool unpinPage(PageDescriptor pd);
    bool flushPage(PageDescriptor pd);
    bool flushPages(int fd);
    void clearBuffer();
    void printBuffer();

private:
    list<int> freeList;
    list<int> LRUList;
    Frame* buffers;
    int numPages;
    int pageSize;
    unordered_map<PageDescriptor, int> hashTable; // pageDescriptor and the slot it stored in buffer
    int findSlot();
    bool readPage(PageDescriptor pd, char* dest);
    bool writePage(PageDescriptor pd, char* data);
    void initializeBuffer(PageDescriptor pd, int slow_no);
};

BufferManager::BufferManager(int num_buffers) {
	this -> numPages = num_buffers;
	this -> pageSize = PAGE_SIZE;
	this -> buffers = new Frame[num_buffers];
	for(int i = 0; i < num_buffers; i++) {
		buffers[i].data = new char[this -> pageSize];
		freeList.push_back(i);
	}
}

BufferManager::~BufferManager() {
	for(int i = 0; i < this -> numPages; i++) 
	    delete buffers[i].data;
	delete this -> buffers;
	freeList.clear();
	LRUList.clear();
	hashTable.clear();
}


char* BufferManager::getPage(PageDescriptor pd) {
	auto slot = hashTable.find(pd);
	if(slot != hashTable.end()) {
		//already in buffer 
		int slotNo = slot -> second;
		// no need to read the page
		// to establish replacement policy, make page MRU 
		LRUList.remove(slotNo); 
		LRUList.push_front(slotNo); // put at front
		return buffers[slotNo].data;
	}
	else {
		// page not in buffers
		// find a suitable slot to load it
		int slotNo = findSlot();
		if(slotNo == -1) throw NoBufferSpaceException (); //error no free slot could be obtained 
		// read data to buffers[slotNo].data
		bool _read_res = readPage(pd, buffers[slotNo].data);
		if(!_read_res) throw BufferManagerException("BufferManagerException : Read request failed");
		// insert into hashTable the corresponding slot
		hashTable.insert(make_pair(pd, slotNo));
		// initialize the rest of Frame elements
		initializeBuffer(pd, slotNo);
		return buffers[slotNo].data;
	}
}

//allocate a new page in buffers
//since new page, no contents in file
//rest same as GetPage
char* BufferManager::allocatePage(PageDescriptor pd) {
	auto slot = hashTable.find(pd);
	if(slot!=hashTable.end()) {
		// error page already in buffers
		throw BufferManagerException("BufferManagerException : Page to be allocated already in buffer");
	}
	else {
		// page not in buffers
		// find a suitable slot to load it
		int slotNo = findSlot();
		if (slotNo==-1) throw NoBufferSpaceException(); //error no free slot could be obtained 
		// insert into hashTable the corresponding slot
		hashTable.insert(make_pair(pd,slotNo));
		// initialize the rest of Frame elements
		initializeBuffer(pd,slotNo);
		return buffers[slotNo].data;
	}
}

// mark the page in buffer as dirty
bool BufferManager::markDirty(PageDescriptor pd) {
	auto slot = hashTable.find(pd);
	if(slot==hashTable.end()) {
		//error page not in buffers
		return false;
	}
	int slotNo = slot->second;
	if(!buffers[slotNo].pinned) {
		//error page unpinned
		return false;
	}
	// set page dirty
	buffers[slotNo].dirty=true;
	//make this page MRU in buffers
	LRUList.remove(slotNo); 
	LRUList.push_front(slotNo); // put at front	
	return true;
}

// unpin page -- required so that buffer manager can free up space from such marked buffers

bool BufferManager::unpinPage(PageDescriptor pd) {
	auto slot = hashTable.find(pd); //find slot 
	if(slot==hashTable.end()) {
		//error page not in buffers
		return false;
	}
	int slotNo = slot->second;
	if(!buffers[slotNo].pinned) {
		//error page unpinned
		return false;
	}
	buffers[slotNo].pinned = false; //set unpinned
	// set page as MRU
	LRUList.remove(slotNo); 
	LRUList.push_front(slotNo); // put at front
	return true;	
}

// release all pages for file and put them onto free list
bool BufferManager::flushPages(int fd) {
	// do a linear scan for all pages belonging to this file
	for(int i=0;i<numPages;i++) {
		if(find(LRUList.begin(),LRUList.end(),i)==LRUList.end()) continue; // skip free slots
		if(buffers[i].pageDescriptor.fd != fd) continue;
		//write back page if dirty
		if(buffers[i].dirty) {
			bool _res = writePage(PageDescriptor(fd,buffers[i].pageDescriptor.pagenum),buffers[i].data);
			if(!_res) return false;
			buffers[i].dirty = false;
		}
		//remove slot from hashTable and LRUList and add to free list
		hashTable.erase(PageDescriptor(fd,buffers[i].pageDescriptor.pagenum));
		LRUList.remove(i);
		freeList.push_front(i);
	}
	return true;
}

// release given page of file from buffers
bool BufferManager::flushPage(PageDescriptor pd) {
	// do a linear scan to find the corresponding page 
	for(int i=0;i<numPages;i++) {
		if(find(LRUList.begin(),LRUList.end(),i)==LRUList.end()) continue; // skip free slots
		if(!(buffers[i].pageDescriptor== pd)) continue;
		//write back page if dirty
		if(buffers[i].dirty) {
			bool _res = writePage(pd,buffers[i].data);
			if(!_res) return false;
			buffers[i].dirty = false;
		}
		//remove slot from hashTable and LRUList and add to free list
		hashTable.erase(pd);
		LRUList.remove(i);
		freeList.push_front(i);
		break; // only once instance 
	}
	return true;
}

void BufferManager::printBuffer() {
	cout << "Buffer contains " << numPages << " pages of size "
      << pageSize <<".\n";
   cout << "Contents in order from most recently used to "
      << "least recently used.\n";

    for(auto slot:LRUList) {
    	cout << slot << " :: \n";
      	cout << "  fd = " << buffers[slot].pageDescriptor.fd << "\n";
      	cout << "  pageNum = " << buffers[slot].pageDescriptor.pagenum << "\n";
      	cout << "  Dirty = " << buffers[slot].dirty << "\n";
      	cout << "  pinned = " << buffers[slot].pinned << "\n";
      
    }
}

void BufferManager::clearBuffer() {
	while(!LRUList.empty()) {
		int slot = LRUList.front();
		LRUList.pop_front(); // remove front LRUList
		hashTable.erase(buffers[slot].pageDescriptor); //remove from hash table
		freeList.push_front(slot); // add to free list
	}
}

//FindSlot - find a free slot , if no free, then find a victim slot from unpinned pages 
// return -1 if replacement not possible
int BufferManager::findSlot() {
	// if free slot available
	if(!freeList.empty()) {
		int slot = freeList.front();
		freeList.pop_front();
		LRUList.push_front(slot);
		return slot;
	}
	else {
		// since used list has MRU in front 
		// start search for a unpinned page from last 
		for(auto slot = LRUList.rbegin();slot!=LRUList.rend();slot++) {
			if(!buffers[*slot].pinned) {
				// page will be replaced
				// if dirty write to the file
				if(buffers[*slot].dirty) {
					int _res = writePage(buffers[*slot].pageDescriptor,buffers[*slot].data);
					if(_res<0) continue;
					buffers[*slot].dirty=false;
				}
				//remove from hash table , and LRUList
				hashTable.erase(buffers[*slot].pageDescriptor);
				// move the slot to MRU (front of used list)
				int slot_val = *slot;
				LRUList.remove(slot_val);
				LRUList.push_front(slot_val);
				return slot_val;
			}
		}
	}
	return -1; // no slot available
}

// read page from the file and return character array to it
bool BufferManager::readPage(PageDescriptor pd, char *dest) {
	// calculate file offset
	long offset = pd.pagenum * (long)pageSize + FILE_HDR_SIZE;
	lseek(pd.fd, offset, 0); // seek to location
	int count = -1;
	count = read(pd.fd, dest, pageSize);
	if(count != pageSize) //read failed
		return false;
	return true;
}

// write page contents to file 
bool BufferManager::writePage(PageDescriptor pd,char *src) {
	// calculate file offset
	long offset = pd.pagenum*(long)pageSize + FILE_HDR_SIZE;
	lseek(pd.fd,offset,0); // seek to location
	if(write(pd.fd,src,pageSize)!=pageSize) //write failed
		return false;
	return true;
}	

// helper function to initialize buffer page after page read 
void BufferManager::initializeBuffer(PageDescriptor pd, int slot) {
	buffers[slot].pageDescriptor = pd;
	buffers[slot].dirty = false;
	buffers[slot].pinned = true;
}

#endif