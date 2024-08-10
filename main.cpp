#include <bits/stdc++.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
#include <ctime>
#include <unordered_set>
#include <set>
#include <map>

using namespace std;

struct Process {
    int pid;
    string name;
    int size;                   // in pages
    vector<int> pageTable;
    map<int, int> addrToVals;
    bool inMain;

    Process(int _pid, string _name, int _size, bool _inMain) : pid(_pid), name(_name), size(_size), inMain(_inMain) {
        pageTable.resize(size, -1);
    }

    Process(int _pid, string _name, int _size, bool _inMain, map<int, int> _addrToVals) : pid(_pid), name(_name), size(_size), inMain(_inMain), addrToVals(_addrToVals) {
        pageTable.resize(size, -1);
    }
};

bool compareByPid(const Process& a, const Process& b) {
    return a.pid < b.pid;
}

int main(int argc, char* argv[]) {
    if(argc == 11) {
        time_t now = time(0);
        tm* localTime = localtime(&now);

        char dateTimeStr[100];
        strftime(dateTimeStr, sizeof(dateTimeStr), "%Y-%m-%d %H:%M:%S", localTime);
        int mainMemSize = stoi(argv[2]);
        int virtMemSize = stoi(argv[4]);
        int pageSize = stoi(argv[6]);
        int nextPID = 1;
        set<int> mainMemFreePages;
        set<int> virtMemFreePages;

        string infile = argv[8];
        string outfile = argv[10];

        infile += ".txt";
        outfile += ".txt";

        ifstream inputFile(infile);
        if(!inputFile.is_open()) {
            cout << "Error: could not open input file." << endl;
        }

        ofstream outputFile(outfile);
        if(!outputFile.is_open()) {
            cout << "Error: could not open output file." << endl;
        }

        int mainMemSizeInPages = (mainMemSize * 1024) / pageSize;
        int virtMemSizeInPages = (virtMemSize * 1024) / pageSize;

        vector<Process> procsInMainMem;
        vector<Process> procsInVirtMem;

        vector<Process> LRUMainMem;

        int timer = 0;
        
        // cout << "main mem and virt mem in pages: " << mainMemSizeInPages << " " << virtMemSizeInPages << endl;

        for(int ii = 0; ii < mainMemSizeInPages; ii++) {
            mainMemFreePages.insert(ii);
        }

        for(int ii = 0; ii < virtMemSizeInPages; ii++) {
            virtMemFreePages.insert(ii);
        }

        vector<int> mainMem(mainMemSizeInPages, 0);
        vector<int> virtMem(virtMemSizeInPages, 0);

        string currLine;
        vector<string> commands;

        while(getline(inputFile, currLine)) {
            commands.push_back(currLine);
        }

        for(const string& command: commands) {
            string currComm = "";
            for(auto ii: command) {
                if(ii == ' ') {
                    break;
                } else {
                    currComm += ii;
                }
            }

            // cout << "Processing the command: " << currComm << endl;

            if(currComm == "load") {
                stringstream ss(command);
                string cmd;
                vector<string> fileNames;

                ss >> cmd;

                string currFileName;
                while(ss >> currFileName) {
                    fileNames.push_back(currFileName);
                }

                for(string currFileName: fileNames) {
                    currFileName = currFileName + ".txt";
                    ifstream file(currFileName);
                    if(!file.is_open()) {
                        outputFile << currFileName << " could not be loaded - file does not exist" << endl;
                        continue;
                    }

                    int size;
                    file >> size;

                    size = (size * 1024) / pageSize;
                    if(size <= mainMemFreePages.size()) {
                        Process thisProc(nextPID, currFileName, size, true);
                        for(int ii = 0; ii < size; ii++) {
                            thisProc.pageTable[ii] = *mainMemFreePages.begin();
                            mainMemFreePages.erase(thisProc.pageTable[ii]);
                            mainMem[thisProc.pageTable[ii]] = nextPID;
                        }
                        procsInMainMem.push_back(thisProc);
                        outputFile << currFileName << " is loaded in main memory and is assigned process id " << nextPID << endl;
                        nextPID++;
                    } else if(size <= virtMemFreePages.size()) {
                        Process thisProc(nextPID, currFileName, size, false);
                        for(int ii = 0; ii < size; ii++) {
                            thisProc.pageTable[ii] = *virtMemFreePages.begin();
                            virtMemFreePages.erase(thisProc.pageTable[ii]);
                            virtMem[thisProc.pageTable[ii]] = nextPID;
                        }
                        procsInVirtMem.push_back(thisProc);
                        outputFile << currFileName << " is loaded in virtual memory and is assigned process id " << nextPID << endl;
                        nextPID++;
                    } else {
                        outputFile << currFileName << " could not be loaded - memory is full" << endl;
                    }

                    // cout << "---Main memory contents---" << endl;
                    // for(auto ii: procsInMainMem) {
                    //     cout << ii.pid << endl;
                    // }

                    // cout << "---Virt memory contents---" << endl;
                    // for(auto ii: procsInVirtMem) {
                    //     cout << ii.pid << endl;
                    // }
                }
            } else if(currComm == "swapout") {
                stringstream ss(command);
                string cmd;
                string pid;
                ss >> cmd;
                ss >> pid;
                int thisPid = stoi(pid);
                bool done = false;
                for(auto ii = procsInMainMem.begin(); ii != procsInMainMem.end(); ii++) {
                    if(ii->pid == thisPid) {
                        // cout << "matched " << thisPid << endl;
                        if(ii->size <= virtMemFreePages.size()) {
                            Process thisProc(ii->pid, ii->name, ii->size, false);
                            for(int jj = 0; jj < ii->pageTable.size(); jj++) {
                                mainMemFreePages.insert(ii->pageTable[jj]);
                            }
                            // cout << "hi" << endl;
                            for(int jj = 0; jj < mainMem.size(); jj++) {
                                if(mainMem[jj] == ii->pid) {
                                    mainMem[jj] = 0;
                                }
                            }
                            
                            for(int jj = 0; jj < ii->size; jj++) {
                                thisProc.pageTable[jj] = *virtMemFreePages.begin();
                                virtMemFreePages.erase(thisProc.pageTable[jj]);
                                virtMem[thisProc.pageTable[jj]] = ii->pid;
                            }
                            // cout << "hiiiii" << endl;
                            procsInVirtMem.push_back(thisProc);
                            procsInMainMem.erase(ii);
                            done = true;
                            break;
                        } else {
                            outputFile << "process id " << thisPid << " does not have enough space to swap out to virtual memory" << endl;
                        }
                    }
                }
                if(done) {
                    outputFile << "Swapped out the process " << thisPid << " into virtual memory" << endl;
                }

            } else if(currComm == "swapin") {
                stringstream ss(command);
                string cmd;
                string pid;
                ss >> cmd;
                ss >> pid;
                int thisPid = stoi(pid);
                bool found = false;
                for(auto ii = procsInVirtMem.begin(); ii != procsInVirtMem.end(); ii++) {
                    if(ii->pid == thisPid) {
                        if(ii->size > mainMemSizeInPages) {
                            outputFile << "Process id " << thisPid << " cannot be swapped into main memory" << endl;
                            break;
                        }
                        found = true;
                        // cout << "found the process " << ii->pid << " in virt meme with size " << ii->size << endl;
                        Process toswapin = Process(ii->pid, ii->name, ii->size, true, ii->addrToVals);
                        while(ii->size > mainMemFreePages.size()) {
                            // cout << "here " << endl;
                            if(!LRUMainMem.empty()) {
                                outputFile << "files in lru" << endl;
                                for(auto ii: LRUMainMem) {
                                    outputFile << ii.pid << " size " << ii.size << endl;
                                }
                                // cout << "hello" << endl;
                                Process toswapout = LRUMainMem[0];
                                if(virtMemFreePages.size() >= toswapout.size) {
                                    auto toerase = procsInMainMem.begin();
                                    for(toerase = procsInMainMem.begin(); toerase != procsInMainMem.end(); toerase++) {
                                        if(toswapout.pid == toerase->pid) {
                                            break;
                                        }
                                    }
                                    Process thisProc(toswapout.pid, toswapout.name, toswapout.size, false, toswapout.addrToVals);
                                    for(int jj = 0; jj < toerase->pageTable.size(); jj++) {
                                        mainMemFreePages.insert(toerase->pageTable[jj]);
                                    }

                                    for(int jj = 0; jj < mainMem.size(); jj++) {
                                        if(mainMem[jj] == thisProc.pid) {
                                            mainMem[jj] = 0;
                                        }
                                    }

                                    for(int jj = 0; jj < thisProc.size; jj++) {
                                        thisProc.pageTable[jj] = *virtMemFreePages.begin();
                                        virtMemFreePages.erase(thisProc.pageTable[jj]);
                                        virtMem[thisProc.pageTable[jj]] = thisProc.pid;
                                    }

                                    procsInVirtMem.push_back(thisProc);
                                    LRUMainMem.erase(LRUMainMem.begin());
                                    procsInMainMem.erase(toerase);
                                } else {
                                    outputFile << "Error: cannot move the least recently used process to virtual memory" << endl;
                                    break;
                                }
                            } else {
                                // cout << "no bye" << endl;
                                Process toswapout = procsInMainMem[0];
                                if(virtMemFreePages.size() >= toswapout.size) {
                                    auto toerase = procsInMainMem.begin();
                                    for(toerase = procsInMainMem.begin(); toerase != procsInMainMem.end(); toerase++) {
                                        if(toswapout.pid == toerase->pid) {
                                            break;
                                        }
                                    }

                                    Process thisProc(toswapout.pid, toswapout.name, toswapout.size, false);
                                    for(int jj = 0; jj < thisProc.pageTable.size(); jj++) {
                                        mainMemFreePages.insert(thisProc.pageTable[jj]);
                                    }

                                    for(int jj = 0; jj < mainMem.size(); jj++) {
                                        if(mainMem[jj] == thisProc.pid) {
                                            mainMem[jj] = 0;
                                        }
                                    }

                                    for(int jj = 0; jj < thisProc.size; jj++) {
                                        thisProc.pageTable[jj] = *virtMemFreePages.begin();
                                        virtMemFreePages.erase(thisProc.pageTable[jj]);
                                        virtMem[thisProc.pageTable[jj]] = thisProc.pid;
                                    }

                                    procsInVirtMem.push_back(thisProc);
                                    procsInMainMem.erase(toerase);
                                } else {
                                    outputFile << "Error: cannot move the process in main memory to virtual memory - Not enough space" << endl;
                                    break;
                                }
                            }
                        }

                        // cout << "required size is " << toswapin.size << endl;
                        if(toswapin.size <= mainMemFreePages.size()) {
                            // cout << "yes" << endl;
                            // cout << "process name " << toswapin.name << endl;
                            for(int jj = 0; jj < toswapin.pageTable.size(); jj++) {
                                virtMemFreePages.insert(toswapin.pageTable[jj]);
                            }
                            // cout << "hi" << endl;
                            for(int jj = 0; jj < virtMem.size(); jj++) {
                                if(virtMem[jj] == toswapin.pid) {
                                    virtMem[jj] = 0;
                                }
                            }
                            for(int ii = 0; ii < toswapin.size; ii++) {
                                toswapin.pageTable[ii] = *mainMemFreePages.begin();
                                mainMemFreePages.erase(toswapin.pageTable[ii]);
                                mainMem[toswapin.pageTable[ii]] = thisPid;
                            }
                            procsInMainMem.push_back(toswapin);
                            auto toerase = procsInVirtMem.begin();
                            for(toerase = procsInVirtMem.begin(); toerase != procsInVirtMem.end(); toerase++) {
                                if(toerase->pid == toswapin.pid) {
                                    break;
                                }
                            }
                            procsInVirtMem.erase(toerase);
                        }
                        break;
                    }
                }
                if(!found) {
                    outputFile << "Invalid process id " << thisPid << " specified" << endl;
                } else {
                    // cout << "After swapping in " << thisPid << endl;
                    outputFile << "Swapped in the process " << thisPid << " into main memory" << endl;
                    // cout << "---Main memory contents---" << endl;
                    // for(auto ii: procsInMainMem) {
                    //     cout << ii.pid << endl;
                    // }

                    // cout << "---Virt memory contents---" << endl;
                    // for(auto ii: procsInVirtMem) {
                    //     cout << ii.pid << endl;
                    // }
                }
            } else if(currComm == "pte") {
                stringstream ss(command);
                string cmd;
                ss >> cmd;
                string herepid;
                ss >> herepid;
                int heepid = stoi(herepid);
                string ofile;
                ss >> ofile;
                ofile = ofile + ".txt";
                ofstream ofileName(ofile, ios_base::app);
                if(!ofileName.is_open()) {
                    outputFile << "Error: could not open output file." << endl;
                } 
                ofileName << "[" << dateTimeStr << "]" << endl;
                bool breakOrNah = false;
                for(int ii = 0; ii < procsInMainMem.size(); ii++) {
                    if(procsInMainMem[ii].pid == heepid) {
                        ofileName << "Process id " << heepid << " is in main memory" << endl;
                        for(int jj = 0; jj < procsInMainMem[ii].pageTable.size(); jj++) {
                            ofileName << "Logical Page number: " << jj << " Physical Page number: " << procsInMainMem[ii].pageTable[jj] << endl;
                        }
                        breakOrNah = true;
                        break;
                    }
                }
                
                bool newBool = false;
                if(!breakOrNah) {
                    for(int ii = 0; ii < procsInVirtMem.size(); ii++) {
                        if(procsInVirtMem[ii].pid == heepid) {
                            breakOrNah = true;
                            ofileName << "Process id " << heepid << " is in virtual memory" << endl;
                            for(int jj = 0; jj < procsInVirtMem[ii].pageTable.size(); jj++) {
                                ofileName << "Logical Page number: " << jj << " Physical Page number: " << procsInVirtMem[ii].pageTable[jj] << endl;
                            }
                            newBool = true;
                            break;
                        }
                    }
                }
                

                if(!newBool && !breakOrNah) {
                    ofileName << "Process with the given pid is not found" << endl;
                }

                ofileName.close();
            } else if(currComm == "run") {
                stringstream ss(command);
                string cmd;
                ss >> cmd;
                string pid;
                ss >> pid;
               int thisPid = stoi(pid);
               bool broke = false;
               bool inMainMem = false;

                for(int ii = 0; ii < procsInMainMem.size(); ii++) {
                    if(procsInMainMem[ii].pid == thisPid) {
                        inMainMem = true;
                        string _name = procsInMainMem[ii].name;
                        ifstream procc(_name);
                        string thisLine;
                        vector<string> procCommands;
                        while(getline(procc, thisLine)) {
                            // cout << "got the line " << thisLine << endl;
                            procCommands.push_back(thisLine);
                        }

                        for(const string& thisCommand: procCommands) {
                            stringstream ss(thisCommand);
                            string wtd;
                            ss >> wtd;
                            if(wtd == "load") {
                                outputFile << "Command: " << thisCommand << "; ";
                                int addr, val;
                                string _addr, _val;
                                ss >> _val >> _addr;
                                _val = _val.substr(0, _val.length() - 1);
                                // cout << _val << " at " << _addr << endl;

                                addr = stoi(_addr);
                                val = stoi(_val);

                                int procSizeInBytes = (procsInMainMem[ii].size * pageSize);
                                // cout << "proc size in bytes is " << procSizeInBytes << endl;

                                if(addr < procSizeInBytes - 1) {
                                    procsInMainMem[ii].addrToVals[addr] = val;
                                    outputFile << "Result: Value in addr " << addr << " = " << val << endl;
                                } else {
                                    outputFile << "Invalid Memory Address " << addr << " specified for process id " << thisPid << endl;
                                    broke = true;
                                }
                            } else if(wtd == "add") {
                                outputFile << "Command: " << thisCommand << "; ";
                                int procSizeInBytes = (procsInMainMem[ii].size * pageSize);
                                int addr1, addr2, addr3;
                                string _addr1, _addr2, _addr3;
                                ss >> _addr1 >> _addr2 >> _addr3;
                                _addr1 = _addr1.substr(0, _addr1.length() - 1);
                                _addr2 = _addr2.substr(0, _addr2.length() - 1);
                                // cout << "hi" << endl;

                                addr1 = stoi(_addr1);
                                addr2 = stoi(_addr2);
                                addr3 = stoi(_addr3);

                                // cout << "hi idnf" << endl;

                                if(addr1 < procSizeInBytes - 1 && addr2 < procSizeInBytes - 1 && addr3 < procSizeInBytes - 1) {
                                    procsInMainMem[ii].addrToVals[addr3] = procsInMainMem[ii].addrToVals[addr1] + procsInMainMem[ii].addrToVals[addr2];
                                    outputFile << "Result: Value in addr " << addr1 << " = " << procsInMainMem[ii].addrToVals[addr1] << ", addr " << addr2 << " = " << procsInMainMem[ii].addrToVals[addr2] << ", addr " << addr3 << " = " << procsInMainMem[ii].addrToVals[addr3] << endl;
                                } else if(addr1 > procSizeInBytes - 1) {
                                    outputFile << "Invalid Memory Address " << addr1 << " specified for process id " << thisPid << endl;
                                    broke = true;
                                } else if(addr2 > procSizeInBytes - 1) {
                                    outputFile << "Invalid Memory Address " << addr2 << " specified for process id " << thisPid << endl;
                                    broke = true;
                                } else {
                                    outputFile << "Invalid Memory Address " << addr3 << " specified for process id " << thisPid << endl;
                                    broke = true;
                                }
                            } else if(wtd == "sub") {
                                outputFile << "Command: " << thisCommand << "; ";
                                int procSizeInBytes = (procsInMainMem[ii].size * pageSize);
                                int addr1, addr2, addr3;
                                string _addr1, _addr2, _addr3;
                                ss >> _addr1 >> _addr2 >> _addr3;
                                _addr1 = _addr1.substr(0, _addr1.length() - 1);
                                _addr2 = _addr2.substr(0, _addr2.length() - 1);

                                addr1 = stoi(_addr1);
                                addr2 = stoi(_addr2);
                                addr3 = stoi(_addr3);

                                if(addr1 < procSizeInBytes - 1 && addr2 < procSizeInBytes - 1 && addr3 < procSizeInBytes - 1) {
                                    procsInMainMem[ii].addrToVals[addr3] = procsInMainMem[ii].addrToVals[addr1] - procsInMainMem[ii].addrToVals[addr2];
                                    outputFile << "Result: Value in addr " << addr1 << " = " << procsInMainMem[ii].addrToVals[addr1] << ", addr " << addr2 << " = " << procsInMainMem[ii].addrToVals[addr2] << ", addr " << addr3 << " = " << procsInMainMem[ii].addrToVals[addr3] << endl;
                                } else if(addr1 > procSizeInBytes - 1) {
                                    outputFile << "Invalid Memory Address " << addr1 << " specified for process id " << thisPid << endl;
                                    broke = true;
                                } else if(addr2 > procSizeInBytes - 1) {
                                    outputFile << "Invalid Memory Address " << addr2 << " specified for process id " << thisPid << endl;
                                    broke = true;
                                } else {
                                    outputFile << "Invalid Memory Address " << addr3 << " specified for process id " << thisPid << endl;
                                    broke = true;
                                }
                            } else if(wtd == "print") {
                                outputFile << "Command: " << thisCommand << "; ";
                                int procSizeInBytes = (procsInMainMem[ii].size * pageSize);
                                string _addr;
                                ss >> _addr;
                                int addr = stoi(_addr);

                                if(addr < procSizeInBytes - 1) {
                                    outputFile << "Result: Value in addr " << addr << " = " << procsInMainMem[ii].addrToVals[addr] << endl;
                                } else {

                                    outputFile << "Invalid Memory Address " << addr << " specified for process id " << thisPid << endl;
                                    broke = true;
                                }
                            } else {
                                ;
                            }

                            if(broke) {
                                break;
                            }
                        }
                        // outputFile << "xx Pushed " << procsInMainMem[ii].pid << " into lru" << endl;
                        LRUMainMem.push_back(procsInMainMem[ii]);
                    }
                }

                for(auto ii = procsInVirtMem.begin(); ii != procsInVirtMem.end(); ii++) {
                    if(ii->pid == thisPid) {
                        bool done = false;
                        if(ii->size > mainMemSizeInPages) {
                            outputFile << "Process id " << thisPid << " cannot be swapped into main memory" << endl;
                            break;
                        }
                        // cout << "found the process " << ii->pid << " in virt meme with size " << ii->size << endl;
                        Process toswapin = Process(ii->pid, ii->name, ii->size, true, ii->addrToVals);
                        while(ii->size > mainMemFreePages.size()) {
                            // cout << "here " << endl;
                            if(!LRUMainMem.empty()) {
                                // outputFile << "files in lru" << endl;
                                // for(auto ii: LRUMainMem) {
                                //     // outputFile << ii.pid << " size " << ii.size << endl;
                                // }
                                // cout << "hello" << endl;
                                Process toswapout = LRUMainMem[0];
                                if(virtMemFreePages.size() >= toswapout.size) {
                                    auto toerase = procsInMainMem.begin();
                                    for(toerase = procsInMainMem.begin(); toerase != procsInMainMem.end(); toerase++) {
                                        if(toswapout.pid == toerase->pid) {
                                            break;
                                        }
                                    }
                                    Process thisProc(toswapout.pid, toswapout.name, toswapout.size, false, toswapout.addrToVals);
                                    for(int jj = 0; jj < toerase->pageTable.size(); jj++) {
                                        mainMemFreePages.insert(toerase->pageTable[jj]);
                                    }

                                    for(int jj = 0; jj < mainMem.size(); jj++) {
                                        if(mainMem[jj] == thisProc.pid) {
                                            mainMem[jj] = 0;
                                        }
                                    }

                                    for(int jj = 0; jj < thisProc.size; jj++) {
                                        thisProc.pageTable[jj] = *virtMemFreePages.begin();
                                        virtMemFreePages.erase(thisProc.pageTable[jj]);
                                        virtMem[thisProc.pageTable[jj]] = thisProc.pid;
                                    }

                                    procsInVirtMem.push_back(thisProc);
                                    LRUMainMem.erase(LRUMainMem.begin());
                                    procsInMainMem.erase(toerase);
                                } else {
                                    outputFile << "Error: cannot move the least recently used process to virtual memory" << endl;
                                    break;
                                }
                            } else {
                                // cout << "no bye" << endl;
                                Process toswapout = procsInMainMem[0];
                                if(virtMemFreePages.size() >= toswapout.size) {
                                    auto toerase = procsInMainMem.begin();
                                    for(toerase = procsInMainMem.begin(); toerase != procsInMainMem.end(); toerase++) {
                                        if(toswapout.pid == toerase->pid) {
                                            break;
                                        }
                                    }

                                    Process thisProc(toswapout.pid, toswapout.name, toswapout.size, false);
                                    for(int jj = 0; jj < thisProc.pageTable.size(); jj++) {
                                        mainMemFreePages.insert(thisProc.pageTable[jj]);
                                    }

                                    for(int jj = 0; jj < mainMem.size(); jj++) {
                                        if(mainMem[jj] == thisProc.pid) {
                                            mainMem[jj] = 0;
                                        }
                                    }

                                    for(int jj = 0; jj < thisProc.size; jj++) {
                                        thisProc.pageTable[jj] = *virtMemFreePages.begin();
                                        virtMemFreePages.erase(thisProc.pageTable[jj]);
                                        virtMem[thisProc.pageTable[jj]] = thisProc.pid;
                                    }

                                    procsInVirtMem.push_back(thisProc);
                                    LRUMainMem.erase(LRUMainMem.begin());
                                    procsInMainMem.erase(toerase);
                                } else {
                                    outputFile << "Error: cannot move the process in main memory to virtual memory - Not enough space" << endl;
                                    break;
                                }
                            }
                        }

                        if(toswapin.size <= mainMemFreePages.size()) {
                            // cout << "yes" << endl;
                            // cout << "process name " << toswapin.name << endl;
                            for(int jj = 0; jj < toswapin.pageTable.size(); jj++) {
                                virtMemFreePages.insert(toswapin.pageTable[jj]);
                            }
                            // cout << "hi" << endl;
                            for(int jj = 0; jj < virtMem.size(); jj++) {
                                if(virtMem[jj] == toswapin.pid) {
                                    virtMem[jj] = 0;
                                }
                            }
                            for(int ii = 0; ii < toswapin.size; ii++) {
                                toswapin.pageTable[ii] = *mainMemFreePages.begin();
                                mainMemFreePages.erase(toswapin.pageTable[ii]);
                                mainMem[toswapin.pageTable[ii]] = thisPid;
                            }
                            procsInMainMem.push_back(toswapin);
                            auto toerase = procsInVirtMem.begin();
                            for(toerase = procsInVirtMem.begin(); toerase != procsInVirtMem.end(); toerase++) {
                                if(toerase->pid == toswapin.pid) {
                                    break;
                                }
                            }
                            procsInVirtMem.erase(toerase);
                        }
                        
                        // for(int ii = 0; ii < procsInMainMem.size(); ii++) {
                        for(int jj = 0; jj < procsInMainMem.size(); jj++) {
                            if(procsInMainMem[jj].pid == thisPid) {
                                string _name = procsInMainMem[jj].name;
                                ifstream procc(_name);
                                string thisLine;
                                vector<string> procCommands;
                                while(getline(procc, thisLine)) {
                                    // cout << "got the line " << thisLine << endl;
                                    procCommands.push_back(thisLine);
                                }

                                for(const string& thisCommand: procCommands) {
                                    stringstream ss(thisCommand);
                                    string wtd;
                                    ss >> wtd;
                                    if(wtd == "load") {
                                        outputFile << "Command: " << thisCommand << "; ";
                                        int addr, val;
                                        string _addr, _val;
                                        ss >> _val >> _addr;
                                        _val = _val.substr(0, _val.length() - 1);
                                        // cout << _val << " at " << _addr << endl;

                                        addr = stoi(_addr);
                                        val = stoi(_val);

                                        int procSizeInBytes = (procsInMainMem[jj].size * pageSize);
                                        // cout << "proc size in bytes is " << procSizeInBytes << endl;

                                        if(addr < procSizeInBytes - 1) {
                                            procsInMainMem[jj].addrToVals[addr] = val;
                                            outputFile << "Result: Value in addr " << addr << " = " << val << endl;
                                        } else {
                                            outputFile << "Invalid Memory Address " << addr << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        }
                                    } else if(wtd == "add") {
                                        outputFile << "Command: " << thisCommand << "; ";
                                        int procSizeInBytes = (procsInMainMem[jj].size * pageSize);
                                        int addr1, addr2, addr3;
                                        string _addr1, _addr2, _addr3;
                                        ss >> _addr1 >> _addr2 >> _addr3;
                                        _addr1 = _addr1.substr(0, _addr1.length() - 1);
                                        _addr2 = _addr2.substr(0, _addr2.length() - 1);
                                        // cout << "hi" << endl;

                                        addr1 = stoi(_addr1);
                                        addr2 = stoi(_addr2);
                                        addr3 = stoi(_addr3);

                                        // cout << "hi idnf" << endl;

                                        if(addr1 < procSizeInBytes - 1 && addr2 < procSizeInBytes - 1 && addr3 < procSizeInBytes - 1) {
                                            procsInMainMem[jj].addrToVals[addr3] = procsInMainMem[jj].addrToVals[addr1] + procsInMainMem[jj].addrToVals[addr2];
                                            outputFile << "Result: Value in addr " << addr1 << " = " << procsInMainMem[jj].addrToVals[addr1] << ", addr " << addr2 << " = " << procsInMainMem[jj].addrToVals[addr2] << ", addr " << addr3 << " = " << procsInMainMem[jj].addrToVals[addr3] << endl;
                                        } else if(addr1 > procSizeInBytes - 1) {
                                            outputFile << "Invalid Memory Address " << addr1 << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        } else if(addr2 > procSizeInBytes - 1) {
                                            outputFile << "Invalid Memory Address " << addr2 << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        } else {
                                            outputFile << "Invalid Memory Address " << addr3 << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        }
                                    } else if(wtd == "sub") {
                                        outputFile << "Command: " << thisCommand << "; ";
                                        int procSizeInBytes = (procsInMainMem[jj].size * pageSize);
                                        int addr1, addr2, addr3;
                                        string _addr1, _addr2, _addr3;
                                        ss >> _addr1 >> _addr2 >> _addr3;
                                        _addr1 = _addr1.substr(0, _addr1.length() - 1);
                                        _addr2 = _addr2.substr(0, _addr2.length() - 1);

                                        addr1 = stoi(_addr1);
                                        addr2 = stoi(_addr2);
                                        addr3 = stoi(_addr3);

                                        if(addr1 < procSizeInBytes - 1 && addr2 < procSizeInBytes - 1 && addr3 < procSizeInBytes - 1) {
                                            procsInMainMem[jj].addrToVals[addr3] = procsInMainMem[jj].addrToVals[addr1] - procsInMainMem[jj].addrToVals[addr2];
                                            outputFile << "Result: Value in addr " << addr1 << " = " << procsInMainMem[jj].addrToVals[addr1] << ", addr " << addr2 << " = " << procsInMainMem[jj].addrToVals[addr2] << ", addr " << addr3 << " = " << procsInMainMem[jj].addrToVals[addr3] << endl;
                                        } else if(addr1 > procSizeInBytes - 1) {
                                            outputFile << "Invalid Memory Address " << addr1 << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        } else if(addr2 > procSizeInBytes - 1) {
                                            outputFile << "Invalid Memory Address " << addr2 << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        } else {
                                            outputFile << "Invalid Memory Address " << addr3 << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        }
                                    } else if(wtd == "print") {
                                        outputFile << "Command: " << thisCommand << "; ";
                                        int procSizeInBytes = (procsInMainMem[jj].size * pageSize);
                                        string _addr;
                                        ss >> _addr;
                                        int addr = stoi(_addr);

                                        if(addr < procSizeInBytes - 1) {
                                            outputFile << "Result: Value in addr " << addr << " = " << procsInMainMem[jj].addrToVals[addr] << endl;
                                        } else {

                                            outputFile << "Invalid Memory Address " << addr << " specified for process id " << thisPid << endl;
                                            broke = true;
                                        }
                                    } else {
                                        ;
                                    }

                                    if(broke) {
                                        break;
                                    }
                                    // cout << "hi" << endl;
                                }
                                // cout << "all proc commands done" << endl;
                                done = true;
                                // outputFile << "Pushed " << procsInMainMem[jj].pid << " into lru" << endl;
                                LRUMainMem.push_back(procsInMainMem[jj]);
                                break;
                            }
                            
                        }
                        if(done) {
                            break;
                        }

                    }
                }
            } else if(currComm == "pteall") {
                stringstream ss(command);
                string cmd; 
                ss >> cmd;
                string ofile;
                ss >> ofile;
                ofile = ofile + ".txt";
                ofstream ofileName(ofile, ios_base::app);
                if(!ofileName.is_open()) {
                    outputFile << "Error: could not open output file." << endl;
                } 
                ofileName << "[" << dateTimeStr << "]" << endl;
                sort(procsInMainMem.begin(), procsInMainMem.end(), compareByPid);
                ofileName << "Processes in main memory" << endl;
                for(int ii = 0; ii < procsInMainMem.size(); ii++) {
                    ofileName << "Process id " << procsInMainMem[ii].pid << endl;
                    for(int jj = 0; jj < procsInMainMem[ii].pageTable.size(); jj++) {
                        ofileName << "Logical Page number: " << jj << " Physical Page number: " << procsInMainMem[ii].pageTable[jj] << endl;
                    }
                }

                sort(procsInVirtMem.begin(), procsInVirtMem.end(), compareByPid);
                ofileName << "Processes in virtual memory" << endl;
                for(int ii = 0; ii < procsInVirtMem.size(); ii++) {
                    ofileName << "Process id " << procsInVirtMem[ii].pid << endl;
                    for(int jj = 0; jj < procsInVirtMem[ii].pageTable.size(); jj++) {
                        ofileName << "Logical Page number: " << jj << " Physical Page number: " << procsInVirtMem[ii].pageTable[jj] << endl;
                    }
                }
            } else if(currComm == "exit") {
                break;
            } else if(currComm == "kill") {
                stringstream ss(command);
                string cmd;
                ss >> cmd;
                string pid;
                ss >> pid;
                int thisPid = stoi(pid);
                for(int ii = 0; ii < procsInMainMem.size(); ii++) {
                    if(procsInMainMem[ii].pid == thisPid) {
                        auto toerase = procsInMainMem.begin();
                        for(toerase = procsInMainMem.begin(); toerase != procsInMainMem.end(); toerase++) {
                            if(thisPid == toerase->pid) {
                                break;
                            }
                        }

                        // Process thisProc(toswapout.pid, toswapout.name, toswapout.size, false);
                        for(int jj = 0; jj < procsInMainMem[ii].pageTable.size(); jj++) {
                            mainMemFreePages.insert(procsInMainMem[ii].pageTable[jj]);
                        }

                        for(int jj = 0; jj < mainMem.size(); jj++) {
                            if(mainMem[jj] == thisPid) {
                                mainMem[jj] = 0;
                            }
                        }
                        procsInMainMem.erase(toerase);
                        outputFile << "Killed the process with pid " << thisPid << endl;
                    }
                }

                for(int ii = 0; ii < procsInVirtMem.size(); ii++) {
                    if(procsInVirtMem[ii].pid == thisPid) {
                        auto toerase = procsInVirtMem.begin();
                        for(toerase = procsInVirtMem.begin(); toerase != procsInVirtMem.end(); toerase++) {
                            if(thisPid == toerase->pid) {
                                break;
                            }
                        }

                        // Process thisProc(toswapout.pid, toswapout.name, toswapout.size, false);
                        for(int jj = 0; jj < procsInVirtMem[ii].pageTable.size(); jj++) {
                            virtMemFreePages.insert(procsInVirtMem[ii].pageTable[jj]);
                        }

                        for(int jj = 0; jj < virtMem.size(); jj++) {
                            if(virtMem[jj] == thisPid) {
                                virtMem[jj] = 0;
                            }
                        }
                        procsInVirtMem.erase(toerase);
                        outputFile << "Killed the process with pid " << thisPid << endl;
                    }
                }
            } else if(currComm == "listpr") {
                sort(procsInMainMem.begin(), procsInMainMem.end(), compareByPid);
                sort(procsInVirtMem.begin(), procsInVirtMem.end(), compareByPid);
                outputFile << "Processes in main memory" << endl;
                for(int ii = 0; ii < procsInMainMem.size(); ii++) {
                    outputFile << "Process id " << procsInMainMem[ii].pid << endl;
                }
                outputFile << "Processes in virtual memory" << endl;
                for(int ii = 0; ii < procsInVirtMem.size(); ii++) {
                    outputFile << "Process id " << procsInVirtMem[ii].pid << endl;
                }
            } else if(currComm == "print") {
                stringstream ss(command);
                string cmd; 
                ss >> cmd;
                string _memloc, _length;
                ss >> _memloc >> _length;
                int memloc, length;
                memloc = stoi(_memloc);
                length = stoi(_length);

                for(int ii = memloc; ii < memloc + length; ii++) {
                    int pageNumber = ii / pageSize;
                    int proc = mainMem[pageNumber];
                    if(proc == 0) {
                        outputFile << "Value of " << ii << ": " << 0 << endl;
                    }
                    int offset = ii % pageSize;
                    // cout << "pageno: " << pageNumber << " proc: " << proc << " offset: " << offset << endl;

                    for(int jj = 0; jj < procsInMainMem.size(); jj++) {
                        if(procsInMainMem[jj].pid == proc) {
                            for(int kk = 0; kk < procsInMainMem[jj].size; kk++) {
                                if(procsInMainMem[jj].pageTable[kk] == pageNumber) {
                                    outputFile << "Value of " << ii << ": " << procsInMainMem[jj].addrToVals[offset + (kk * pageSize)] << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}