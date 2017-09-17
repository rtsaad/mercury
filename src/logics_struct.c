/*
 * File:    logic_struct.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS
 * Created  on February 26, 2011, 4:09 PM
 * 
 * LICENSE
 *
 * MIT License
 *
 * Copyright LAAS-CNRS / Vertics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * DESCRIPTION
 *
 * This file builds the abstract view of the CTL formula parsed by LEX/YACC.
 * It is used by YACC to construct the abstract tree.
 * 
 */

#include "reset_define_includes.h"
#define STDIOLIB
#define STDLIB
#define ASSERTLIB
#include "logics_struct.h"


/**
 * CTL like 
 **/

/**
 * Expressions
 */

//Functions declarations
char * logic_bool_op_to_string(BoolLogicsOperators op){
    char *str = (char *) malloc(64*sizeof(char));
    switch (op){
        case L_NOT:
            strcpy(str, "!");
            break;
        case L_AND:
            strcpy(str, "and");
            break;
        case L_OR:
            strcpy(str, "or");
            break;
        case L_EQ:
            strcpy(str, "=");
            break;
        case L_SMALLER:
            strcpy(str, "<");
            break;
        case L_GREATER:
            strcpy(str, ">");
            break;
        case L_SMALLER_EQ:
            strcpy(str, "<=");
            break;
        case L_GREATER_EQ:
            strcpy(str, "=>");
            break;
    }
    return str;
}

char * logic_math_op_to_string(MathLogicsOperators op){
    char *str = (char *) malloc(64*sizeof(char));
    switch (op){
        case L_PROD:
            strcpy(str, "*");
            break;
        case L_PLUS:
            strcpy(str, "+");
            break;
    }
    return str;
}

Expression * logic_create_property_expression(int var, SourceProperty source){
    //Create An argExpression for a place
    errno=0;
    Expression *exp = NULL;
    exp = (Expression *) malloc(sizeof(Expression));
    if(!exp || errno){
        ERRORMACRO("logic_create_property_expression: Impossible to create new Expression.\n");
    }
    exp->type = L_PROPERTY;
    exp->get.property.source = source;
    exp->get.property.value = var;
    return exp;
}

Expression * logic_create_natural_expression(int nat){
    //Create An argExpression for a place
    errno=0;
    Expression *exp = NULL;
    exp = (Expression *) malloc(sizeof(Expression));
    if(!exp || errno){
        ERRORMACRO("logic_create_natural_expression: Impossible to create new Expression.\n");
    }
    exp->type = L_NATURAL;
    exp->get.natural = nat;
    return exp;
}

Expression * logic_create_unary_expression(Expression *arg){
    assert(arg);
    //Create An argExpression for an expression -  nested expressions
    errno=0;
    Expression *exp = NULL;
    exp = (Expression *) malloc(sizeof(Expression));
    if(!exp || errno){
        ERRORMACRO("logic_create_unary_expression: Impossible to create new Expression.\n");
    }
    exp->type = L_UNARY_EXPRESSION;
    exp->get.unary_exp = arg;
    return exp;
}

Expression * logic_create_math_expression(Expression *arg1,
        Expression *arg2, MathLogicsOperators op){
    assert(arg1 && arg2);
    //Create a math expression: arg1 op arg2
    errno=0;
    Expression *exp = NULL;
    exp = (Expression *) malloc(sizeof(Expression));
    if(!exp || errno){
        ERRORMACRO("logic_create_math_expression: Impossible to create new Expression .\n");
    }
    exp->type = L_BINARY_EXPRESSION;
    exp->get.binary_exp.type = L_MATH;
    exp->get.binary_exp.op.math_op = op;
    exp->get.binary_exp.arg1 = arg1;
    exp->get.binary_exp.arg2 = arg2;
    return exp;
}

Expression * logic_create_bool_expression(Expression *arg1,
        Expression *arg2, BoolLogicsOperators op){
    assert(arg2 || (arg1 && arg2));
    //Create a bool expression: arg1 op arg2
    errno=0;
    Expression *exp = NULL;
    exp = (Expression *) malloc(sizeof(Expression));
    if(!exp || errno){
        ERRORMACRO("logic_create_bool_expression: Impossible to create new Expression .\n");
    }
    exp->type = L_BINARY_EXPRESSION;
    exp->get.binary_exp.type = L_BOOL;
    exp->get.binary_exp.op.bool_op = op;
    exp->get.binary_exp.arg1 = arg1;
    exp->get.binary_exp.arg2 = arg2;
    return exp;
}

//Print functions for debug
void _logic_print_binary_expression(Expression *arg);

void logic_print_expression(Expression *arg){
    assert(arg);
    switch (arg->type){
        case L_PROPERTY:
            switch (arg->get.property.source){
                case L_NET:
                    fprintf(stdout, " place[%d] ", arg->get.property.value);
                    break;
                case L_DATA:
                    fprintf(stdout, " data[%d] ", arg->get.property.value);
                    break;
                case L_DEAD:
                    fprintf(stdout, " dead ");
                    break;
                case L_TIME:
                    ERRORMACRO(" Time is not supported\n")
                    break;

            }
            break;
        case L_NATURAL:
            fprintf(stdout, " %d ", arg->get.natural);
            break;
        case L_BINARY_EXPRESSION:
            _logic_print_binary_expression(arg);
            break;
        case L_UNARY_EXPRESSION:
            logic_print_expression(arg->get.binary_exp.arg1);
            break;
    }
}

