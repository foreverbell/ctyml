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

bool Interpret(const string& filename, const string& input) {
  unique_ptr<Lexer> lexer;
  Locator locator(filename, input);

  try {
    lexer = unique_ptr<Lexer>(Lexer::Create(input));
  } catch (const lexical_exception& e) {
    locator.Error(2, e.location(), e.what());
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
    locator.Error(2, e.location(), e.what());
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
        ctx.AddBinding(term_stmt->variable(), new Binding(term.release(), type.release()));
      } else if (type_stmt != nullptr) {
        type = unique_ptr<TermType>(type_stmt->type()->clone());
        ctx.AddBinding(type_stmt->type_alias(), new Binding(nullptr, type.release()));
      }
    } catch (const type_exception& e) {
      locator.Error(2, e.location(), e.what());
      return false;
    } catch (const runtime_exception& e) {
      locator.Error(2, e.location(), e.what());
      return false;
    }
  }
  return true;
}

void Dispatch(const string& input, bool* multi_line_stmts) {
  if (input == ":dumpctx") {
    if (ctx.size() == 0) {
      puts("empty context.");
    } else {
      PrettyPrinter pprinter(&ctx);
      for (size_t i = 0; i < ctx.size(); ++i) {
        const string& bind = ctx.get(i).first;
        const Term* term = ctx.get(i).second->term();
        const TermType* type = ctx.get(i).second->type();

        assert(type != nullptr);
        if (term == nullptr) {
          printf("%s = %s\n", bind.c_str(), pprinter.PrettyPrint(type).c_str());
        } else {
          printf("%s = %s : %s\n", bind.c_str(), pprinter.PrettyPrint(term).c_str(), pprinter.PrettyPrint(type).c_str());
        }
      }
    }
  } else if (input == ":{") {
    if (*multi_line_stmts) {
      puts("already in multi-line statement mode, skipped.");
    }
    *multi_line_stmts = true;
  } else if (input == ":}") {
    if (!*multi_line_stmts) {
      puts("not in multi-line statement mode, ignored.");
    }
    *multi_line_stmts = false;
  } else {
    printf("unknown command %s.\n", input.c_str());
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    usage(argc, argv);
  }

  if (strcmp(argv[1], "-i") == 0) {
    bool multi_line_stmts = false;
    string stmts;

    while (true) {
      string input;

      printf("ctyml> ");
      if (!std::getline(std::cin, input)) break;

      if (input != ":}" && multi_line_stmts) {
        stmts += input;
        stmts += "\n";
        continue;
      }

      if (input.empty()) continue;

      if (input[0] == ':') {
        Dispatch(input, &multi_line_stmts);

        if (!multi_line_stmts && !stmts.empty()) {
          Interpret("(file)", stmts);
          stmts.clear();
        }
      } else {
        Interpret("(file)", input);
      }
    }
  } else {
    ifstream fin(argv[1]);
    string input;

    fin.seekg(0, std::ios::end);
    input.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);
    input.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

    Interpret(argv[1], input);
  }
  return 0;
}
