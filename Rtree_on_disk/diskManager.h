#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <iostream>
#include "config.h"
#include "bufferManager.h"
#include "errors.h"

class BufferManager;
class StatsManager;
extern int FileManagerInstanceCount;

struct PageHdr {
    int nextFreePage;
};

class PageHandler {
friend class FileHandler;
public:
    PageHandler();
    PageHandler(int, char*);
    PageHandler(const PageHandler& pageHandler);
    bool operator == (const PageHandler& pageHandler);
    char* getData();
    int getPageNum();

private:
    int pageNum;
    char* data;
};

PageHandler::PageHandler() {
	pageNum = -1;
	data = NULL;
}

PageHandler::PageHandler(int pageNum, char *data) {
	this -> pageNum = pageNum;
	this -> data = data;
}

PageHandler::PageHandler(const PageHandler& pageHandle) {
	this -> pageNum = pageHandle.pageNum;
	this -> data = pageHandle.data;
}

bool PageHandler::operator == (const PageHandler& pageHandle) {
	return this -> pageNum == pageHandle.pageNum && this -> data == pageHandle.data;
}

char* PageHandler::getData() {
	return data;
}

int PageHandler::getPageNum() {
	return pageNum;
}

struct FileHdr {
    int firstFreePage;
    int totalPages;
};

class FileHandler {
friend class FileManager;
public:
    FileHandler();
    FileHandler(const FileHandler& fileHandler);
    bool operator == (const FileHandler& fileHandler);
    PageHandler firstPage();
    PageHandler nextPage(int page_number);
    PageHandler pageAt(int page_number);
    PageHandler lastPage();
    PageHandler prevPage(int page_number);
    PageHandler newPage();
    bool disposePage(int pageNum);
    bool markDirty(int pageNum);
    bool unpinPage(int pageNum);
    bool flushPage(int page_number);
    bool flushPages();

private:
    bool checkPageValid(int page_number);
    BufferManager* bufferManager;
    FileHdr hdr;
    bool isOpen;
    bool hdrChanged;
    int unix_file_desc;
    char* fileName;
};

FileHandler::FileHandler() {
	isOpen = false;
	hdrChanged = false;
	unix_file_desc = -1;
	bufferManager = NULL;
}

FileHandler::FileHandler(const FileHandler& fileHandle) {
	this -> isOpen = fileHandle.isOpen;
	this -> hdr = fileHandle.hdr;
	this -> bufferManager = fileHandle.bufferManager;
	this -> hdrChanged = fileHandle.hdrChanged;
	this -> unix_file_desc = fileHandle.unix_file_desc;
	this -> fileName = fileHandle.fileName;
}

bool FileHandler::operator == (const FileHandler& fileHandle) {
	return this -> unix_file_desc == fileHandle.unix_file_desc;
}

PageHandler FileHandler::firstPage() {
    return nextPage(-1);
}

PageHandler FileHandler::nextPage(int page_number) {
	PageHandler pageHandle;
	if(!checkPageValid(page_number) && page_number!=-1) {
		throw InvalidPageException();
	}
	// start searching for next valid(used) page 
	page_number = page_number + 1;
	for( ; page_number <= hdr.totalPages; page_number++) { // MR: page_number <= totalpages to ensure that it runs this at least once if file contains only one page
		pageHandle = pageAt(page_number);
		if(pageHandle.getPageNum() != -1) break;
	}
	return pageHandle;
}

PageHandler FileHandler::pageAt(int page_number) {
	PageHandler pageHandle;
	if(!checkPageValid(page_number)) {
		throw InvalidPageException();
	}
 
	char* page_in_buffer = bufferManager -> getPage(PageDescriptor(this -> unix_file_desc, page_number));	
	PageHdr* page_hdr = (PageHdr*)page_in_buffer;
	// if slot is not free
	// set page number 
	// and set page data to point to starting position of data part in page obtained from buffer manager
	if (page_hdr -> nextFreePage == NOT_FREE) {
		pageHandle.pageNum = page_number;
		pageHandle.data = page_in_buffer + sizeof(PageHdr);
	}
	else {
		// unpin page according to redbase logic
		unpinPage(page_number);
	}
	return pageHandle;
}

