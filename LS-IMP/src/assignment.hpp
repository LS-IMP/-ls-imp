#pragma once

#include "utils.hpp"
#include "formula.hpp"

namespace LS_NIA {

class Value {
    friend ostream & operator << (ostream & out, const Value& value);
protected:
    Int _lb, _ub;           // ??? need Float ?
    Int _val;
public:
    Value() { _val = _lb = DUMMY_MIN_INT; _val = _ub = DUMMY_MAX_INT; }
    // explicit Value(Float lb, Float ub) { _lb = lb; _ub = ub;}
    Value & operator = (Value v) { _val = v._val, _lb = v._lb; _ub = v._ub; return *this;}
    Value & operator = (Int val) { _val = val; return *this;}
    bool operator == (const Value &other) const { return _val == other._val && _lb == other._lb && _ub == other._ub;}
    bool operator != (const Value &other) const { return _val != other._val || _lb != other._lb || _ub != other._ub;}
    bool operator == (const Int value) const { return _val == value; }      // not judge _lb _ub
    bool operator != (const Int value) const { return _val != value; }      // not judge _lb _ub

    inline Int  getLB() const { return _lb; }
    inline Int  getUB() const { return _ub; }
    inline void setLB(Int lb) { _lb = lb;}
    inline void setUB(Int ub) { _ub = ub;}
    inline void updateLB(Int lb) { _lb = std::max(_lb, lb); }
    inline void updateUB(Int ub) { _ub = std::min(_ub, ub); }
    inline void getUnion(Value v) { getUnion(v._lb, v._ub); }
    inline void getUnion(Int lb, Int ub) { _lb = std::max(_lb, lb); _ub = std::min(_ub, ub); }

    inline bool isValid() const { return _lb <= _ub && isValid(_val); }
    inline bool isValid(Int val) const { return _lb <= val && val <= _ub; }

    inline Int  getAssign() const { return _val; }
    // inline bool isUndef() const { return _val == DUMMY_MIN_INT;} // not exists paritial assignment
    const static Int undef;
};

class Assignment {
    friend ostream & operator << (ostream & out, const Assignment& assignment);
protected:
    Variable _varCnt;
    // Value * _assignment;
    Int* _val;
    Int* _lb;
    Int* _ub;
public:
    Variable varCnt() { return _varCnt;}
    Assignment() : _varCnt(Variable::undef) {}
    ~Assignment() { if (_varCnt != Variable::undef) freeAuxiliaryMemory(); }
    void allocateAuxiliaryMemory(Variable maxVar);
	// void freeAuxiliaryMemory() { delete [] _assignment; }
	void freeAuxiliaryMemory() { delete [] _val; delete [] _lb; delete [] _ub; }
    void operator = (const Assignment& assignment);
    
    // Value& at(Variable var);
    // const Value& find(Variable var) const;

    inline Int& valAt(Variable var) { return _val[var]; }
    inline Int& lbAt(Variable var) { return _lb[var]; }
    inline Int& ubAt(Variable var) { return _ub[var]; }
    inline Int& getVal(Variable var) const { return _val[var]; }
    inline Int& getLB(Variable var) const { return _lb[var]; }
    inline Int& getUB(Variable var) const { return _ub[var]; }
    inline bool isValid(Variable var, Int val) const { return val >= _lb[var] && val <= _ub[var]; }
};

// struct Operator {
//     Variable _var;
//     Int      _val;
// }; 

class OperatorPool {
protected:
    vector<Variable> _vars;
    vector<Int>      _vals;
    Int              _size;

    Map<unsigned, Set<Int> >_operatorSet;

public:
    OperatorPool() { _size = 0; }
    void clear();
    Int  size();
    void push(Variable var, Int val);
    bool empty() const;

    bool haveOperator(const Variable& var, const Int& val);
    void addOperator(const Variable& var, const Int& val);

    Variable varAt(Int index) const;
    Int valAt(Int index) const;

    void removeOpAt(Int index);

    void printOperators() const;
};

} // namespace LS_NIA
