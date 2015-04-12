#ifndef STATE_H
#define STATE_H

#include "afx.h"

enum StateType
{
      statement, constant , temporality , epsilon , terminal
    // 非终结符	 静态常量	临时型	    空	     终结符
};


class State
{
public:
    State();
    State(const State& other);
    ~State();
    State& operator=(const State& other);
    bool operator==(const State& other);
    
    void init();
    
    void AddBrotherState(State* _brother);
    void AddChildrenState(State* _children);
    
    State* children; //此指针列表里放入state的定义项
    State* brother;
    
    int id;
    
    CHAR* state_const;
    CHAR* state_class;
    CHAR* state_var;

    CHAR* script;
    
    enum StateType state_type; // 0为class的声明, 1为常量, 2为临时型, 3为Epsilon

    int Repeatable; // 0 null  1 ?  2 +  3 *

    bool isList; //如何是List的话，子列表就是每项依次匹配，否则子列表是（a|b）式或匹配
};

#endif // STATE_H
