#ifndef _TREE_H_
#define _TREE_H_

#include "Stage.h"
#include <string>
#include<time.h>


//! @brief Tree类，用于记录commit tree
class Tree{
public:

    static void save(const Stage&);//将Stage保存为Tree文件
    static void clear();//清空Tree文件
    static void print_tree();//打印Tree文件

    static bool is_empty();//判断是否为空
private:
    static std::string get_time_to_set();//获取当前时间，用来记录当前的commit时间
};

#endif