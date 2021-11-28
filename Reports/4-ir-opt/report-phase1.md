# Lab4 实验报告-阶段一

小组成员 姓名 学号

## 实验要求

请按照自己的理解，写明本次实验需要干什么

## 思考题
### LoopSearch
1. 循环数据结构: CFGNodePtrSet
```cpp
struct CFGNode;
struct CFGNode
{
    std::unordered_set<CFGNodePtr> succs;
    std::unordered_set<CFGNodePtr> prevs;
    BasicBlock *bb;
    int index;   // the index of the node in CFG
    int lowlink; // the min index of the node in the strongly connected componets
    bool onStack;
};
using CFGNodePtrSet = std::unordered_set<CFGNode *>;
```
2. 入口
對於當前的scc的每個節點，如果有不屬於該scc的就是根節點
```cpp
for (auto n : *set)
    for (auto prev : n->prevs)
        if (set->find(prev) == set->end())
            base = n;
```
3. 嵌套
使用 reserved 集合，加入外層已經訪問過的循環入口，並在它的前驅和後繼中刪除自己
4. 代码
```cpp
reserved.insert(base);
nodes.erase(base);
for (auto su : base->succs)
{
    su->prevs.erase(base);
}
for (auto prev : base->prevs)
{
    prev->succs.erase(base);
}
```
刪除节点，破坏强連通分量，但是内層循環仍然存在

### Mem2reg
1. dominance
   1. 支配性：所有从入口到节点n的路径都必须经过节点d，称为d支配n.
   2. 严格支配性: $`d\gg n, d\neq n`$
   3. 直接支配性：d直接支配节点n却又不直接支配任何直接支配节点n的其他节点，可以理解为d离n“最近”
   4. 支配边界：d的支配性停止的点集
2. phi: 控制流语句的后继基本块无法判断使用的变量来自哪一个路径，引入根据控制流选择多个前驱的变量
3. 多次load与store
4. 减少了分配函数参数空间的load和store；减少多次load同一个地址；分支语句中的store改为branch后的phi语句
5. 指出放置phi节点的代码，并解释是如何使用支配树的信息的。需要给出代码中的成员变量或成员函数名称。TODO

### 代码阅读总结

此次实验有什么收获

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
