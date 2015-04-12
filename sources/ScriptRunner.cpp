#include "ScriptRunner.h"
#include <string>

static ScriptRunner* runner;
static int cachecode;
static Grammer_Node* rootnode;
static IDTable* idtable;
static CodeGenerator* generator;

static int CallFunc() {
    vector< pair<CHAR*, Grammer_Node*> >& env = runner->getEnv();
    for (auto p : env) {
        lua_rawgeti(runner->L,LUA_REGISTRYINDEX, p.second->lua_data);
    }
    lua_call(runner->L, env.size(), 1);
    return luaL_ref(runner->L,LUA_REGISTRYINDEX);//获得引用的索引,并pop当前栈顶
}

static int getLuaFunc(lua_State * L)
{
//    lua_pushstring(L, "hello");
    lua_pushvalue(L,-1);//复制栈顶
    cachecode = luaL_ref(L,LUA_REGISTRYINDEX);//获得引用的索引,并pop当前栈顶
    rootnode->lua_data = CallFunc();
    return 0;
}


// lua 输入参数：符号x 返回 lua表t 包含t.kind t.type t.level t.address
static int find_id(lua_State * L) {
    const char* str = lua_tostring(L,-1);
    const id* pid = idtable->find(str);
    printf("find %s\n",str);
    if (pid == 0) {
        lua_pushnil(L);
        return 1;
    }
    // 构造一张lua表将符号表的项目返回回去
    lua_newtable(L);
    lua_pushinteger(L,pid->kind);
    lua_setfield(L,-2,"kind");
    lua_pushinteger(L,pid->type);
    lua_setfield(L,-2,"type");
    lua_pushinteger(L,pid->level);
    lua_setfield(L,-2,"level");
    lua_pushinteger(L,pid->address);
    lua_setfield(L,-2,"address");
    return 1;
}

// lua 输入参数：符号x kind type address 返回bool 是否保存成功
static int save_id(lua_State * L) {
    int kind,type,address;
    const char* str = lua_tostring(L,-4);
    kind = lua_tointeger(L,-3);
    type = lua_tointeger(L,-2);
    address = lua_tointeger(L,-1);
    lua_pop(L,4);
    const id* pid = idtable->find(str);
    if (pid != NULL && pid->level == idtable->getLevel()) {
        lua_pushboolean(L, 0); return 1; // 判断是同级要存入相同符号，要报符号表异常
    }

    id* newid = new id();
    newid->level = idtable->getLevel();
    newid->kind = kind;
    newid->type = type;
    newid->address = address;
    idtable->insert(str,newid);
    printf("save: %s\n",str);
    lua_pushboolean(L, 1); return 1;
}

// lua 输入参数：指令p 参数a 返回bool 是否生成成功
static int make_code(lua_State * L) {
    int p,l,a;
    if (lua_gettop(L)<=1) {
        p = lua_tointeger(L,-1); lua_pop(L,1);
        l = -1; a = -1;
    }
    else if (lua_gettop(L)<=2) {
        a = lua_tointeger(L,-1); lua_pop(L,1);
        p = lua_tointeger(L,-1); lua_pop(L,1);
        l = -1;
    } else {
        a = lua_tointeger(L,-1); lua_pop(L,1);
        l = lua_tointeger(L,-1); lua_pop(L,1);
        p = lua_tointeger(L,-1); lua_pop(L,1);
    }
    if (l != -1) {
        if (l == 0) // 表示寻找的是全局变量
            l = -1; // VM中用-1表示是一个全局变量
        else
            l = idtable->getLevel() - l;
        generator->Write(p,l,a);
    } else {
        if (a == -1) {
            generator->Write(p);
        } else {
            generator->Write(p,a);
        }
    }
    lua_pushboolean(L, 1); return 1;
}


static int write_code(lua_State * L) {
    int p, a;
    a = lua_tointeger(L,-1); lua_pop(L,1);
    p = lua_tointeger(L,-1); lua_pop(L,1);
    generator->WritePointer(p,a);
    return 0;
}


