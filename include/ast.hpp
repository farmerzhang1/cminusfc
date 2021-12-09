#ifndef _SYNTAX_TREE_HPP_
#define _SYNTAX_TREE_HPP_
extern "C"
{
#include "syntax_tree.h"
    extern syntax_tree *parse(const char *input);
}
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <functional>

enum class CminusType
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VOID
};

enum class RelOp
{
    // <=
    OP_LE,
    // <
    OP_LT,
    // >
    OP_GT,
    // >=
    OP_GE,
    // ==
    OP_EQ,
    // !=
    OP_NEQ
};

enum class AddOp
{
    // +
    OP_PLUS,
    // -
    OP_MINUS
};

enum class MulOp
{
    // *
    OP_MUL,
    // /
    OP_DIV
};

class AST;

struct ASTNode;
struct ASTProgram;
struct ASTDeclaration;
struct ASTNum;
struct ASTVarDeclaration;
struct ASTFunDeclaration;
struct ASTParam;
struct ASTCompoundStmt;
struct ASTStatement;
struct ASTExpressionStmt;
struct ASTSelectionStmt;
struct ASTIterationStmt;
struct ASTReturnStmt;
struct ASTFactor;
struct ASTExpression;
struct ASTVar;
struct ASTAssignExpression;
struct ASTSimpleExpression;
struct ASTAdditiveExpression;
struct ASTTerm;
struct ASTCall;

class ASTVisitor;

class AST
{
public:
    AST() = delete;
    AST(syntax_tree *);
    AST(AST &&tree)
    {
        root = tree.root;
        tree.root = nullptr;
    };
    ASTProgram *get_root() { return root.get(); }
    void run_visitor(ASTVisitor &visitor);
    std::map<AddOp, std::function<int(int, int)>> add_int_map = {
        {AddOp::OP_PLUS, [](int l, int r)
         { return l + r; }},
        {AddOp::OP_MINUS, [](int l, int r)
         { return l - r; }}};
    std::map<AddOp, std::function<float(float, float)>> add_float_map = {
        {AddOp::OP_PLUS, [](float l, float r)
         { return l + r; }},
        {AddOp::OP_MINUS, [](float l, float r)
         { return l - r; }}};
    std::map<MulOp, std::function<int(int, int)>> mul_int_map = {
        {MulOp::OP_MUL, [](int l, int r)
         { return l * r; }},
        {MulOp::OP_DIV, [](int l, int r)
         { return l / r; }}};
    std::map<MulOp, std::function<float(float, float)>> mul_float_map = {
        {MulOp::OP_MUL, [](float l, float r)
         { return l * r; }},
        {MulOp::OP_DIV, [](float l, float r)
         { return l / r; }}};
    std::map<RelOp, std::function<bool(int, int)>> comp_int_map = {
        {RelOp::OP_EQ, [](int l, int r)
         { return l == r; }},
        {RelOp::OP_NEQ, [](int l, int r)
         { return l != r; }},
        {RelOp::OP_GE, [](int l, int r)
         { return l >= r; }},
        {RelOp::OP_GT, [](int l, int r)
         { return l > r; }},
        {RelOp::OP_LE, [](int l, int r)
         { return l <= r; }},
        {RelOp::OP_LT, [](int l, int r)
         { return l < r; }}};
    std::map<RelOp, std::function<bool(float, float)>> comp_float_map = {
        {RelOp::OP_EQ, [](float l, float r)
         { return l == r; }},
        {RelOp::OP_NEQ, [](float l, float r)
         { return l != r; }},
        {RelOp::OP_GE, [](float l, float r)
         { return l >= r; }},
        {RelOp::OP_GT, [](float l, float r)
         { return l > r; }},
        {RelOp::OP_LE, [](float l, float r)
         { return l <= r; }},
        {RelOp::OP_LT, [](float l, float r)
         { return l < r; }}};
    // we assume pointers are non-empty
    bool both_int(ASTNum *l, ASTNum *r);
    float to_f(ASTNum *n);

private:
    ASTNode *transform_node_iter(syntax_tree_node *);
    std::shared_ptr<ASTProgram> root = nullptr;
};

struct ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
    // we should call it: connect all the way down to an ASTNum
    virtual bool is_num() = 0;
};

struct ASTProgram : ASTNode
{
    virtual void accept(ASTVisitor &) override final;
    std::vector<std::shared_ptr<ASTDeclaration>>
        declarations;
    bool is_num() override final { return false; }
};

struct ASTDeclaration : ASTNode
{
    CminusType type;
    std::string id; // 这里是数字 int or float
    virtual ~ASTDeclaration() = default;
    bool is_num() override final { return false; }
};

struct ASTFactor : ASTNode
{
    virtual ~ASTFactor() =default;
    bool is_num() override;
};

