#include "afx.h"
#include "Grammer_Node.h"
#include "lua.hpp"
#include <vector>
#include "LexInterface.h"
#include "idtable.h"
#include "codegenerator.h"
using namespace std;

#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H



class ScriptRunner
{
public:
    ScriptRunner();
    ~ScriptRunner();
    void Init();
    int MakeEnv(CHAR*, Grammer_Node*);
    int MakeNewLuaTable(Token* t);
    int Run(int&, CHAR*, Grammer_Node*);
    void ClearEnv() { if (env.size() != 0) env.clear(); }
    vector< pair<CHAR*, Grammer_Node*> >& getEnv() { return env; }
    lua_State* L = NULL;

    void WriteFile(const char* path) { code_generator->WriteFile(path); }
private:
    vector< pair<CHAR*, Grammer_Node*> > env;

    IDTable* id_table;
    CodeGenerator* code_generator;

    char* WCharToChar(wchar_t* data,int& size);
    char* addFunction(CHAR* data);
};

#endif // SCRIPTRUNNER_H
