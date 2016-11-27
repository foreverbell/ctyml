#include "evaluator.h"

#include <memory>
#include <vector>

#include "error.h"
#include "context.h"

using std::unique_ptr;

unique_ptr<Term> TermMapper::Map(const Term* term) {
  term->Accept(this);
  unique_ptr<Term> ret = get(term);
  result_.clear();
  return ret;
}

void TermMapper::Visit(const NullaryTerm* term) {
  result_[term] = std::make_unique<NullaryTerm>(term->location(), term->type());
}

void TermMapper::Visit(const UnaryTerm* term) {
  term->term()->Accept(this);

  result_[term] = std::make_unique<UnaryTerm>(term->location(), term->type(), get(term->term()).release());
}

void TermMapper::Visit(const BinaryTerm* term) {
  term->term1()->Accept(this);
  term->term2()->Accept(this);

  result_[term] = std::make_unique<BinaryTerm>(term->location(), term->type(), get(term->term1()).release(),
                                               get(term->term2()).release());
}

void TermMapper::Visit(const TernaryTerm* term) {
  term->term1()->Accept(this);
  term->term2()->Accept(this);
  term->term3()->Accept(this);

  result_[term] = std::make_unique<TernaryTerm>(term->location(), term->type(), get(term->term1()).release(),
                                                get(term->term2()).release(), get(term->term3()).release());
}

void TermMapper::Visit(const NilTerm* term) {
  result_[term] = std::make_unique<NilTerm>(term->location(), term->list_type()->clone());
}

void TermMapper::Visit(const VariableTerm* term) {
  result_[term] = VariableMap(term->location(), term->index());
}

void TermMapper::Visit(const RecordTerm* term) {
  auto record_term = std::make_unique<RecordTerm>(term->location());

  for (size_t i = 0; i < term->size(); ++i) {
    term->get(i).second->Accept(this);
    record_term->add(term->get(i).first, get(term->get(i).second).release());
  }
  result_[term] = std::move(record_term);
}

void TermMapper::Visit(const ProjectTerm* term) {
  term->term()->Accept(this);
  result_[term] = std::make_unique<ProjectTerm>(term->location(), get(term->term()).release(), term->field());
}

void TermMapper::Visit(const LetTerm* term) {
  term->bind_term()->Accept(this);
  ++depth_; term->body_term()->Accept(this); --depth_;

  result_[term] = std::make_unique<LetTerm>(term->location(), term->variable(),
                                            get(term->bind_term()).release(), get(term->body_term()).release());
}

void TermMapper::Visit(const AbsTerm* term) {
  ++depth_; term->term()->Accept(this); --depth_;

  result_[term] = std::make_unique<AbsTerm>(term->location(), term->variable(), term->variable_type()->clone(),
                                            get(term->term()).release());
}

void TermMapper::Visit(const AscribeTerm* term) {
  term->term()->Accept(this);

  result_[term] = std::make_unique<AscribeTerm>(term->location(), get(term->term()).release(),
                                                term->ascribe_type()->clone());
}

unique_ptr<Term> TermShifter::VariableMap(Location location, int var) {
  // <var> >= <depth> means this variable is a free variable.
  return std::make_unique<VariableTerm>(location, var >= depth() ? var + delta_ : var);
}

unique_ptr<Term> TermSubstituter::VariableMap(Location location, int var) {
  if (var == depth()) {
    TermShifter shifter(depth());
    unique_ptr<Term> ret = shifter.TermShift(substitute_to_);
    ret->relocate(location);
    return ret;
  }
  return std::make_unique<VariableTerm>(location, var);
}

namespace {

template <typename T>
T* term_cast(Term* ptr) { return dynamic_cast<T*>(ptr); }

}  // namespace

// This line should never be executed unless there is a bug in typechecker.
#define DieGuardedByTypeChecker() assert(false && "death guarded by type-checker")

unique_ptr<Term> TermEvaluator::Evaluate(const Term* term) {
  term->Accept(this);
  unique_ptr<Term> ret = eval(term);
  result_.clear();
  return ret;
}

void TermEvaluator::Visit(const NullaryTerm* term) {
  result_[term] = std::unique_ptr<Term>(term->clone());
}

