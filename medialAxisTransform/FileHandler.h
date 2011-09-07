//
//  FileHandler.h
//  openCLImageLoad
//
//  Created by Beau Johnston on 25/08/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#ifndef openCLImageLoad_FileHandler_h
#define openCLImageLoad_FileHandler_h

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

class FileHandler
{
private:
    
    /* private variables */
    vector<string> files;
    string * orderedFiles;
    int traverser;
    string path;
    
    template <class T> bool from_string(T& t, 
                                        const std::string& s, 
                                        std::ios_base& (*f)(std::ios_base&))
    {
        std::istringstream iss(s);
        return !(iss >> f >> t).fail();
    }
    
    /* private functions */
    int getdir (string dir, string ext, vector<string> &files);
    int getFilesInDirectoryWithName (string dir, string name, vector<string> &files);
    string remove(string str, string substr);
    bool doesNumberingStartAt0(string name, string ext);
    bool doesNumberingStartAt1(string name, string ext);
    bool noFilesAreMissing(string name, string ext);

    
public: 
    FileHandler();
    ~FileHandler();
    
    void generateListOfAssociatedFiles(char* filename);
    char* getNextFileName(void);
    bool areFilesLeft(void);
    int numberOfFiles(void);
    void printFiles(void);
    void sortFilesNumerically(std::string name, std::string ext);
};

#endif