void _logic_print_binary_expression(Expression* arg){
    char *op = NULL;
    int print_first = 1;
    switch (arg->get.binary_exp.type){
        case L_MATH:
            op = logic_math_op_to_string(arg->get.binary_exp.op.math_op);
            break;
        case L_BOOL:
            if(arg->get.binary_exp.op.bool_op==L_NOT)
                print_first = 0;
            op = logic_bool_op_to_string(arg->get.binary_exp.op.bool_op);
            break;
    }
    fprintf(stdout, "(");
    if(print_first)
        logic_print_expression(arg->get.binary_exp.arg1);
    fprintf(stdout, " %s ", op);
    logic_print_expression(arg->get.binary_exp.arg2);
    fprintf(stdout, ")");
    //Release op string
    free(op);
}

//---------------------------------------------------------------------------//
/**
 * Formulas
 */

char * logic_binary_op_to_string(BinarieOperator op){
    char *str = (char *) malloc(64*sizeof(char));
    switch (op){
        case L_UNTIL:
            strcpy(str, "U");
            break;
        case L_LEADSTO:
            strcpy(str, "=>");
            break;
    }
    return str;
}

char * logic_unary_op_to_string(UnarieOperator op){
    char *str = (char *) malloc(64*sizeof(char));
    switch (op){
        case L_GLOBAL:
            strcpy(str, "A");
            break;
        case L_EXISTENTIAL:
            strcpy(str, "E");
            break;
        case L_NEXT:
            strcpy(str, "()");
            break;
        case L_NEGATION:
            strcpy(str, "!");
            break;
    }
    return str;
}

Formula * logic_create_formula_expression(Expression *exp){
    assert(exp);
    errno=0;
    Formula *arg = NULL;
    arg = (Formula *) malloc(sizeof(Formula));
    if(!arg || errno){
        ERRORMACRO("logic_create_arg_expression: Impossible to create new Formula.\n");
    }
    arg->type = L_EXPRESSION;
    arg->get.exp = exp;
    return arg;
}

Formula * logic_create_formula_binary(Formula *arg1, Formula *arg2,
        BinarieOperator op){
    assert(arg2);
    errno=0;
    Formula *formula = NULL;
    formula = (Formula *) malloc(sizeof(Formula));
    if(!formula || errno){
        ERRORMACRO("logic_create_formula_binarie: Impossible to create new Formula.\n");
    }
    formula->type = L_BINARY;
    formula->get.binary.logic = op;
    formula->get.binary.arg1 = arg1;
    formula->get.binary.arg2 = arg2;
    return formula;
}

Formula * _logic_check_for_double_negation(Formula *arg){
    if(arg->type == L_UNARY && arg->get.unary.logic == L_NEGATION){
        return arg->get.unary.arg;
    }
    else
        return NULL;
}

Formula * logic_create_formula_unary(Formula *arg,
        UnarieOperator op){
    assert(arg);
    errno=0;
    Formula *formula = NULL;
    formula = (Formula *) malloc(sizeof(Formula));
    if(!formula || errno){
        ERRORMACRO("logic_create_formula_binarie: Impossible to create new Formula.\n");
    }
    //Check for double negation -- Remove double negation
    Formula * f= _logic_check_for_double_negation(arg);
    if(f)
        return f;
    formula->type = L_UNARY;
    formula->get.unary.logic = op;
    formula->get.unary.arg = arg;
    return formula;
}

//Print functions for debug
void _logic_print_binary_formula(Formula *formula);
void _logic_print_unary_formula(Formula *formula);

void logic_print_formula(Formula *formula){
    assert(formula);
    switch (formula->type){
        case L_EXPRESSION:
            logic_print_expression(formula->get.exp);
            break;
        case L_BINARY:
            _logic_print_binary_formula(formula);
            break;
        case L_UNARY:
            _logic_print_unary_formula(formula);
            break;
    }
}


void _logic_print_binary_formula(Formula *formula){
    assert(formula);
    char *op = logic_binary_op_to_string(formula->get.binary.logic);
    Formula *arg1, *arg2;
    arg1 = formula->get.binary.arg1;
    arg2 = formula->get.binary.arg2;
    //Print Formula
    fprintf(stdout, "(");
    if(arg1)
        logic_print_formula(arg1);
    else
        fprintf(stdout, " TRUE ");
    fprintf(stdout, " %s ", op);
    logic_print_formula(arg2);
    fprintf(stdout, ")");
    //Release string
    free(op);
}

void _logic_print_unary_formula(Formula *formula){
    assert(formula);
    char *op = logic_unary_op_to_string(formula->get.unary.logic);
    Formula *arg;
    arg = formula->get.unary.arg;
    fprintf(stdout, " %s ", op);
    logic_print_formula(arg);
    //Release string
    free(op);
}

