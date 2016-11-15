#pragma once

#include <memory>
#include <vector>

#include "token.h"

// Context free grammar for our toy PL.
//
// Topmost = Statement ';'
//         | Statement ';' Topmost
//
// Statement = Term
//           | 'type' ucid '=' Type
//           | 'let' LetBinder
//           | 'letrec' LetrecBinder
//
// Term = AppTerm
//      | 'lambda' TypedBinders '.' Term
//      | 'if' Term 'then' Term 'else' Term
//      | 'let' LetBinder 'in' Term
//      | 'letrec' LetrecBinder 'in' Term
//
// TypedBinders = TypedBinder
//              | TypedBinder TypedBinders
//
// TypedBinder = lcid ':' Type
// 	       | '_' ':' Type
//
// Pattern = lcid
//         | '_'
//         | '{' FieldPatterns '}'
//
// FieldPatterns = FieldPattern
//               | FieldPattern ',' FieldPatterns
//
// FieldPattern = Pattern '=' lcid
//
// LetBinder = Pattern '=' Term
//
// LetrecBinder = TypedBinder '=' Term
//
// AppTerm = PathTerm
//         | AppTerm PathTerm
//         | 'succ' PathTerm
//         | 'pred' PathTerm
//         | 'iszero' PathTerm
//         | 'cons' PathTerm PathTerm
//         | 'isnil' PathTerm
//         | 'head' PathTerm
//         | 'tail' PathTerm
//
// PathTerm = PathTerm '.' lcid
//          | AscribeTerm
//
// AscribeTerm = AtomicTerm
//             | AtomicTerm 'as' Type
//
// AtomicTerm = '(' Term ')'
//            | 'true'
//            | 'false'
//            | int
//            | 'nil' '[' Type ']'
//            | 'unit'
//            | '{' Fields '}'
//            | lcid
//
// Fields = Field
//        | Field ',' Fields
//
// Field = lcid '=' Term
//
// Type = ArrowType
//
// ArrowType = AtomicType '->' ArrowType
//           | AtomicType
//
// AtomicType = '(' Type ')'
//            | 'Bool'
//            | 'Nat'
//            | 'List' '[' Type ']'
//            | 'Unit'
//            | '{' FieldTypes '}'
//            | ucid
//
// FieldTypes = FieldType
//            | FieldType ',' FieldTypes
//
// FieldType = lcid ':' Type

bool ParseToken(const std::vector<std::unique_ptr<Token>>& tokens);
