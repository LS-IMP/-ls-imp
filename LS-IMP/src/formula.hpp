#pragma once

#include "utils.hpp"

/**
 * @brief 
 * 
 * @todo normalized Polynomial
 */

namespace LS_NIA {

class Variable {
protected:
	unsigned _val;
public:
	Variable() {}
	explicit Variable( unsigned var ) { _val = var; }
	Variable Next() const { return Variable( _val + 1 ); }
	Variable & operator ++(int) { _val++; return *this; }
	Variable & operator --(int) { _val--; return *this; }
	Variable & operator = ( unsigned var ) { _val = var; return *this; }
	Variable & operator = ( Variable var ) { _val = var; return *this; }
	bool operator == (const Variable &other) const { return _val == other._val; }
	bool operator != (const Variable &other) const { return _val != other._val; }
	bool operator == (const unsigned other) const { return _val == other; }
	bool operator != (const unsigned other) const { return _val != other; }
	operator unsigned () const { return _val; }
	// unsigned getVal() const { return _val; }

	// std::size_t operator()(const Variable& var) const {
	// 	return std::hash<unsigned>()(var.getVal());
    // }

    const static Variable start;
    const static Variable undef;
};

// used for Map and Set
// struct VariableHash {
//     std::size_t operator()(const Variable& var) const {
// 		return std::hash<unsigned>()(var.getVal());
//     }
// };

class Polynomial;
class Monomial {
	friend ostream & operator << (ostream & out, const Monomial& mono);
protected:
	vector<Variable> _vars;		// ??? multi vars? sorted
	Float 			 _coef;		// ??? need long double ? or long long
	bool 			 _normalFlag;
public:
	Monomial(){}
	Monomial(const Monomial& mono) : _vars(mono._vars), _coef(mono._coef), _normalFlag(mono._normalFlag) {}
	explicit Monomial(vector<Variable> vars, Float coef) : _vars(vars), _coef(coef), _normalFlag(false) {}
	explicit Monomial(Variable var, Float coef) : _coef(coef) { _vars.clear(); _vars.push_back(var); normalized();}

	Monomial& operator =  (const Monomial& m) { _vars = m._vars; _coef = m._coef; _normalFlag = m._normalFlag; return *this;}
	bool      operator == (const Monomial& m) const;
	Monomial& operator += (const Monomial& mono);
	Monomial& operator -= (const Monomial& mono);
	Monomial  operator -  () const;
	Monomial  operator *  (const Monomial& mono) const;

	inline bool empty() const { return _vars.size() == 0 || _coef == 0;}
	inline void clear() { _vars.clear(); _coef = NEGATIVE_INFINITY; }

	bool isContain(const Variable& var) const;
	
	const vector<Variable>& getVars() const { return _vars; }
	Float getCoef() const { return _coef; }
	void  setCoef(Float coef) { _coef = coef; }

	inline bool isLinear() const { return _coef != 0 && _vars.size() == 1; }
	inline bool isNormal() const { return _normalFlag; }
	inline void normalized() { std::sort(_vars.begin(), _vars.end()); _normalFlag = true; }		// sort _vars;
};

enum Op {LEQUAL, EQUAL, GEQUAL, UNDEF};

class Polynomial {
	friend ostream & operator << (ostream & out, const Polynomial& poly);
protected:
	// sum(mono)
	vector<Monomial> _monoVec;
	bool 			 _linearFlag;
	bool 			 _normalFlag;
public:
	Polynomial() { _normalFlag = false; }
	Polynomial(const Polynomial& poly) : _monoVec(poly._monoVec), _linearFlag(poly._linearFlag), _normalFlag(poly._normalFlag) { }
	Polynomial  operator +  (const Polynomial& poly) const;
	Polynomial& operator += (const Monomial& mono);
	Polynomial& operator -= (const Monomial& mono);
	Polynomial  operator -  (const Polynomial& poly) const;
	Polynomial  operator -  () const;
	Polynomial  operator *  (const Polynomial& poly) const;

	inline bool empty() const { return _monoVec.size() == 0;}
	const vector<Monomial>& getMonoVec() const { return _monoVec; }
	inline bool isLinear() const { return _linearFlag; }
	inline bool isNormal() const { return _normalFlag; }

	void pushBack(const Monomial& mono) { _monoVec.push_back(mono); } 

	void normalized();
};

class Constraint {
	friend ostream & operator << (ostream & out, const Constraint& cons);
protected:
	Polynomial 	_poly;
	Float 	   	_limit;
	Op			_op;
public:
	Constraint() : _limit(NEGATIVE_INFINITY), _op(Op::UNDEF) {}
	explicit Constraint(Polynomial poly, Op op, Float limit) : _poly(poly),  _limit(limit), _op(op) { }

	const Polynomial& getPolynomial() const { return _poly; }
	const Int         getLimit()      const { return _limit; }
	const Op          getOp()         const { return _op; }
	const vector<Monomial>& getMonoVec() const { return _poly.getMonoVec(); }
	inline bool isLinear() const { return _poly.isLinear(); }
};

class NIA_Formula {
protected:
	Int 			   _varCnt;
	vector<Constraint> _consVec;
	Polynomial		   _objectiveFuntion;
public:
	inline void setVarCnt(Int varCnt) { _varCnt = varCnt; }
	inline Int  getVarCnt() const { return _varCnt; }
	inline Int  getConsCnt() const { return _consVec.size(); }
	const vector<Constraint>& getConsVec() const { return _consVec; }

	void addConstraint(Polynomial poly, Op op, Float limit) {poly.normalized(); _consVec.push_back(Constraint(poly, op, limit));}
	void addObjectiveFunction(Polynomial poly) { _objectiveFuntion = poly; }
	const Polynomial& getObjectiveFunction() const { return _objectiveFuntion; }

	void judgeConstraints() const;
	// void read
};

namespace lpReader {
	NIA_Formula readLpFile(string fileName);
	void readObjectiveFunction(NIA_Formula& formula, string line, Map<string, Int>& varMap);
	void readConstraint(NIA_Formula& formula, string line, Map<string, Int>& varMap);
	void readVars(NIA_Formula& formula, string line, Map<string, Int>& varMap);
}

} // namespace LS_NIA