static int now_pointer(lua_State * L) {
    lua_pushinteger(L, generator->getPointer()); return 1;
}


static int push_stack(lua_State * L) {
    printf("stack push!\n");
    idtable->push();
    return 0;
}

static int pop_stack(lua_State * L) {
    printf("stack pop!\n");
    idtable->pop();
    return 0;
}

static int exitfunc(lua_State * L) {
    // TODO: 这里需要加入一个打断方法,使得语法分析器能够被异常打断
    return 0;
}

ScriptRunner::ScriptRunner()
{
    Init();
    id_table = new IDTable();
    idtable = this->id_table;
    code_generator = new CodeGenerator();
    generator = this->code_generator;
}

ScriptRunner::~ScriptRunner() {
    lua_close(L);
    delete id_table;
    delete code_generator;
}

void ScriptRunner::Init() {
    if (L == NULL) {
        L = luaL_newstate();
        luaL_openlibs(L);
        lua_register(L,"Cfunc",getLuaFunc);
        lua_register(L,"find_id",find_id);
        lua_register(L,"save_id",save_id);
        lua_register(L,"make_code",make_code);
        lua_register(L,"push_stack",push_stack);
        lua_register(L,"pop_stack",pop_stack);
        lua_register(L,"now_pointer",now_pointer);
        lua_register(L,"write_code",write_code);
        lua_register(L,"exit",exitfunc);
    }   
}

char* ScriptRunner::WCharToChar(wchar_t* data,int& size) {
    int len = STRLEN(data);
    char* newdata = new char[len+1];
    int i = 0;
    while (data[i] != 0) {
        if (data[i] < 255)
        newdata[i] = (char)data[i];
        ++i;
    }
    newdata[i] = 0;
    size = i;
    return newdata;
}

int ScriptRunner::MakeEnv(CHAR* name, Grammer_Node* node) {
    env.push_back(make_pair(name,node));
}

int ScriptRunner::MakeNewLuaTable(Token* t) {
    lua_newtable(L);
    // TODO: 这里需要进行Unicode支持，先暂时使用简单的转换函数
    int len;
    char* str = WCharToChar(t->pToken,len);
    lua_pushstring(L,str);
    lua_setfield(L,-2,"val");
    lua_pushinteger(L,t->type);
    lua_setfield(L,-2,"type");
    delete[] str;
    return luaL_ref(L,LUA_REGISTRYINDEX);//获得引用的索引,并pop当前栈顶
}

char* ScriptRunner::addFunction(CHAR* data) {
    char* func;
    string params;
    int len = 0; bool isfirst = true;
    for (auto p : env) {
        int namelen;
        char* name = WCharToChar(p.first,namelen);
        if (isfirst) isfirst = false; else { params.push_back(','); ++len; }
        params.append(name);
        len += namelen;
        delete[] name;
    }

    int codelen;
    char* code = WCharToChar(data,codelen);
    len += codelen; len += 30;
    func = new char[len];
    sprintf(func,"Cfunc(function(%s) %s end)\n",params.c_str(),code);
    delete[] code;
    return func;
}




int ScriptRunner::Run(int& code, CHAR* data, Grammer_Node* node) {
    int error;
    runner = this; rootnode = node;
    printf("LUA STACK: %d\n",lua_gettop(L));
    if (code == 0) {
        char* buff = addFunction(data);
        printf("load script: %s",buff);
        error = luaL_loadbuffer(L, buff, strlen(buff) ,"chunk") //加载当前script
                | lua_pcall(L, 0, 0, 0); // 巧妙的利用或运算符，前面若成功返回0，则执行后面的
        delete[] buff;
        if (error) goto LUA_ERROR;
        code = cachecode; // 将C函数处理得到的缓存代码返回
    } else {
        printf("run script: ");
        lua_rawgeti(L,LUA_REGISTRYINDEX, code);
        node->lua_data = CallFunc();
    }

    return 0;
LUA_ERROR:
    printf("%s", lua_tostring(L, -1));
    lua_pop(L, 1);/* pop error message from the stack */
    return -1;
}