void TermEvaluator::Visit(const UnaryTerm* term) {
  term->term()->Accept(this);
  unique_ptr<Term> subterm = eval(term->term());

  switch (term->type()) {
    case UnaryTermToken::Pred: {
      NullaryTerm* const zero_term = term_cast<NullaryTerm>(subterm.get());
      UnaryTerm* const succ_term = term_cast<UnaryTerm>(subterm.get());
      if (zero_term != nullptr && zero_term->type() == NullaryTermToken::Zero) {
        result_[term] = std::make_unique<NullaryTerm>(term->location(), NullaryTermToken::Zero);
      } else if (succ_term != nullptr && succ_term->type() == UnaryTermToken::Succ) {
        result_[term] = std::move(succ_term->term());
      } else {
        DieGuardedByTypeChecker();
      }
    } break;
    case UnaryTermToken::Succ: {
      result_[term] = std::make_unique<UnaryTerm>(term->location(), UnaryTermToken::Succ, subterm.release());
    } break;
    case UnaryTermToken::IsZero: {
      NullaryTerm* const zero_term = term_cast<NullaryTerm>(subterm.get());
      UnaryTerm* const succ_term = term_cast<UnaryTerm>(subterm.get());
      if (zero_term != nullptr && zero_term->type() == NullaryTermToken::Zero) {
        result_[term] = std::make_unique<NullaryTerm>(term->location(), NullaryTermToken::True);
      } else if (succ_term != nullptr && succ_term->type() == UnaryTermToken::Succ) {
        result_[term] = std::make_unique<NullaryTerm>(term->location(), NullaryTermToken::False);
      } else {
        DieGuardedByTypeChecker();
      }
    } break;
    case UnaryTermToken::Head: {
      NilTerm* const nil_term = term_cast<NilTerm>(subterm.get());
      BinaryTerm* const cons_term = term_cast<BinaryTerm>(subterm.get());
      if (nil_term != nullptr) {
        throw runtime_exception(term->location(), "<head> on an empty list");
      } else if (cons_term != nullptr && cons_term->type() == BinaryTermToken::Cons) {
        result_[term] = std::move(cons_term->term1());
      } else {
        DieGuardedByTypeChecker();
      }
    } break;
    case UnaryTermToken::Tail: {
      NilTerm* const nil_term = term_cast<NilTerm>(subterm.get());
      BinaryTerm* const cons_term = term_cast<BinaryTerm>(subterm.get());
      if (nil_term != nullptr) {
        throw runtime_exception(term->location(), "<tail> on an empty list");
      } else if (cons_term != nullptr && cons_term->type() == BinaryTermToken::Cons) {
        result_[term] = std::move(cons_term->term2());
      } else {
        DieGuardedByTypeChecker();
      }
    } break;
    case UnaryTermToken::IsNil: {
      NilTerm* const nil_term = term_cast<NilTerm>(subterm.get());
      result_[term] = std::make_unique<NullaryTerm>(term->location(),
                                                    nil_term ? NullaryTermToken::True : NullaryTermToken::False);
    } break;
    case UnaryTermToken::Fix: {
      AbsTerm* const abs_term = term_cast<AbsTerm>(subterm.get());
      if (abs_term != nullptr) {
        result_[term] = Substitute(abs_term->term().get(), abs_term);
      } else {
        DieGuardedByTypeChecker();
      }
    } break;
  }
}

void TermEvaluator::Visit(const BinaryTerm* term) {
  term->term1()->Accept(this);
  term->term2()->Accept(this);
  unique_ptr<Term> subterm1 = eval(term->term1());
  unique_ptr<Term> subterm2 = eval(term->term2());

  switch (term->type()) {
    case BinaryTermToken::Cons: {
      result_[term] = std::make_unique<BinaryTerm>(term->location(), BinaryTermToken::Cons,
                                                   subterm1.release(), subterm2.release());
    } break;
    case BinaryTermToken::App: {
      AbsTerm* const abs_term = term_cast<AbsTerm>(subterm1.get());
      if (abs_term != nullptr) {
        result_[term] = Substitute(abs_term->term().get(), subterm2.get());
      } else {
        DieGuardedByTypeChecker();
      }
    } break;
  }
}

void TermEvaluator::Visit(const TernaryTerm* term) {
  switch (term->type()) {
    case TernaryTermToken::If: {
      term->term1()->Accept(this);
      unique_ptr<Term> predicate = eval(term->term1());
      NullaryTerm* const bool_term = term_cast<NullaryTerm>(predicate.get());

      if (bool_term != nullptr && bool_term->type() == NullaryTermToken::True) {
        term->term2()->Accept(this);
        result_[term] = eval(term->term2());
      } else if (bool_term != nullptr && bool_term->type() == NullaryTermToken::False) {
        term->term3()->Accept(this);
        result_[term] = eval(term->term3());
      } else {
        DieGuardedByTypeChecker();
      }
    } break;
  }
}

void TermEvaluator::Visit(const NilTerm* term) {
  result_[term] = std::make_unique<NilTerm>(term->location(), term->list_type()->clone());
}

void TermEvaluator::Visit(const VariableTerm* term) {
  result_[term] = TermShifter(term->index() + 1).TermShift(ctx_->get(term->index()).second->term());
}

void TermEvaluator::Visit(const RecordTerm* term) {
  unique_ptr<RecordTerm> record_term;

  for (size_t i = 0; i < term->size(); ++i) {
    term->get(i).second->Accept(this);
    record_term->add(term->get(i).first, eval(term->get(i).second).release());
  }
  result_[term] = std::move(record_term);
}

void TermEvaluator::Visit(const ProjectTerm* term) {
  term->term()->Accept(this);
  unique_ptr<Term> subterm = eval(term->term());

  RecordTerm* const record_term = term_cast<RecordTerm>(subterm.get());
  if (record_term != nullptr) {
    for (size_t i = 0; i < record_term->size(); ++i) {
      if (record_term->get(i).first == term->field()) {
        result_[term] = std::move(record_term->get(i).second);
        return;
      }
    }
    DieGuardedByTypeChecker();
  }
  result_[term] = std::make_unique<ProjectTerm>(term->location(), subterm.release(), term->field());
}

void TermEvaluator::Visit(const LetTerm* term) {
  term->bind_term()->Accept(this);
  result_[term] = Substitute(term->body_term().get(), eval(term->bind_term()).release());
}

void TermEvaluator::Visit(const AbsTerm* term) {
  result_[term] = unique_ptr<Term>(term->clone());
}

void TermEvaluator::Visit(const AscribeTerm* term) {
  term->term()->Accept(this);
  result_[term] = eval(term->term());
}

unique_ptr<Term> TermEvaluator::Substitute(const Term* term, const Term* to) {
  unique_ptr<Term> up = TermShifter(1).TermShift(to);
  TermSubstituter substitutor(up.get());
  // Shift down by 1 so we can kill the variable 0.
  unique_ptr<Term> down = TermShifter(-1).TermShift(substitutor.TermSubstitute(term).get());

  return TermEvaluator(ctx_).Evaluate(down.get());
}