PageHandler FileHandler::lastPage() {
	// logic - use PrevPage with page number = total_pages == same as fetch first valid page from last (total_pages-1)
	int total_pages = (this->hdr).totalPages;
	return prevPage(total_pages);
}

// get prev page from given page number
PageHandler FileHandler::prevPage(int page_number) {
	if(!checkPageValid(page_number) && page_number != hdr.totalPages) { //allow totalPages (call from LastPage)
		// return invalid page
		throw InvalidPageException();
	}
	// start searching for prev valid(used) page 
	page_number = page_number - 1;
	PageHandler pageHandle;
	for( ; page_number >= 0; page_number--) {
		pageHandle = pageAt(page_number);
		// if page retrieved without error, stop checking further
		if(pageHandle.getPageNum() != -1) break;
	}
	return pageHandle;
}

// allocate a new page in file ( either add at end (if free list empty) or use one from the free list)
// incase of errors - returns INVALID_PAGE
PageHandler FileHandler::newPage() {
	int page_number ; // new page number
	char *page_buffer ; //to store page read from buffer manager
	PageHandler pageHandle;
	//if free list not empty 
	if(hdr.firstFreePage != END_FREE) {
		// first free page number will be the new page number 
		page_number = hdr.firstFreePage;
		//contents of page are read using the buffer manager
		page_buffer = bufferManager -> getPage(PageDescriptor(this -> unix_file_desc, page_number));
		// update the head of free page list for file
		hdr.firstFreePage = ((PageHdr*)page_buffer) -> nextFreePage;
	}
	else { // free pages
		page_number = hdr.totalPages; // new page will be added at the end of file
		page_buffer = bufferManager -> allocatePage(PageDescriptor(this -> unix_file_desc,page_number)); //allocate and load the page in buffer manager
		//increase number of pages by one
		hdr.totalPages++;
	}
	//mark file header is changed
	this -> hdrChanged = true;
	// mark the new page as used
    ((PageHdr*)page_buffer) -> nextFreePage = NOT_FREE;
    //overwrite page contents
    memset(page_buffer + sizeof(PageHdr), 0, PAGE_CONTENT_SIZE);
    //mark the page dirty due to next pointer of free list changed
    markDirty(page_number);
    pageHandle.pageNum = page_number;
    pageHandle.data = page_buffer + sizeof(PageHdr);
 
    return pageHandle;
}

bool FileHandler::disposePage(int page_number) {
	if(!checkPageValid(page_number)) return false; // invalid request
	// read the page from the buffer manager 
	char *page_buffer;
	page_buffer = bufferManager -> getPage(PageDescriptor(this -> unix_file_desc,page_number));
	PageHdr * page_header = (PageHdr*) page_buffer;
	// if already free - 
	if (page_header -> nextFreePage != NOT_FREE) {
		unpinPage(page_number); // since page read into buffer manager
		return false;
	}
	// update the head of free list by adding this page at the head
	page_header->nextFreePage = hdr.firstFreePage;
	hdr.firstFreePage = page_number;
	this->hdrChanged = true;
	// mark the page as dirty and then unpin it 
	markDirty(page_number);
	unpinPage(page_number);
	return true;
}

// mark the page as dirty
// then before page is unpinned from buffer
// it will be written to the file
bool FileHandler::markDirty(int page_number) {
	auto pp = PageDescriptor(this -> unix_file_desc, page_number);
	return bufferManager -> markDirty(pp);
}

// unpin page wrapper for buffer manager unpin page function
bool FileHandler::unpinPage(int page_number) {
	auto pp = PageDescriptor(this -> unix_file_desc, page_number);
	return bufferManager -> unpinPage(pp);
}

// flush all pages to file
// note if header is changed, we need to write it back here
// since buffer manager would only deal with pages
bool FileHandler::flushPages() {
	if(this -> hdrChanged) {
		lseek(this -> unix_file_desc, 0, 0); // seek file pointer to start
		int _wr_res = write(this -> unix_file_desc, (char*)&hdr, sizeof(FileHdr));
		if(_wr_res < 0) return false; //write error
		this -> hdrChanged = false;
	}
	return bufferManager -> flushPages(this -> unix_file_desc);
}

