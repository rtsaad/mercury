/**
 * @file        logic_struct.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on February 26, 2011, 4:09 PM
 *
 * @section LICENSE
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
 * @section DESCRIPTION
 *
 * This file builds the abstract view of the CTL formula parsed by LEX/YACC.
 * It is used by YACC to construct the abstract tree.
 *
 */


#ifndef _LOGICS_STRUCT_H
#define	_LOGICS_STRUCT_H

#include "flags.h"

#include "standard_includes.h"

//#include "reachgraph_parallel.h"

/**
 * CTL like logics definition
 **/

/**
 * Expressions
 */

typedef enum BoolOperatorsEnum {L_AND, L_OR, L_NOT, L_EQ,
    L_SMALLER, L_GREATER, L_SMALLER_EQ, L_GREATER_EQ} BoolLogicsOperators;

typedef enum MathOperatorsEnum {L_PROD, L_PLUS} MathLogicsOperators;

typedef enum TypeExpressionEnum {L_BOOL, L_MATH} TypeExpression;

typedef enum SourcePropertyEnum{L_DEAD, L_NET, L_DATA, L_TIME}SourceProperty;

typedef enum TypeStructEnum {L_PROPERTY, L_BINARY_EXPRESSION,
    L_UNARY_EXPRESSION, L_NATURAL} TypeStruct;

    typedef struct ExpressionStruct{
    TypeStruct type;
    union{
        //Expression
        struct {
            TypeExpression type;
            union {
                MathLogicsOperators math_op;
                BoolLogicsOperators bool_op;
            }op;
            struct ExpressionStruct * arg1;
            struct ExpressionStruct * arg2;
        }binary_exp;
        struct ExpressionStruct * unary_exp;
        //Property
        struct {
            SourceProperty source;
            int value;
        }property;
        int natural;
    }get;
}Expression;

//Functions Interface

/*
 * Translate the given operator to a string
 * @param op a boolean operator from BoolLogicsOperators enum
 */
extern char * logic_bool_op_to_string(BoolLogicsOperators op);

/*
 * Translate the given operator to a string
 * @param op a math operator from MathLogicsOperators enum
 */
extern char * logic_math_op_to_string(MathLogicsOperators op);

/*
 * Create an Expression structure for a model variable (integer)
 * @param var An interger from the place (or var net data) vector
 * @return ArgExpression structure
 */
extern Expression * logic_create_property_expression(int var, SourceProperty source);

/*
 * Create an Expression structure for a natural number
 * @param nat Natural number
 * @return ArgExpression structure
 */
extern Expression * logic_create_natural_expression(int nat);

/*
 * Create an Expression structure for an expression. It is usefull for nested
 * expressions.
 * @param expression A valid Expression reference
 * @return ArgExpression structure
 */
extern Expression * logic_create_unary_expression(Expression *exp);

/*
 * Create a math expression.
 * @param arg1 An ArgExpression Structure
 * @param arg2 An ArgExpression Structure
 * @param op A math operator from MathLogicsOperators enum
 * @return An Expression structure
 */
extern Expression * logic_create_math_expression(Expression *arg1,
        Expression *arg2, MathLogicsOperators op);

/*
 * Create a boolean expression.
 * @param arg1 An ArgExpression Structure
 * @param arg2 An ArgExpression Structure
 * @param op A bool operator from BoolLogicsOperators enum
 * @return An Expression structure
 */
extern Expression * logic_create_bool_expression(Expression *arg1,
        Expression *arg2, BoolLogicsOperators op);

/*
 * Print an expression.
 * @param arg An ArgExpression Structure
 */
extern void logic_print_expression(Expression *exp);

//---------------------------------------------------------------------------//
/**
 * Formulas
 */

//typedef enum BranchOperatorEnum {L_GLOBAL, L_EXISTENTIAL} BranchOperator;
//typedef enum LinearOperatorEnum {L_NEXT, L_UNTIL} LinearOperator;

typedef enum BinarieOperatorEnum {L_UNTIL, L_LEADSTO} BinarieOperator;
typedef enum UnarieOperatorEnum {L_GLOBAL, L_EXISTENTIAL, L_NEXT, L_NEGATION} UnarieOperator;
typedef enum TypeFormulaEnum {L_UNARY, L_BINARY, L_EXPRESSION} TypeFormula;

typedef struct FormulaStruct{
    TypeFormula type;
    union{
        struct{
            BinarieOperator logic;
            struct FormulaStruct * arg1;
            struct FormulaStruct * arg2;
        }binary;
        struct{
            UnarieOperator logic;
            struct FormulaStruct * arg;
        }unary;
        Expression *exp;
    }get;
} Formula;

//Functions Interface

/*
 * Translate the given operator to a string
 * @param op a binarie operator from BinarieOperator enum
 */
extern char * logic_binary_op_to_string(BinarieOperator op);

/*
 * Translate the given operator to a string
 * @param op a unarie operator from UnarieOperator enum
 */
extern char * logic_unary_op_to_string(UnarieOperator op);

/*
 * Create a Formula structure for an expression.
 * @param expression A valid Expression reference
 * @return Logic structure
 */
extern Formula * logic_create_formula_expression(Expression *exp);

/*
 * Create a binary formula.
 * @param arg1 A Formula Structure
 * @param arg2 A Formula Structure
 * @param op A logic operator from BinarieOperator enum
 * @return A Formula structure
 */
extern Formula * logic_create_formula_binary(Formula *arg1, Formula *arg2,
        BinarieOperator op);

/*
 * Create a unarie formula.
 * @param arg A Logic Structure
 * @param op A logic operator from UnarieOperator enum
 * @return A Formula structure
 */
extern Formula * logic_create_formula_unary(Formula *arg,
        UnarieOperator op);

/*
 * Print the given formula.
 * @param formula A Formula structure
 */
extern void logic_print_formula(Formula *formula);

#endif	/* _LOGICS_STRUCT_H */

