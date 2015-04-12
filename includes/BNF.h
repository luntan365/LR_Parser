/* 
* @Author: sxf
* @Date:   2014-12-31 08:38:50
* @Last Modified by:   sxf
* @Last Modified time: 2015-01-03 14:32:21
*/

#ifndef BNF_H
#define BNF_H

#include "afx.h"
#include "State.h"
#include <vector>
#include <map>
#include "VMap.h"

using namespace std;

class BNF
{
public:
    BNF() { scriptcode = NULL; }
    ~BNF() {}

    // 通过EBNF得出的root节点，遍历所有可能的BNF范式
    static vector<BNF*> BuildAllBNF(State*,VMap&);
    
    void print_bnf() const;
    void print_bnf(int) const;

    void addBNFdata(State* _s) { BNFdata.push_back(_s); }
    
    // ======= setter and getter ========
    
    void setRoot(State* _s) { root = _s; }
    State* getRoot() const { return root; }
    const vector<State*>& getBNFdata() const { return BNFdata; }
    void setID(int _id) { id = _id; }
    int getID() const { return id; }
    CHAR* getScript() { return script; }
    int& getScriptCode() { return scriptcode; }
protected:
    static void BuildFromNode(State*,VMap&);
    
    
private:
    int id;
    // 一条BNF范式
    State* root;
    vector<State*> BNFdata;
    CHAR* script;
    int scriptcode;
    
    static int temp_size;
    static vector<BNF*> bnfs;
};


#endif // BNF_H