// flush individual page out of buffer manager
bool FileHandler::flushPage(int page_number) {
	if(this -> hdrChanged) {
		lseek(this -> unix_file_desc, 0, 0); // seek file pointer to start
		int _wr_res = write(this -> unix_file_desc, (char *)&hdr, sizeof(FileHdr));
		if(_wr_res < 0) return false; //write error
		this -> hdrChanged = false;
	}
	auto pp = PageDescriptor(this -> unix_file_desc,page_number);
	return bufferManager -> flushPage(pp);
}

bool FileHandler::checkPageValid(int page_number) {
	if(isOpen && page_number>=0 && page_number < hdr.totalPages) return true;
	return false;
}

class FileManager {
public:
    FileManager();
    ~FileManager();
    FileHandler createFile(const char* fileName);
    FileHandler openFile(const char* fileName);
    bool destroyFile(const char* fileName);
    bool closeFile(FileHandler& fileHandle);
    void clearBuffer();
    void printBuffer();

private:
    BufferManager* bufferManager;
};

int FileManagerInstanceCount = 0;

FileManager::FileManager() {
	// intialize manager with a buffer manager
	// thus buffer manager will be hidden from student API access 
	if(FileManagerInstanceCount==1) {
		throw FileManagerInstanceException();
	}
	FileManagerInstanceCount = 1;
	bufferManager = new BufferManager(BUFFER_SIZE);
}

FileManager::~FileManager() {
	// destroy buffer manager in destructor
	FileManagerInstanceCount = 0;
	delete bufferManager;
 
}

// create a file and return a file handle for it 
FileHandler FileManager::createFile(const char* filename) {
	int fd = open(filename, O_CREAT | O_EXCL | O_RDWR, 0660);
	if(fd == -1) throw InvalidFileException();
	char hdr_buf[FILE_HDR_SIZE];
	memset(hdr_buf, 0, FILE_HDR_SIZE);
	FileHdr *hdr = (FileHdr*)hdr_buf;
	hdr -> firstFreePage = END_FREE;
	hdr -> totalPages = 0;
	write(fd, hdr_buf, FILE_HDR_SIZE);
	close(fd);
	return openFile(filename);
}

// delete file, return true if success
bool FileManager::destroyFile(const char* filename) {
	int res = unlink(filename); // reduce the open file count
	return (res == 0) ; // success if return code == 0 
}


// open already existing file, return File handle for it
FileHandler FileManager::openFile(const char *filename) {
	FileHandler fileHandle;
	// open file 
	fileHandle.fileName = new char[strlen(filename) + 1];
	strcpy(fileHandle.fileName, filename);
 

	fileHandle.unix_file_desc = open(filename, O_RDWR);
	if(fileHandle.unix_file_desc == -1) throw InvalidFileException();
	//read header
	fileHandle.hdr.firstFreePage = 0;
	fileHandle.hdr.totalPages = 0;
	int _temp_res = read(fileHandle.unix_file_desc, (char*)&fileHandle.hdr, sizeof(FileHdr));
    // update file metadata
    fileHandle.isOpen = true;
    fileHandle.hdrChanged = false;
    fileHandle.bufferManager = bufferManager;
    return fileHandle;
}

//close file using its file Handle
bool FileManager::closeFile(FileHandler &fileHandle) {
	if(!fileHandle.isOpen) return false; // if not already open
	if(!fileHandle.flushPages()) return false; //flush pages to file, if error in flushing all pages , return false
	close(fileHandle.unix_file_desc); // close the file

	//update meta data
	fileHandle.isOpen = false;
	fileHandle.bufferManager = NULL;	
	return true;
}

// interface to clearing buffer 
void FileManager::clearBuffer() {
	bufferManager -> clearBuffer();
}
// interface to printing buffer contents
void FileManager::printBuffer() {
	bufferManager -> printBuffer();
}

#endif