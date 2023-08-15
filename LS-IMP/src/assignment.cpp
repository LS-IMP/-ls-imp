#include "assignment.hpp"

namespace LS_NIA {

const Int Value::undef = -1;

ostream & operator << (ostream & out, const Value& value) {
    out << "(" << value._val << ", [" << value._lb << ", " << value._ub << "])";
    return out;
}

ostream & operator << (ostream & out, const Assignment& assignment) {
    for (Variable var = Variable::start; var != assignment._varCnt; var++) {
        // cout << var << " = " << assignment.find(var) << endl;
        cout << var << " = " << assignment._val[var] << " [" << assignment._lb[var] << ", " << assignment._ub[var]  << "]" << endl;
    }
    return out;
}

void Assignment::operator = (const Assignment& assignment) {
    assert(_varCnt == assignment._varCnt);

    for (Int i = 0; i < _varCnt; i++) {
        // _assignment[i] = assignment._assignment[i];
        _val[i] = assignment._val[i];
        _lb[i]  = assignment._lb[i];
        _ub[i]  = assignment._ub[i];
    }
}

void Assignment::allocateAuxiliaryMemory(Variable maxVar) {
	if (_varCnt == maxVar) return;
	if (_varCnt != Variable::undef ) freeAuxiliaryMemory();
	_varCnt = maxVar;
	// _assignment = new Value [_varCnt + 2];
    _val = new Int[_varCnt + 2];
    _ub  = new Int[_varCnt + 2];
    _lb  = new Int[_varCnt + 2];
}

// Value& Assignment::at(Variable var) {
//     if (JUDGE) assert(var < _varCnt);
//     return _assignment[var];
// }

// const Value& Assignment::find(Variable var) const {
//     if (JUDGE) assert(var < _varCnt);
//     return _assignment[var];
// }

void OperatorPool::clear() {
    if (JUDGE) assert(_vars.size() == _vals.size());
    _vars.clear();
    _vals.clear();
    _operatorSet.clear();
    _size = 0;
}

Int OperatorPool::size() {
    if (JUDGE) assert(_vars.size() == _vals.size());
    return _vars.size();
}

void OperatorPool::push(Variable var, Int val) {
    if (JUDGE) assert(_vars.size() == _vals.size());
    if (JUDGE) assert(_vars.size() == _size);

    if (!haveOperator(var, val)) {
        _vars.push_back(var);
        _vals.push_back(val);

        addOperator(var, val);
        _size++;
    }
}

bool OperatorPool::empty() const {
    if (JUDGE) assert(_vars.size() == _vals.size());
    return _vars.empty();
}

bool OperatorPool::haveOperator(const Variable& var, const Int& val) {
    if (_operatorSet.count(var) != 0 && _operatorSet[var].count(val) != 0) return true;
    return false;
}

void OperatorPool::addOperator(const Variable& var, const Int& val) {
    if (JUDGE) assert(!haveOperator(var, val));

    if (_operatorSet.count(var) == 0) {
        _operatorSet.insert(std::make_pair(var, Set<Int>()));
    }
    _operatorSet[var].insert(val);
}

Variable OperatorPool::varAt(Int index) const {
    if (JUDGE) assert(index < _vars.size());
    return _vars.at(index);
}

Int OperatorPool::valAt(Int index) const {
    if (JUDGE) assert(index < _vals.size());
    return _vals.at(index);
}

void OperatorPool::removeOpAt(Int index) {
    _size--;
    _vars[index] = _vars[_size];
    _vals[index] = _vals[_size];
}

void OperatorPool::printOperators() const {
    int lineCnt = 6;
    for (Int i = 0; i < _vars.size(); i++) {
        cout << _vars.at(i) << ": " << _vals.at(i) << "\t";
        if (i != 0 && i % lineCnt == 0) cout << endl;
    }
}

}