struct ASTNum : ASTFactor
{
    virtual void accept(ASTVisitor &) override final;
    CminusType type;
    union
    {
        int i_val;
        float f_val;
    };
    bool is_num() override final { return true; }
};

struct ASTVarDeclaration : ASTDeclaration
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTNum> num; // num若不为空，值为数组类型和数组个数
};

struct ASTFunDeclaration : ASTDeclaration
{
    virtual void accept(ASTVisitor &) override final;
    std::vector<std::shared_ptr<ASTParam>> params;
    std::shared_ptr<ASTCompoundStmt> compound_stmt;
};

struct ASTParam : ASTNode
{
    virtual void accept(ASTVisitor &) override final;
    CminusType type;
    std::string id;
    // true if it is array param
    bool isarray;
    bool is_num() override final { return false; }
};

struct ASTStatement : ASTNode
{
    virtual ~ASTStatement() = default;
    bool is_num() override final { return false; }
};

struct ASTCompoundStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::vector<std::shared_ptr<ASTVarDeclaration>> local_declarations;
    std::vector<std::shared_ptr<ASTStatement>> statement_list;
};

struct ASTExpressionStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
};

struct ASTSelectionStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
    std::shared_ptr<ASTStatement> if_statement;
    // should be nullptr if no else structure exists
    std::shared_ptr<ASTStatement> else_statement;
};

struct ASTIterationStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
    std::shared_ptr<ASTStatement> statement;
};

struct ASTReturnStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    // should be nullptr if return void
    std::shared_ptr<ASTExpression> expression;
};

struct ASTExpression : ASTFactor
{
    bool is_num() override;
};

struct ASTAssignExpression : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTVar> var;
    std::shared_ptr<ASTExpression> expression;
    bool is_num() override final { return false; }
};

struct ASTSimpleExpression : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTAdditiveExpression> additive_expression_l; // lhs is always nonempty
    std::shared_ptr<ASTAdditiveExpression> additive_expression_r;
    RelOp op;
    bool is_num() override final;
};

struct ASTVar : ASTFactor
{
    virtual void accept(ASTVisitor &) override final;
    std::string id;
    // nullptr if var is of int type
    std::shared_ptr<ASTExpression> expression;
};

struct ASTAdditiveExpression : ASTNode
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTAdditiveExpression> additive_expression;
    AddOp op;
    std::shared_ptr<ASTTerm> term;
    bool is_num() override final;
};

struct ASTTerm : ASTNode
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTTerm> term;
    MulOp op;
    std::shared_ptr<ASTFactor> factor;
    bool is_num() override final;
};

struct ASTCall : ASTFactor
{
    virtual void accept(ASTVisitor &) override final;
    std::string id;
    std::vector<std::shared_ptr<ASTExpression>> args;
};

class ASTVisitor
{
public:
    virtual void visit(ASTProgram &) = 0;
    virtual void visit(ASTNum &) = 0;
    virtual void visit(ASTVarDeclaration &) = 0;
    virtual void visit(ASTFunDeclaration &) = 0;
    virtual void visit(ASTParam &) = 0;
    virtual void visit(ASTCompoundStmt &) = 0;
    virtual void visit(ASTExpressionStmt &) = 0;
    virtual void visit(ASTSelectionStmt &) = 0;
    virtual void visit(ASTIterationStmt &) = 0;
    virtual void visit(ASTReturnStmt &) = 0;
    virtual void visit(ASTAssignExpression &) = 0;
    virtual void visit(ASTSimpleExpression &) = 0;
    virtual void visit(ASTAdditiveExpression &) = 0;
    virtual void visit(ASTVar &) = 0;
    virtual void visit(ASTTerm &) = 0;
    virtual void visit(ASTCall &) = 0;
};

class ASTPrinter : public ASTVisitor
{
public:
    virtual void visit(ASTProgram &) override final;
    virtual void visit(ASTNum &) override final;
    virtual void visit(ASTVarDeclaration &) override final;
    virtual void visit(ASTFunDeclaration &) override final;
    virtual void visit(ASTParam &) override final;
    virtual void visit(ASTCompoundStmt &) override final;
    virtual void visit(ASTExpressionStmt &) override final;
    virtual void visit(ASTSelectionStmt &) override final;
    virtual void visit(ASTIterationStmt &) override final;
    virtual void visit(ASTReturnStmt &) override final;
    virtual void visit(ASTAssignExpression &) override final;
    virtual void visit(ASTSimpleExpression &) override final;
    virtual void visit(ASTAdditiveExpression &) override final;
    virtual void visit(ASTVar &) override final;
    virtual void visit(ASTTerm &) override final;
    virtual void visit(ASTCall &) override final;
    void add_depth() { depth += 2; }
    void remove_depth() { depth -= 2; }

private:
    int depth = 0;
};

#endif
