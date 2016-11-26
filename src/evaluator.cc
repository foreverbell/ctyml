#include "evaluator.h"

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
  
  result_[term] = std::make_unique<LetTerm>(term->location(), 
                                            new Pattern(term->pattern()->location(), term->pattern()->variable()),
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

// TermEvaluator.

void TermEvaluator::Visit(const NullaryTerm* term) {
	
}

void TermEvaluator::Visit(const UnaryTerm* term) {

}

void TermEvaluator::Visit(const BinaryTerm* term) {

}

void TermEvaluator::Visit(const TernaryTerm* term) {

}

void TermEvaluator::Visit(const NilTerm* term) {

}

void TermEvaluator::Visit(const VariableTerm* term) {

}

void TermEvaluator::Visit(const RecordTerm* term) {

}

void TermEvaluator::Visit(const ProjectTerm* term) {

}

void TermEvaluator::Visit(const LetTerm* term) {

}

void TermEvaluator::Visit(const AbsTerm* term) {

}

void TermEvaluator::Visit(const AscribeTerm* term) {

}
