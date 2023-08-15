#include "formula.hpp"
// NIA Formula

namespace LS_NIA {

const Variable Variable::start(0);
const Variable Variable::undef((unsigned) -1);
// const Value    Value::undef(NEGATIVE_INFINITY);

ostream& operator << (ostream &out, const Monomial& mono) {
    if (mono._coef > 0) out << "+" << mono._coef;
    else if (mono._coef < 0) out << mono._coef;
    else {
        out << "0";
        return out;
    }
    out << " ";
    for(const Variable& var : mono._vars) {
        out << "x" << var;
        // out << " x" << var;
    }
    return out;
}

ostream& operator << (ostream &out, const Polynomial& poly) {
    if (poly.empty()) {
        out << "0";
        return out;
    }
    for (const Monomial& mono : poly._monoVec) {
        out << mono << " ";
    }
    return out;
}

ostream& operator << (ostream &out, const Constraint& cons) {
    out << cons._poly;

    if (cons._op == Op::GEQUAL) out << " >= ";
    else if (cons._op == Op::LEQUAL) out << " <= ";
    else if (cons._op == Op::EQUAL) out << " = ";
    else if (cons._op == Op::UNDEF) out << " [undef] ";
    else util::showError("Wrong Op");

    out << cons._limit;

    return out;
}

// need normalized first
bool Monomial::operator == (const Monomial& mono) const {
    assert(_normalFlag && mono._normalFlag);
    if (_vars.size() != mono._vars.size()) return false;
    for (Int size = _vars.size(), i = 0; i < size; i++) {
        if (_vars[i] != mono._vars[i]) return false;
    }

    return true;
}

Monomial& Monomial::operator += (const Monomial& mono) {
    assert(*this == mono);
    _coef += mono._coef;
    return *this;
}

Monomial& Monomial::operator -= (const Monomial& mono) {
    assert(*this == mono);
    _coef -= mono._coef;
    return *this;
}

Monomial Monomial::operator * (const Monomial& mono) const {
    Monomial res(mono);

    for (const Variable& var : _vars) {
        res._vars.push_back(var);
    }
    res._coef *= _coef;

    res.normalized();
    return std::move(res);
}

Monomial Monomial::operator - () const {
    Monomial res(*this);
    res._coef = -res._coef;
    res.normalized();
    return std::move(res);
}

bool Monomial::isContain(const Variable& var) const {
    for(const Variable& _var : _vars) {
        if (_var == var) return true;
    }
    return false;
}

Polynomial Polynomial::operator + (const Polynomial& poly) const {
    Polynomial res(poly);
    for (const Monomial & mono : _monoVec) {
        res += mono;
    }
    res.normalized();
    return std::move(res);
}

// -= and += without normalized to minimize normalized times
Polynomial& Polynomial::operator += (const Monomial& mono) { 
    bool found = false;
    _normalFlag = false;
    for (Int size = _monoVec.size(), i = 0; i < size; i++) {           // ??? O(n) need update?
        if (_monoVec[i] == mono) {
            assert(!found);
            _monoVec[i] += mono;
            found = true;
        }
    }
    
    if(!found) _monoVec.push_back(mono);
    return *this;
}

// -= and += without normalized to minimize normalized times
Polynomial& Polynomial::operator -= (const Monomial& mono) { 
    bool found  = false;
    _normalFlag = false;
    for (Int size = _monoVec.size(), i = 0; i < size; i++) {           // ??? O(n) need update?
        if (_monoVec[i] == mono) {
            assert(!found);
            _monoVec[i] -= mono;
            found = true;
        }
    }
    
    if(!found) _monoVec.push_back(-mono);
    return *this;
}

Polynomial Polynomial::operator - (const Polynomial& poly) const {
    Polynomial res(*this);
    for (const Monomial& mono : poly._monoVec) {
        res -= mono;
    }
    res.normalized();
    return res;
}

Polynomial Polynomial::operator - () const {
    Polynomial res;
    for (const Monomial& mono : _monoVec) {
        res -= mono;
    }
    res.normalized();
    return std::move(res);
}

void Polynomial::normalized() {
    _linearFlag = true;
    for (auto it = _monoVec.begin(); it != _monoVec.end(); ) {
        if (it->empty()) {
            it = _monoVec.erase(it);    // delete empty mono
        }
        else {
            if (!it->isLinear()) _linearFlag = false;
            it++;
        }
    }
    _normalFlag = true;
}

Polynomial Polynomial::operator * (const Polynomial& poly) const {
    Polynomial res;
    for (const Monomial& mono1 : _monoVec) {
        for (const Monomial& mono2 : poly._monoVec) {
            res += mono1 * mono2;
        }
    }
    return std::move(res);
}

void NIA_Formula::judgeConstraints() const {
    assert(JUDGE);
    for (Int i = 0; i < _consVec.size(); i++) {
        cout << i << "  linear: " << _consVec[i].isLinear() << "  normal: " << _consVec[i].getPolynomial().isNormal() << endl;
    }
}

void lpReader::readObjectiveFunction(NIA_Formula& formula, string line, Map<string, Int>& varMap) {
    vector<string> words = util::splitStr(line, ' ');
    Int startIndex = 0;

    while (words.at(startIndex) == "") startIndex++;
    assert(words.at(startIndex) == "obj:");

    Int varCnt = varMap.size();
    Polynomial objectiveFunction;
    for (Int i = startIndex + 1, wordSize = words.size(); i < wordSize; i++) {
        if (words.at(i) == "") continue;
        Int coef   = std::stoi(words.at(i));
        string varStr = words.at(++i);
        Variable var;
        if(varMap.count(varStr) == 0) {
            var = varCnt;            
            varMap.insert(std::make_pair(varStr, varCnt++));
        }
        else {
            var = varMap.at(varStr);
        }
        objectiveFunction += Monomial(var, coef);
    }
    formula.addObjectiveFunction(objectiveFunction);
    if (DEBUG && true) cout << "Objective funtion: " << objectiveFunction << endl;
}

void lpReader::readConstraint(NIA_Formula& formula, string line, Map<string, Int>& varMap) {
    vector<string> words = util::splitStr(line, ' ');

    Polynomial poly;
    Int varCnt = varMap.size();
    Op  op     = Op::UNDEF;
    Int limit;
    Monomial mono;

    for (Int i = 1, wordSize = words.size(); i < wordSize; i++) {
        // if (words.at(i) == "" || words.at(i) == "[" || words.at(i) == "]") continue;
        if (words.at(i) == "" || words.at(i) == "[" || words.at(i) == "]" || words.at(i) == "+") continue;
        else if (words.at(i) == "*") {
            string varStr = words.at(++i);
            Variable var;
            if(varMap.count(varStr) == 0) {
                var = varCnt;            
                varMap.insert(std::make_pair(varStr, varCnt++));
            }
            else {
                var = varMap.at(varStr);
            }

            mono = mono * Monomial(var, 1);
        }
        else if (words.at(i) == "<=") {
            // poly += mono;
            poly.pushBack(mono);
            mono.clear();

            op = Op::LEQUAL;
            limit = std::stoi(words.at(++i));
        }
        else if (words.at(i) == ">=") {
            // poly += mono;
            poly.pushBack(mono);
            mono.clear();

            op = Op::GEQUAL;
            limit = std::stoi(words.at(++i));
        }
        else {
            if (!mono.empty()) {
                // poly += mono;
                poly.pushBack(mono);
                mono.clear();
            }

            Int coef   = std::stoi(words.at(i));
            string varStr = words.at(++i);
            Variable var;
            if(varMap.count(varStr) == 0) {
                var = varCnt;            
                varMap.insert(std::make_pair(varStr, varCnt++));
            }
            else {
                var = varMap.at(varStr);
            }

            if (mono.empty()) mono =  Monomial(var, coef);
            else assert(false);
        }
    }
    assert(op != Op::UNDEF);
    formula.addConstraint(poly, op, limit);
    if (DEBUG && false) cout << "Add constraint" << poly << endl;
}

void lpReader::readVars(NIA_Formula& formula, string line, Map<string, Int>& varMap) {
    vector<string> words = util::splitStr(line, ' ');
    Int cnt = 0;
    for (Int i = 1, wordSize = words.size(); i < wordSize; i++) {
        assert(varMap.count(words.at(i)) > 0);
        cnt ++;
    }
    assert(cnt == varMap.size());
}

NIA_Formula lpReader::readLpFile(string fileName) {
    std::ifstream fileStream(fileName);
    string line;

    Int  stage = 0;
    Map<string, Int> varMap;
    NIA_Formula formula;

    startTime = util::getTimePoint();

    while (getline(fileStream, line)) {
        if (line == "") continue;

        if (line == "Minimize") {
            assert(stage == 0);
            stage++;
        }
        else if (line == "Subject To") {
            assert(stage == 1);
            stage++;
        }
        else if (line == "Bounds") {
            assert(stage == 2);
            stage++;
        }
        else if (line == "Generals") {
            assert(stage == 3);
            stage++;
        }
        else if (stage == 1) readObjectiveFunction(formula, line, varMap);
        else if (stage == 2) readConstraint(formula, line, varMap);
        else if (stage == 3) continue;
        else if (stage == 4) readVars(formula, line, varMap);
    }

    formula.setVarCnt(varMap.size());

    // std::ofstream fout("mapping.out");
    // for (auto p : varMap) {
    //     fout << p.first << " -> " << p.second << endl;
    // }

    cout << "Reading Done " << util::getSeconds(startTime) << endl;
    return formula;
}

}   // namespace LS_NIA