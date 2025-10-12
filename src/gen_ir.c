#include "tane.h"

static IR ir_head;
static IR* ir_tail = &ir_head;

static void gen_stmt(ASTNode* node);
static Operand* gen_expr(ASTNode* node);
static IR* new_IR(IRCmd cmd, Operand* lhs, Operand* rhs, Operand* target);
static Operand* new_Operand();
static Operand* new_ImmOperand(int val);

IR* gen_ir(ASTNode* node) {
    gen_stmt(node);
    return ir_head.next;
}

static void gen_stmt(ASTNode* node){
    switch(node->kind){
        case ND_RETURN: {
            Operand* ret_val = gen_expr(node->lhs);
            new_IR(IR_RETURN, NULL, NULL, ret_val);
            break;
        }
        default:
            // expr statement
            gen_expr(node);
            break;
    }
}

static Operand* gen_expr(ASTNode* node){
    switch(node->kind){
        case ND_NUM:
            return new_ImmOperand(node->data.val);
        default:
            fprintf(stderr, "Unknown AST node kind: %d\n", node->kind);
            exit(1);
            break;
    }
}

static IR* new_IR(IRCmd cmd, Operand* lhs, Operand* rhs, Operand* target){
    IR* ir = calloc(1, sizeof(IR));
    ir->cmd = cmd;
    ir->lhs = lhs;
    ir->rhs = rhs;
    ir->target = target;

    ir_tail->next = ir;
    ir_tail = ir;

    return ir;
}

static Operand* new_Operand(OperandKind kind){
    Operand* op = calloc(1, sizeof(Operand));
    op->kind = kind;
    return op;
}

static Operand* new_ImmOperand(int val){
    Operand* op = new_Operand(OP_IMM);
    op->val = val;
    return op;
}

// for debugging

/// @brief Print the generated IR code for debugging purposes.
/// @param ir The head of the IR linked list to print.
void print_ir(IR* ir) {

    printf("Generated IR code:\n");
    for (IR* curr = ir; curr != NULL; curr = curr->next) {
        switch (curr->cmd) {
            case IR_FNAME:
                printf("IR_FNAME\n");
                break;
            case IR_FHEAD:
                printf("IR_FHEAD\n");
                break;
            case IR_FTAIL:
                printf("IR_FTAIL\n");
                break;
            case IR_ADD:
                printf("IR_ADD\n");
                break;
            case IR_RETURN:
                printf("IR_RETURN\n");
                break;
            default:
                printf("Unknown IR command: %d\n", curr->cmd);
                break;
        }
    }
}