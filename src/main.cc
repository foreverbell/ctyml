#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ast.h"
#include "context.h"
#include "error.h"
#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include "pprinter.h"
#include "type-checker.h"

using std::ifstream;
using std::string;
using std::unique_ptr;
using std::vector;

void usage(int argc, char** argv) {
  printf("usage: %s [-i | file]\n", argv[0]);
  puts("\n"
       "options:\n"
       "  -i      interactive mode\n");
  exit(0);
}

Context ctx;
vector<unique_ptr<Term>> evaluated_terms;
vector<unique_ptr<TermType>> defined_types;

bool Interpret(const string& input) {
  std::unique_ptr<Lexer> lexer(Lexer::Create(input));

  if (lexer == nullptr) {
    fprintf(stderr, "lexical error\n");
    return false;
  }

  Parser parser(lexer.get());
  vector<unique_ptr<Stmt>> stmts;
  PrettyPrinter pprinter(&ctx);
  TypeChecker type_checker(&ctx);
  TermEvaluator evaluator(&ctx);

  try {
    stmts = parser.ParseAST(&ctx);
  } catch (const ast_exception& e) {
    lexer->locator()->Error(2, e.location(), e.what());
    return false;
  }

  for (size_t i = 0; i < stmts.size(); ++i) {
    EvalStmt* eval_stmt = dynamic_cast<EvalStmt*>(stmts[i].get());
    BindTermStmt* term_stmt = dynamic_cast<BindTermStmt*>(stmts[i].get());
    BindTypeStmt* type_stmt = dynamic_cast<BindTypeStmt*>(stmts[i].get());

    unique_ptr<TermType> type;
    unique_ptr<Term> term;

    try {
      if (eval_stmt != nullptr) {
        type = type_checker.TypeCheck(eval_stmt->term().get());
        term = evaluator.Evaluate(eval_stmt->term().get());
        printf("%s\n", pprinter.PrettyPrint(term.get()).c_str());
      } else if (term_stmt != nullptr) {
        type = type_checker.TypeCheck(term_stmt->term().get());
        term = evaluator.Evaluate(term_stmt->term().get());
        ctx.AddBinding(term_stmt->variable(), new Binding(term.get(), type.get()));
      } else if (type_stmt != nullptr) {
        type = unique_ptr<TermType>(type_stmt->type()->clone());
        ctx.AddBinding(type_stmt->type_alias(), new Binding(nullptr, type.get()));
      }
    } catch (const type_exception& e) {
      lexer->locator()->Error(2, e.location(), e.what());
      return false;
    } catch (const runtime_exception& e) {
      lexer->locator()->Error(2, e.location(), e.what());
      return false;
    }
    if (term != nullptr) {
      evaluated_terms.push_back(std::move(term));
    }
    if (type != nullptr) {
      defined_types.push_back(std::move(type));
    }
  }
  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    usage(argc, argv);
  }

  if (strcmp(argv[1], "-i") == 0) {
    while (true) {
      string input;

      printf("ctyml> ");
      if (!std::getline(std::cin, input)) {
        break;
      }
      Interpret(input);
    }
  } else {
    ifstream fin(argv[1]);
    string input;

    fin.seekg(0, std::ios::end);
    input.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);
    input.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

    Interpret(input);
  }
  return 0;
}
