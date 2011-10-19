/*
 *  FileHandler.h
 *  MedialAxisTransform
 *
 *
 *  Created by Beau Johnston on 25/08/11.
 *  Copyright (C) 2011 by Beau Johnston.
 *
 *  Please email me if you have any comments, suggestions or advice:
 *                              beau@inbeta.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */


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