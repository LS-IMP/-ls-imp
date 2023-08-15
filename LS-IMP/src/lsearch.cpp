#include "lsearch.hpp"

namespace LS_NIA {

void LsSolver::outputInfo(ostream& out) const {
    out << "#vars: " << _varCnt << endl;
    out << "#cons: " << _consCnt << endl;
}

bool LsSolver::checkOffFlag() const {
    if (_options._timeOff && util::getSeconds(startTime) > _options._maxTime && _curStep % 100 == 0) {
        if (DEBUG) cout << "Time off with limit: " << _options._maxTime << endl;
        return true;
    }
    if (_options._stepOff && _curStep >= _options._maxStep) {
        if (DEBUG) cout << "Step off with limit" << _options._maxStep << endl;
        return true;
    }

    return false;
}

void LsSolver::displayOffInfo() const {
    cout << "#step: " << _curStep << endl;
    cout << "#time: " << util::getSeconds(startTime) << endl;
}

void LsSolver::displayBestSolution() const {
    if (_bestUnSatConsNum == 0) {
        cout << " ***** SAT ***** " << endl;
    }
    else {
        cout << " ***** UNSAT *****    Best Unsat Cons Num: " << _bestUnSatConsNum << endl;
    }
    cout << "Best Value: " << _bestObjectiveValue << endl;
    cout << " ***** Best Assignment ******\n" << _bestAssignment << endl;
    if (_bestUnSatConsNum == 0)
        cout << "ObjectiveFuntion Value: " << _bestObjectiveValue << endl;
    else cout << "ObjectiveFuntion Value: inf" << endl;
}

void LsSolver::displayObjectiveAssignment(bool onlyUnBounded) const {
    cout << " ***** Objective Assignment Begin *****\n";

    const Polynomial& poly = _formula->getObjectiveFunction();
    Int cnt = 0, endlCnt = 6;
    for (const Monomial& mono : poly.getMonoVec()) {
        for (const Variable& var : mono.getVars()) {
            if (onlyUnBounded) {
                if (_assignment.getVal(var) < _assignment.getUB(var)) {
                    cout << var << ": " << _assignment.getVal(var) << " [" << _assignment.getLB(var) << ", " << _assignment.getUB(var)  << "]" << "\t";
                    if (++cnt % endlCnt == 0) cout << endl;
                }
            } else {
                cout << var << ": " << _assignment.getVal(var) << " [" << _assignment.getLB(var) << ", " << _assignment.getUB(var)  << "]" << "\t";
                if (++cnt % endlCnt == 0) cout << endl;
            }
        }
    }
    cout << endl << " ***** Objective Assignment Done *****\n";
}

void LsSolver::displayNoneZeroAssignment() const {
    cout << " ***** None Zero Assignment Begin *****\n";

    Int cnt = 0, endlCnt = 6;
    for (Variable var = Variable::start; var != _varCnt; var++) {
        if (_assignment.getVal(var) != 0) {
            cout << var << ": " << _assignment.getVal(var) << " [" << _assignment.getLB(var) << ", " << _assignment.getUB(var)  << "]" << "\t";
            if (++cnt % endlCnt == 0) cout << endl;
        }
    }
    cout << endl << " ***** None Zero Assignment Done *****\n";
}

void LsSolver::displayUsefulBestAssignment() const {
    Set<unsigned> varSet;

    cout << " ***** Useful Objective Assignment Begin *****\n";
    const Polynomial& poly = _formula->getObjectiveFunction();
    Int cnt = 0, endlCnt = 6;
    for ( const Monomial& mono : poly.getMonoVec()) {
        for (const Variable& var : mono.getVars()) {
            cout << var << ": " << _assignment.getVal(var) << " [" << _assignment.getLB(var) << ", " << _assignment.getUB(var)  << "]" << "\t";
            if (++cnt % endlCnt == 0) cout << endl;
            varSet.insert(var);
        }
    }

    for (Variable var = Variable::start; var != _varCnt; var++) {
        if (_assignment.getVal(var) != 0 && varSet.count(var) == 0) {
            cout << var << ": " << _assignment.getVal(var) << " [" << _assignment.getLB(var) << ", " << _assignment.getUB(var)  << "]" << "\t";
            if (++cnt % endlCnt == 0) cout << endl;
        }
    }

    cout << endl << " ***** Useful Objective Assignment Done *****\n"; 
}

void LsSolver::initSolver() {
    _varCnt  = _formula->getVarCnt();
    _consCnt = _formula->getConsCnt();

    initObjectiveVars();
    initAssignment();
    initConsValue();
    initConsWeight();
    initConsVarInfo();
    initConsCoef();
    initTabuStep();
    initDebugVar();         // for DEBUG
}

void LsSolver::initObjectiveVars() {
    _objectiveVars.clear();
    const Polynomial& objectiveFunction = _formula->getObjectiveFunction();

    for (const Monomial& mono : objectiveFunction.getMonoVec()) {
        for (const Variable& var : mono.getVars()) {
            _objectiveVars.push_back(var);
        }
    }
}

void LsSolver::initAssignment() {
    if (_options._greedyInit) {
        // not implemented yet
        assert(false);
    } 
    else {  // assign every var to 0;
        _assignment.allocateAuxiliaryMemory(_varCnt);
        for (Variable var = Variable::start; var != _varCnt; var++) {
            _assignment.valAt(var) = 0;
        }
    }

    initAssignmentBound();
    initBestAssignment();
}

void LsSolver::initAssignmentBound() {
    for (Variable var = Variable::start; var != _varCnt; var++) {
        if (JUDGE) assert(_assignment.valAt(var) == 0);     // _options.greedyInit = false
        _assignment.lbAt(var) = 0;                          // every variable >= 0
    }

    const vector<Constraint>& consVec = _formula->getConsVec();
    for (Int consIndex = 0; consIndex < _consCnt; consIndex ++) {
        // const Constraint& cons = consVec.at(consIndex);
        const Constraint& cons = consVec[consIndex];
        Int limit = cons.getLimit();
        Op  op    = cons.getOp();

        if (op == Op::GEQUAL) continue;
        else if (op == Op::EQUAL || op == Op::UNDEF) util::showError("Un except Op in initAssignmentBound");

        //  <= constraint
        for (Monomial mono : cons.getMonoVec()) {
            for (Variable var : mono.getVars()) {
                // _assignment.at(var).updateUB(limit);    // correct with x >= 0 
                _assignment.ubAt(var) = limit;    // correct with x >= 0 
            }
        }
    }

    if (DEBUG && _options._printStep && false) {
        cout << "Init Assignment:\n" << _assignment; 
        cout << " ***** Done ****** " << endl;
    }
}

void LsSolver::initBestAssignment() {
    _bestAssignment.allocateAuxiliaryMemory(_varCnt);
    _bestAssignment = _assignment;

    _bestObjectiveValue = -NEGATIVE_INFINITY;       // minimize
    _bestUnSatConsNum   = _consCnt;                 // minimize
}

void LsSolver::initConsValue() {
    _consValue.clear();
    _consValue.resize(_consCnt);
    assert(_unSatConstraint.size() == 0);

    const vector<Constraint>& consVec = _formula->getConsVec();
    if (JUDGE) assert(consVec.size() == _consCnt);
    for (Int consIndex = 0; consIndex < _consCnt; consIndex++) {
        const Constraint& cons = consVec[consIndex];
        _consValue[consIndex] = calcConsValue(cons);

        Op  op    = cons.getOp();
        Int limit = cons.getLimit();
        Int val   = _consValue[consIndex];

        if (!judgeLimitOpVal(limit, op, val)) addUnSatConstraint(consIndex);
    }
}

void LsSolver::initTabuStep() {
    _tabuStepOnVar.resize(_varCnt, 0);
}

bool LsSolver::judgeLimitOpVal(Int limit, Op op, Int val) const {

    if (op == Op::GEQUAL && val < limit) return false;
    else if (op == Op::LEQUAL && val > limit) return false;
    else if (op == Op::EQUAL || op == Op::UNDEF) util::showError("UnExcept Op::EQUAL in our instance");

    return true;
}

void LsSolver::initConsWeight() {
    _consWeight.clear();
    _consWeight.resize(_consCnt, 1);

    _objectWeight = 0;
}

/**
 * @brief init _consVarSet
 *             _var2ConsIndex
 * 
 */
void LsSolver::initConsVarInfo() {
    assert(_formula->getConsCnt() == _consCnt);

    _var2ConsIndex.clear();
    _var2ConsIndex.resize(_varCnt);
    for (Int i = 0; i < _varCnt; i++) _var2ConsIndex[i].clear();

    _consVarSet.clear();
    _consVarSet.resize(_consCnt);
    for (Int i = 0; i < _consCnt; i++) _consVarSet[i].clear();

    const vector<Constraint>& consVec = _formula->getConsVec();
    for (Int consIndex = 0; consIndex < _consCnt; consIndex++) {
        const Constraint& cons = consVec[consIndex];
        for (const Monomial& mono : cons.getMonoVec()) {
            for (const Variable& var : mono.getVars()) {
                if (JUDGE) assert(var < _varCnt);

                if (!util::isFound(consIndex, _var2ConsIndex[var]))
                    _var2ConsIndex[var].push_back(consIndex);
                _consVarSet[consIndex].insert(var);
            }
        }
    }
}

void LsSolver::initUnSatConstraint() {
    // implemented in initConstraintValue()
}

/**
 * @brief init _consCoefOnVar
 *             _consFreeOnVar
 *        assert(coefficient * var + freeTerm == value)
 *        p * x + q = v
 */
void LsSolver::initConsCoef() {
    _consCoefOnVar.resize(_consCnt);
    // _consFreeOnVar.resize(_consCnt);
    Variable var;
    for (Int consIndex = 0; consIndex < _consCnt; consIndex++) {
        const Constraint& cons = _formula->getConsVec().at(consIndex);
        // bool  linearFlag = cons.isLinear();
        for (const unsigned& varVal : _consVarSet.at(consIndex)) {
            var = varVal;
            if (JUDGE) assert(_consCoefOnVar.at(consIndex).find(var) == _consCoefOnVar.at(consIndex).end());
            // if (JUDGE) assert(_consFreeOnVar.at(consIndex).find(var) == _consFreeOnVar.at(consIndex).end());
            Float coef  = calcVarCoefOnPoly(cons.getPolynomial(), var);

            _consCoefOnVar[consIndex].insert(std::make_pair(var, coef));
        }
    }
}

void LsSolver::addUnSatConstraint(Int consIndex) {
    if (isUnSatConstraint(consIndex)) return;

    _unSatConstraint.insert(consIndex);
}

void LsSolver::delUnSatConstraint(Int consIndex) {
    if (!isUnSatConstraint(consIndex)) util::showError("Delete not Exists consIndex");
    
    _unSatConstraint.erase(consIndex);
}

bool LsSolver::judgeUnSatConstraint() const {
    assert(JUDGE);

    const vector<Constraint>& consVec = _formula->getConsVec();
    for (Int consIndex = 0; consIndex < _consCnt; consIndex ++) {
        const Constraint& cons = consVec.at(consIndex);
        assert(_consValue.at(consIndex) == calcConsValue(cons));

        Int  limit = cons.getLimit();
        Int  op    = cons.getOp();
        bool satF  = true;

        if (op == Op::GEQUAL) {
            if (_consValue.at(consIndex) < limit) satF = false;
        }
        else if (op == Op::LEQUAL) {
            if (_consValue.at(consIndex) > limit) satF = false;
        }
        else util::showError("Un Except Op::EQUAL or OP::UNDEF in judgeUnSatConstraint");

        assert((satF && !isUnSatConstraint(consIndex)) || (!satF && isUnSatConstraint(consIndex)));
    }

    return true;    // false will failed assert previously
}

bool LsSolver::judgeCoefFreeValue() const {
    assert(JUDGE);
    for (Int consIndex = 0; consIndex < _consCnt; consIndex ++) {
        const Constraint& cons = _formula->getConsVec()[consIndex];
        Float curValue = _consValue[consIndex];
        Variable var;
        for (const unsigned& varVal : _consVarSet[consIndex]) {
            var = varVal;
            // Float freeTerm = _consFreeOnVar.at(consIndex).at(var);
            Float coefTerm = _consCoefOnVar.at(consIndex).at(var);
            Float freeTerm = curValue - coefTerm * getVarAssign(var);
            // assert(curValue == coefTerm * getVarAssign(var) + freeTerm);
            if (JUDGE && coefTerm != calcVarCoefOnPoly(cons.getPolynomial(), var)) {
                cout << "Cons [" << consIndex << "]: " << cons << endl;
                cout << "Var: " << var << endl;
                cout << "Coef: " << coefTerm << " calc: " << calcVarCoefOnPoly(cons.getPolynomial(), var) << endl;
                cout << "Free: " << freeTerm << " calc: " << calcConsValueExVar(cons, var) << endl;
            }
            if (JUDGE) assert(coefTerm == calcVarCoefOnPoly(cons.getPolynomial(), var));
            if (JUDGE) assert(freeTerm == calcConsValueExVar(cons, var));
        }
    }
    return true;    // false will failed assert previously
}

Float LsSolver::calcConsValue(const Constraint& cons) const {
    const Polynomial& poly = cons.getPolynomial();
    return calcPolyValue(poly);
}

Float LsSolver::calcPolyValue(const Polynomial& poly) const {
    if (poly.empty()) return 0;

    Float res = 0;
    const vector<Monomial>& monoVec = poly.getMonoVec();
    for (const Monomial& mono : monoVec) {
        res += calcMonomialValue(mono);
    }
    return res;
}

Float LsSolver::calcConsValueExVar(const Constraint& cons, const Variable& exVar) const {
    const Polynomial& poly = cons.getPolynomial();
    if (poly.empty()) return 0;

    Float res = 0;
    const vector<Monomial>& monoVec = poly.getMonoVec();
    for (const Monomial& mono : monoVec) {
        res += calcMonomialValueExVar(mono, exVar);
    }
    return res;
}

Float LsSolver::calcMonomialValue(const Monomial& mono) const {
    if (mono.empty()) return 0;

    Float res = mono.getCoef();
    const vector<Variable>& vars = mono.getVars();
    for (Variable var : vars) {
        res *= getVarAssign(var);
    }
    return res;
}

Float LsSolver::calcMonomialValueExVar(const Monomial& mono, const Variable& exVar) const {
    if (mono.empty()) return 0;

    Float res = mono.getCoef();
    const vector<Variable>& vars = mono.getVars();
    for (Variable var : vars) {
        if (var == exVar) return 0;
        res *= getVarAssign(var);
    }
    return res;
}

Float LsSolver::calcVarCoefOnPoly(const Polynomial& poly, const Variable& exVar) const {
    if (poly.empty()) return 0;

    Float res = 0;
    const vector<Monomial>& monoVec = poly.getMonoVec();
    for (const Monomial& mono : monoVec) {
        res += calcVarCoefOnMono(mono, exVar);
    }
    return res;
}

Float LsSolver::calcVarCoefOnMono(const Monomial& mono, const Variable& exVar) const {
    if (mono.empty()) return 0;

    Float res = mono.getCoef();
    Float flg = 0;
    const vector<Variable>& vars = mono.getVars();
    for (Variable var : vars) {
        if (var == exVar) flg = 1;
        else res *= getVarAssign(var);
    }
    return res * flg;
}

/**
 * @brief clac coefTerm and freeTerm for var in cons
 * 
 *      coefTerm * var + freeTerm = consValue
 */
Pair<Float, Float> LsSolver::clacVarInfoInCons(const Int& consIndex, const Variable& exVar) const {
    Float coefTerm = _consCoefOnVar[consIndex].at(exVar);
    Float freeTerm = _consValue[consIndex] - coefTerm * getVarAssign(exVar);
    return std::make_pair(coefTerm, freeTerm);
}

bool LsSolver::liftObjectiveFunction() {
    const Polynomial& objectiveFunction = _formula->getObjectiveFunction();
    
    Float    bestScore  = NEGATIVE_INFINITY;
    Variable bestVar    = Variable::undef;
    Int      bestVarVal = 0;

    for (const Monomial& mono : objectiveFunction.getMonoVec()) {
        if (JUDGE) assert(mono.getVars().size() == 1);
        for (const Variable& var : mono.getVars()) {        // if size == 1, do not need for-loop
            if (_options._tabuFlag) {
                if (_curStep < _tabuStepOnVar[var]) continue;
            }

            // Float curCoef   = calcVarCoefOnPoly(objectiveFunction, var);    
            Float curCoef   = -1;                           // only for this formula
            Float curScore  = NEGATIVE_INFINITY;
            Int   curVarVal = -1;

            if (mono.getCoef() > 0) {
                util::showWarning("Not except coef in objective funtion is positive");
            }
            else if (mono.getCoef() < 0) {      // findMax to minimize objective function
                curVarVal = liftObjectiveFunctionOnVar(var, true);
                if (curVarVal == DUMMY_MAX_INT || curVarVal == DUMMY_MIN_INT) continue;
                curScore  = -1.0 * curCoef * (curVarVal - getVarAssign(var));
            }
            else util::showWarning("Empty Mono in objective function");
            if (curScore > bestScore) {
                bestScore  = curScore;
                bestVar    = var;
                bestVarVal = curVarVal;
            }
        }
    }

    // do lift
    if (bestScore > 0) {
        setVarWithNewVal(bestVar, bestVarVal);
        return true;
    }
    return false;
}

Int LsSolver::liftObjectiveFunctionOnVar(const Variable& var, bool findMax) {
    return findFeasibleVarValue(var, findMax);
}

Int LsSolver::findFeasibleVarValue(const Variable& var, bool findMax) {
    Int res = findMax ? DUMMY_MAX_INT : DUMMY_MIN_INT;      // feasible value, findMax needs min()

    for (Int consIndex : _var2ConsIndex[var]) {
        Int value = findFeasibleVarValueOnCons(var, consIndex, findMax);

        if (value == DUMMY_MIN_INT || value == DUMMY_MAX_INT) continue;
        res = findMax ? std::min(res, value) : std::max(res, value);
    }
    res = findMax ? std::min(res, getVarUB(var)) : std::max(res, getVarLB(var));        // limit with correctness ???

    return res;
}

/**
 * @brief coef * var + otherValue [op] limit
 *        if op is =   return exactValue = (limit - otherValue) / coef
 *        if op is >=  return (coef > 0 ? minValue : maxValue)
 *        if op is <=  return (coef < 0 ? maxValue : minValue)
 */
Int LsSolver::findFeasibleVarValueOnCons(const Variable& var, const Int consIndex, bool findMax) {
    const Constraint& cons = _formula->getConsVec()[consIndex];
    Op    op    = cons.getOp();
    Float limit = cons.getLimit();      // return is Int  need updated ?

    Float coefTerm = _consCoefOnVar[consIndex][var]; 
    Float freeTerm = _consValue[consIndex] - coefTerm * getVarAssign(var);

    // if (coefTerm == 0) return findMax ? _assignment.getUB(var) : _assignment.getLB(var);
    if (!couldCalc(op, coefTerm, findMax)) 
        return findMax ? DUMMY_MIN_INT : DUMMY_MAX_INT;  // can not find

    assert (coefTerm != 0);
    Float res = (limit - freeTerm) / coefTerm;

    if (DEBUG && var == _debugVar) {
        cout << "FreeTerm: " << freeTerm << endl;
        cout << "Var: " << var << "  In cons: " << cons <<endl;
        cout << "calc: " << res << endl;
    }

    if (op == Op::EQUAL) util::showError("Not implemented yet");  // EQUAL can not floor or ceil, need judge
    return findMax ? std::floor(res) : std::ceil(res);      // floor for max, ceil for min    -> Int 
}

bool LsSolver::couldCalc(Op op, Int coef, bool findMax) const {
    if (coef == 0) return false;
    if (op == Op::EQUAL) return true;
    bool couldMax = (op == Op::GEQUAL && coef < 0) || (op == Op::LEQUAL && coef > 0);

    return (couldMax && findMax) || (!couldMax && !findMax);
}

inline bool LsSolver::checkOperator(Variable var, Int val) const {
    // if (DEBUG && var == _debugVar) cout << "var: " << var << " val: " << val << "  tabu step: " << _tabuStepOnVar[var] << endl;
    if (val == DUMMY_MAX_INT || val == DUMMY_MIN_INT) return false;
    if (val == getVarAssign(var)) return false;
    if (!_assignment.isValid(var, val)) return false;
    if (_options._tabuFlag && _curStep < _tabuStepOnVar[var]) return false;
    return true;
}

void LsSolver::insertOperatorOnCons(const Int consIndex) {
    const Constraint& cons = _formula->getConsVec()[consIndex];
    for (const Monomial& mono : cons.getMonoVec()) {
        for (const Variable& var : mono.getVars()) {
            Int maxVal = findFeasibleVarValueOnCons(var, consIndex, true);
            Int minVal = findFeasibleVarValueOnCons(var, consIndex, false);

            // ??????? This is a problem !!!! 
            // 只插非界限
            // if (maxVal != DUMMY_MIN_INT) {
            //     maxVal = std::min(_assignment.getUB(var), maxVal);
            //     maxVal = std::max(_assignment.getLB(var), maxVal);
            // }
            // if (minVal != DUMMY_MAX_INT) {
            //     minVal = std::min(_assignment.getUB(var), minVal);
            //     minVal = std::max(_assignment.getLB(var), minVal);
            // }

            if (checkOperator(var, maxVal)) {
                if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << maxVal << "   in inOpOnCons" << endl;
                _operatorPool.push(var, maxVal);
            }
            if (checkOperator(var, minVal)) {
                if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << minVal << "   in inOpOnCons" << endl;
                _operatorPool.push(var, minVal);
            }
        }
    }
}

// judge objective var's value from unbounded constraint
bool LsSolver::doFeasibleSatOperator() {
    _operatorPool.clear();

    Set<Int> visitedConsIndex;
    for (const Variable& var : _objectiveVars) {
        // insertSatOperatorOnVar(var);

        if (checkOperator(var, _assignment.getLB(var))) {
            if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << _assignment.getLB(var) <<  "  in doFSO" << endl;
            _operatorPool.push(var, _assignment.getLB(var));
        }
        if (checkOperator(var, _assignment.getUB(var))) {
            if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << _assignment.getUB(var) <<  "  in doFSO" << endl;
            _operatorPool.push(var, _assignment.getUB(var));
        }

        for (const Int& consIndex : _var2ConsIndex[var]) {
            if (visitedConsIndex.count(consIndex) > 0) continue;
            if (_consValue[consIndex] == _formula->getConsVec()[consIndex].getLimit()) continue;
            visitedConsIndex.insert(consIndex);
            insertOperatorOnCons(consIndex);
        }
    }

    if (DEBUG && _options._printStep && !_operatorPool.empty()) cout << "In doFeasibleSatOperator: ";
    if (selectOperatorAndMove(0)) return true;      // need score > 0
    return false;
}

void LsSolver::insertSatOperatorOnVar(const Variable& var) {
    // const vector<Constraint>& consVec = _formula->getConsVec();

    for (Int consIndex : _var2ConsIndex[var]) {
        Int maxVal = findFeasibleVarValueOnCons(var, consIndex, true);
        Int minVal = findFeasibleVarValueOnCons(var, consIndex, false);

        if (JUDGE) {
            Float coefTerm = _consCoefOnVar[consIndex][var];
            Float freeTerm = _consValue[consIndex] - coefTerm * getVarAssign(var);
            Float maxValue = coefTerm * maxVal + freeTerm;
            Float minValue = coefTerm * minVal + freeTerm;

            Int limit = _formula->getConsVec()[consIndex].getLimit();
            Op op     = _formula->getConsVec()[consIndex].getOp();

            if (maxVal != DUMMY_MIN_INT) assert(judgeLimitOpVal(limit, op, maxValue));
            if (minVal != DUMMY_MAX_INT) assert(judgeLimitOpVal(limit, op, minValue));
        }

        if (checkOperator(var, maxVal)) {
            _operatorPool.push(var, maxVal);
            if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << maxVal <<  "  in insertSatOpOnVar" << endl;
        }
        if (checkOperator(var, minVal)) {
            _operatorPool.push(var, minVal);
            if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << minVal <<  "  in insertSatOpOnVar" << endl;
        }
    }
}

void LsSolver::insertSatBoundOperatorOnVar(const Variable& var, bool findMax) {
    Int  objValue = findMax ? _assignment.getUB(var) : _assignment.getLB(var);
    if (JUDGE) findMax ? assert(getVarAssign(var) < objValue) : assert(getVarAssign(var) > objValue); 

    if (checkOperator(var, objValue)) {
        _operatorPool.push(var, objValue);
        if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << objValue <<  "  in insertSatBoundOpOnVar" << endl;
    }
}

void LsSolver::insertSatOperatorOnVarUnBoundCons(const Variable& var, bool findMax) {
    Int oldVarVal = getVarAssign(var);
    if (JUDGE) findMax ? assert(oldVarVal != _assignment.getUB(var)) : assert(oldVarVal != _assignment.getLB(var));

    if (_options._tabuFlag && _curStep < _tabuStepOnVar[var]) return;

    for (Int consIndex : _var2ConsIndex[var]) {
        const Constraint& cons = _formula->getConsVec()[consIndex];

        if (_consValue[consIndex] == cons.getLimit()) continue; // bounded

        Int updVal = findFeasibleVarValueOnCons(var, consIndex, findMax);

        if (JUDGE) {
            Float coefTerm = _consCoefOnVar[consIndex][var];
            Float freeTerm = _consValue[consIndex] - coefTerm * getVarAssign(var);
            Float updValue = coefTerm * updVal + freeTerm;

            Int limit = cons.getLimit();
            Op op     = cons.getOp();

            if (updVal != DUMMY_MIN_INT && updVal != DUMMY_MAX_INT) assert(judgeLimitOpVal(limit, op, updValue));
        }

        if (findMax && updVal <= oldVarVal) continue;
        else if (!findMax && updVal >= oldVarVal) continue;

        if (checkOperator(var, updVal)) {
            _operatorPool.push(var, updVal);
            if (var == _debugVar) cout << "Operator Pool add: " << var << " -> " << updVal <<  "  in insertSatOpOnVarUnBoundCons" << endl;
        }
    }
}

bool LsSolver::doFeasibleUnSatOperator() {
    _operatorPool.clear();

    assert(_unSatConstraint.size() > 0);
    for (Int consIndex : _unSatConstraint) {
        if (DEBUG && _options._printStep) {
            cout << "UnSat Cons: [" << consIndex << "]:  " << _formula->getConsVec()[consIndex] << endl;
            cout << "Value: " << _consValue[consIndex] << "  limit: " << _formula->getConsVec()[consIndex].getLimit() << endl;
        }
        insertOperatorOnCons(consIndex);   
    }

    if (DEBUG && _options._printStep && !_operatorPool.empty()) cout << "In doFeasibleUnSatOperator: ";
    if (selectOperatorAndMove(0)) return true;
    return false;
}

bool LsSolver::doObjectiveBoundMove() {
    if (JUDGE) assert(_unSatConstraint.size() == 0);
    _operatorPool.clear();

    for (const Variable& var : _objectiveVars) {
        Int varAssign = getVarAssign(var);
        // assert coefTerm < 0
        if (varAssign != _assignment.getUB(var)) insertSatBoundOperatorOnVar(var, true);
    }

    if (DEBUG && _options._printStep && !_operatorPool.empty()) cout << "In doObjectiveBoundMove: "; 
    if (selectOperatorAndMove(0)) return true;
    return false;
}

bool LsSolver::doObjectiveTwoLevelMove() {
    if (JUDGE) assert(_unSatConstraint.size() == 0);
    const Polynomial& objectiveFunction = _formula->getObjectiveFunction();
    _operatorPool.clear();

    for (const Variable& var : _objectiveVars) {
        Int varAssign = getVarAssign(var);
        if (varAssign != _assignment.getUB(var)) insertSatOperatorOnVarUnBoundCons(var, true);
    }

    if (DEBUG && _options._printStep && !_operatorPool.empty()) cout << "In doObjectiveTwoLevelMove: ";
    if (selectOperatorAndMove(0)) return true;
    return false;
}

bool LsSolver::doSampleSatMove() {
    if (JUDGE) assert(_unSatConstraint.size() == 0);
    // const Polynomial& objectiveFunction = _formula->getObjectiveFunction();
    const vector<Constraint>& consVec   = _formula->getConsVec(); 

    _operatorPool.clear();

    Set<unsigned> objVars;
    vector<Pair<unsigned, Int> >  sampledVar2ConsIndex;
    vector<Pair<unsigned, bool> > sampledVar2UpperFlag;        // <non-objVar, upper Flag>

    const Int sampleConsBMS  = 50;
    Int sampleConsCnt;
    /* randomly sample 50 constraint for vars in objective funtion */
    for (const Variable& var : _objectiveVars) {
        // assert coef < 0
        objVars.insert(var);
        if (getVarAssign(var) == _assignment.getUB(var)) continue;  // bounded var

        Int consSize = _var2ConsIndex[var].size();
        sampleConsCnt = std::min(sampleConsBMS, consSize);

        for (Int randTimes = 0; randTimes < sampleConsCnt; randTimes++) {
            Int consID = (sampleConsCnt == sampleConsBMS ? genRandom() % consSize : randTimes);
            Int consIndex = _var2ConsIndex[var][consID];
            // can not set back here

            if (_consCoefOnVar[consIndex][var] == 0) continue;
            sampledVar2ConsIndex.push_back(std::make_pair(var, consIndex));
        } 
    }

    const Int sampleLinearBMS    = 100;
    const Int sampleNonLinearBMS = 1000;
    Int sampleLinearCnt;
    Int sampleNonLinearCnt;
    /* sample 1000 constraint */
    for (Pair<unsigned, Int>& var2Cons : sampledVar2ConsIndex) {
        unsigned var  =  var2Cons.first;
        Int consIndex = var2Cons.second;
        Int setSize = _consVarSet[consIndex].size();

        const Constraint& cons = consVec[consIndex];

        if (!cons.isLinear()) {     // nonlinear
            if (_consCoefOnVar[consIndex][var] >= 0) continue;  // only consider coefTerm < 0

            assert(cons.getOp() == Op::GEQUAL);
            sampleNonLinearCnt = std::min(setSize, sampleNonLinearBMS); 

            for (Int randTimes = 0; randTimes < sampleNonLinearCnt; randTimes++) {
                Int offset = (sampleNonLinearCnt == sampleNonLinearBMS ? genRandom() % setSize : randTimes);
                
                Set<unsigned>::iterator it = _consVarSet[consIndex].begin();
                std::advance(it, offset);
                unsigned curVar = *it;

                // coefTerm * var + freeTerm >= 0
                if (!objVars.count(curVar)) {
                    if (isFactor(cons.getPolynomial(), curVar, var)) {
                        sampledVar2UpperFlag.push_back(std::make_pair(curVar, false));
                    }
                    else {
                        if (_consCoefOnVar[consIndex][curVar] > 0) sampledVar2UpperFlag.push_back(std::make_pair(curVar, true));
                        else sampledVar2UpperFlag.push_back(std::make_pair(curVar, false));
                    }
                }
            }
        }
        else {                      // lineaer constraint
            if (cons.getOp() == Op::GEQUAL) continue;       // Do not consider Demand Constratint,  >= limit
            sampleLinearCnt = std::min(setSize, sampleLinearBMS);

            for (Int randTimes = 0; randTimes < sampleLinearCnt; randTimes++) {
                Int offset = (sampleLinearCnt == sampleLinearBMS ? genRandom() % setSize : randTimes);
                
                Set<unsigned>::iterator it = _consVarSet[consIndex].begin();
                std::advance(it, offset);
                unsigned curVar = *it;

                if (!objVars.count(curVar)) {      // curVar not in objective function
                    if (JUDGE) assert(_consCoefOnVar[consIndex][curVar] > 0);
                    sampledVar2UpperFlag.push_back(std::make_pair(curVar, false));  // do lower
                }
            }
        }
    }


    // const Int sampleConsBMS = 50
    for (Pair<unsigned, bool>& var2Flag : sampledVar2UpperFlag) {
        Variable var(var2Flag.first);
        bool     upperFlag = var2Flag.second;

        Int consSize = _var2ConsIndex[var].size();
        // sampleConsCnt = consSize > sampleConsBMS ? sampleConsBMS : consSize;
        sampleConsCnt = sampleConsBMS;

        for (Int randTimes = 0; randTimes < sampleConsCnt; randTimes++) {
            Int consID = (sampleConsCnt == sampleConsBMS ? genRandom() % consSize : randTimes);
            Int consIndex = _var2ConsIndex[var][consID];
            // const Constraint& cons = consVec[consIndex];

            if (_consCoefOnVar[consIndex][var] == 0) continue;      // coefTerm != 0
            
            if (upperFlag) {
                Int maxVal = findFeasibleVarValueOnCons(var, consIndex, true);
                if (checkOperator(var, maxVal)) {
                    _operatorPool.push(var, maxVal);
                    if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << maxVal <<  "  in sampleSatOp" << endl;
                }
            }
            else {
                Int minVal = findFeasibleVarValueOnCons(var, consIndex, false);
                if (checkOperator(var, minVal)) {
                    _operatorPool.push(var, minVal);
                    if (DEBUG && var == _debugVar) cout << "Operator Pool add: " << var << " -> " << minVal <<  "  in sampleSatOp" << endl;
                }
            }
        } 

    }

    if (DEBUG && _options._printStep && !_operatorPool.empty()) cout << "In doSampleSatMove: ";
    if (selectOperatorAndMove(-_objectWeight)) return true;
    return false;
}

// ??? update, now is O(n)  maybe n^2
bool LsSolver::isFactor(const Polynomial& poly, unsigned var1, unsigned var2) const {
    Variable minVar(std::min(var1, var2));
    Variable maxVar(std::max(var1, var2));

    Monomial judgeMono = Monomial(minVar, 1) * Monomial(maxVar, 1);

    for (const Monomial& mono : poly.getMonoVec()) {
        if (judgeMono == mono) return true;
    }
    return false;
}

bool LsSolver::selectOperatorAndMove(Float minScore) {
    if (_operatorPool.empty()) return false;

    Int  poolSize = _operatorPool.size();
    // bool bmsFlag  = poolSize < _options._bmsThreshold ? false : true;
    Int  smpCnt   = std::min(_options._bmsThreshold, poolSize);

    if (DEBUG && _options._printStep) cout << "Operator Pool Size: " << poolSize << "  smpCnt: " << smpCnt << endl;

    Variable bestVar;
    Int      bestValue;
    Float    bestScore = NEGATIVE_INFINITY;

    for (Int i = 0; i < smpCnt; i++) {
        Variable curVar;
        Int      curValue;
        Float    curScore = NEGATIVE_INFINITY;

        assert(poolSize > 0);

        Int randIndex = genRandom() % poolSize;
        curVar   = _operatorPool.varAt(randIndex);
        curValue = _operatorPool.valAt(randIndex);
        _operatorPool.removeOpAt(randIndex); 
        poolSize--;

        curScore = clacScore(curVar, curValue);
        if (DEBUG && curVar == _debugVar) cout << curVar << " " << getVarAssign(curVar) << " -> " << curValue << "  score: " << curScore << endl;
        if (curScore > bestScore) {
            bestScore = curScore;
            bestVar   = curVar;
            bestValue = curValue;
        }
    }

    if (DEBUG && _options._printStep) cout << "After select best score: " << bestScore 
        << " (" << clacHardScore(bestVar, bestValue) << ", " << clacSoftScore(bestVar, bestValue) 
        << ")  " << bestVar << " -> " << bestValue << endl;
    if (bestScore > minScore) {
        setVarWithNewVal(bestVar, bestValue);
        return true;
    } 
    return false;
}

/**
 * @brief coef * var + free [op] limit
 * 
 */
Float LsSolver::clacHardScore(Variable var, Int val) const {
    Float    score = 0;

    for (Int consIndex : _var2ConsIndex[var]) {
        const Constraint& cons = _formula->getConsVec()[consIndex];

        Int limit  = cons.getLimit();
        Op  op     = cons.getOp();
        if (JUDGE) assert(_consVarSet[consIndex].count(var) > 0);

        Float coefTerm   = _consCoefOnVar[consIndex].at(var);
        Float freeTerm   = _consValue[consIndex] - coefTerm * getVarAssign(var);

        Float preValue   = _consValue[consIndex];
        Float postValue  = freeTerm + coefTerm * val;
        if (JUDGE) assert(preValue == _consValue[consIndex]);

        bool preSat  = judgeLimitOpVal(limit, op, preValue);
        bool postSat = judgeLimitOpVal(limit, op, postValue);

        if (preSat && !postSat) {       // sat -> unsat 
            score -= _consWeight[consIndex];
        } 
        else if (!preSat && postSat) {  // unsat -> sat
            score += _consWeight[consIndex];
        }
    }
    return score;
}

Float LsSolver::clacSoftScore(Variable var, Int val) const {
    Float coef  = calcVarCoefOnPoly(_formula->getObjectiveFunction(), var);
    Float score = coef * (val - getVarAssign(var));       // minimize objective function

    if (score < 0) return 1.0 * _objectWeight;
    else if (score > 0) return -1.0 * _objectWeight;
    else return 0;
}

/**
 * @brief random walk belong unbounded cons
 * 
 */
void LsSolver::randomWalkSat() {
    _operatorPool.clear();
    vector<Int> consPool;
    const vector<Constraint>& consVec = _formula->getConsVec();
    for (Int consIndex = 0; consIndex < _consCnt; consIndex++) {
        if (JUDGE) assert(!isUnSatConstraint(consIndex));
        if (_consValue[consIndex] != consVec[consIndex].getLimit()) consPool.push_back(consIndex);
    }

    Int consPoolSize = consPool.size();
    for (Int i = 0 ; i < _options._randomStep; i++) {
        Int index;
        if (consPool.size() <= _options._randomStep) {
            if (i >= consPool.size()) break;
            index = i;
        }
        else index = genRandom() % (consPoolSize - 1);

        insertOperatorOnCons(consPool[index]);
    }

    Int randomSelectConsIndex = consPool.size() == 1 ? 0 : consPool[genRandom() % (consPoolSize - 1)];

    if (DEBUG && _options._printStep && !_operatorPool.empty()) cout << "In randomWalkSat: ";
    if (!_operatorPool.empty() && selectOperatorAndMove(NEGATIVE_INFINITY)) return;     // 有操作就执行
    else randomWalkOnCons(consVec[randomSelectConsIndex]);
}

bool LsSolver::randomWalkSatOnObjVar() {
    vector<Variable> unBoundedObjVars;
    for (const Variable& var : _objectiveVars) {
        if (getVarAssign(var) != _assignment.getUB(var)) unBoundedObjVars.push_back(var);
    }

    if (unBoundedObjVars.size() == 0) return false;

    Int randInex = genRandom() % unBoundedObjVars.size();

    Variable var = unBoundedObjVars[randInex];
    Int newVal = getVarAssign(var) + 1;

    if (JUDGE) assert(newVal <= _assignment.getUB(var));

    setVarWithNewVal(var, newVal);
    return true;
}

void LsSolver::randomWalkUnSat() {
    _operatorPool.clear();

    vector<Int> consPool;
    const vector<Constraint>& consVec = _formula->getConsVec();
    for (Int consIndex : _unSatConstraint) {
        if (JUDGE) assert(isUnSatConstraint(consIndex));
        consPool.push_back(consIndex);
    }

    Int consPoolSize = consPool.size();
    for (Int i = 0 ; i < _options._randomStep; i++) {
        Int index;
        if (consPool.size() <= _options._randomStep) {
            if (i >= consPool.size()) break;
            index = i;
        }
        else index = genRandom() % (consPoolSize - 1);

        // insertOperatorOnCons(consVec.at(consPool.at(index)));
        insertOperatorOnCons(consPool[index]);
    }

    // Int randomSelectConsIndex = consPool.size() == 1 ? 0 : consPool.at(genRandom() % (consPoolSize - 1));
    Int randomSelectConsIndex = consPool.size() == 1 ? 0 : consPool[genRandom() % (consPoolSize - 1)];

    if (DEBUG && _options._printStep && !_operatorPool.empty()) cout << "In randomWalkUnSat: ";
    if (!_operatorPool.empty() && selectOperatorAndMove(NEGATIVE_INFINITY)) return;
    else randomWalkOnCons(consVec[randomSelectConsIndex]);
}

void LsSolver::randomWalkOnCons(const Constraint& cons) {
    /* random select cons and var */
    if (DEBUG && _options._printStep) {
        cout << "Completely random walk" << std::endl;
        cout << cons.getPolynomial() << " " << cons.getOp() << " " << cons.getLimit() << endl;
    }
    if (JUDGE) assert(_operatorPool.empty());
    vector<Variable> varPool;
    for (const Monomial& mono : cons.getMonoVec()) {
        for (const Variable& var : mono.getVars()) {
            varPool.push_back(var);
        }
    }
    assert(varPool.size() >= 1);
    Variable selectVar = varPool[varPool.size() == 1 ? 0 : genRandom() % (varPool.size() - 1)];
    Int val = genRandom() % 2 ? _assignment.getLB(selectVar) : _assignment.getUB(selectVar);

    if (val == _assignment.getUB(selectVar)) val = _assignment.getLB(selectVar);
    else if (val == _assignment.getLB(selectVar)) val = _assignment.getUB(selectVar);

    setVarWithNewVal(selectVar, val);
}

/**
 * @brief update _assignment
 *               _consValue
 *               _unSatConstraint
 *               _tabuStepOnVar
 *               _consCoefOnVar
 *               _consFreeOnVar
 */
void LsSolver::setVarWithNewVal(const Variable& var, Int val) {
    if (JUDGE) assert(_assignment.isValid(var, val));
    if (DEBUG && _options._printStep) cout << "Set: " << var << " " << val << endl;
    
    Int oldVal = getVarAssign(var);
    if (oldVal == val) return ;

    setVarAssign(var, val);     // now assign of var is newVal

    const vector<Constraint>& conVec = _formula->getConsVec();
    for (Int consIndex : _var2ConsIndex[var]) {
        updateConsInfo(consIndex, var, val - oldVal);

        const Constraint& cons = conVec[consIndex];
        _consValue[consIndex] = calcConsValue(cons);

        Op  op    = cons.getOp();
        Int limit = cons.getLimit();
        Int val   = _consValue[consIndex];

        if (judgeLimitOpVal(limit, op, val)) {
            if (isUnSatConstraint(consIndex)) delUnSatConstraint(consIndex);
        }
        else {
            if (!isUnSatConstraint(consIndex)) addUnSatConstraint(consIndex);
        }
    }

    if (_options._tabuFlag) {       // update tabuStepOnVar
        if (DEBUG && var == _debugVar) cout << var << ": " << _tabuStepOnVar.at(var) << "  curStep: " << _curStep << endl;
        _tabuStepOnVar[var] = _curStep + _options._tabuConst + genRandom() % _options._tabuRand;
    }
}

/**
 * @brief update _consValue
 *               _consCoefOnVar
 *               _consFreeOnVar
 *               _unSatConstraint
 */
void LsSolver::updateConsInfo(const Int& consIndex, const Variable& var, const Int& delta) {
    const Constraint& cons = _formula->getConsVec()[consIndex];

    // now assign of var is new var
    Float coefTerm = _consCoefOnVar[consIndex][var];
    _consValue[consIndex] += coefTerm * delta;
    Float freeTerm = _consValue[consIndex] - coefTerm * getVarAssign(var);

    if (JUDGE) assert(coefTerm == calcVarCoefOnPoly(cons.getPolynomial(), var));
    if (JUDGE) assert(freeTerm == calcConsValueExVar(cons, var));
    if (JUDGE) assert(coefTerm * getVarAssign(var) + freeTerm == _consValue.at(consIndex));

    if (!cons.isLinear()) {              // linear do not need update coef term
        Map<unsigned, Float>& curCoefOnVar = _consCoefOnVar[consIndex];
        for (const Monomial& mono : cons.getMonoVec()) {
            if (mono.isContain(var)) {  // var \in mono, adjust otherVar's coef
                for (const Variable& otherVar : mono.getVars()) {
                    if (otherVar == var) continue;
                    if (DEBUG && otherVar == _debugVar && false) cout << "DEBUG Var: " << otherVar << " preCoef: " << curCoefOnVar[otherVar];
                    curCoefOnVar[otherVar] += mono.getCoef() * delta;
                    if (DEBUG && otherVar == _debugVar && false) cout << " postCoef: " << curCoefOnVar[otherVar] << " calcCoef: " << calcVarCoefOnPoly(cons.getPolynomial(), otherVar) << endl;
                }
            }
        }
    }
}

void LsSolver::updateConstraintWeight() {
    const vector<Constraint>& consVec = _formula->getConsVec();
    for (Int consIndex : _unSatConstraint) {
        const Constraint& cons = consVec[consIndex];

        if (JUDGE) assert(_consValue.at(consIndex) == calcConsValue(cons));
        if (JUDGE) assert(!judgeLimitOpVal(cons.getLimit(), cons.getOp(), _consValue.at(consIndex)));

        _consWeight[consIndex]++;
    }

    // update objective weight
    if (_objectWeight < 100 && _bestUnSatConsNum == 0 && _curObjectiveValue >= _bestObjectiveValue) {
        _objectWeight++;
    }
}

bool LsSolver::updateResultJudge() {
    if (JUDGE) assert(_bestUnSatConsNum >= 0);
    if (_bestUnSatConsNum > 0) {    // unsat assignment
        Int curUnSatConsNum = _unSatConstraint.size();
        if (curUnSatConsNum < _bestUnSatConsNum) return true;
        else if (curUnSatConsNum == _bestUnSatConsNum && _curObjectiveValue < _bestObjectiveValue) return true;
    }
    else if (_unSatConstraint.size() == 0) {    // already found a sat assginment and update objective function
        if ( _bestObjectiveValue > _curObjectiveValue) { // minimiaze
            return true;
        }
    }
    return false;
}

void LsSolver::updateResult() {
    _curObjectiveValue = calcPolyValue(_formula->getObjectiveFunction());
    if (updateResultJudge()) {
        if (JUDGE) assert (_unSatConstraint.size() <= _bestUnSatConsNum);

        if (_bestUnSatConsNum > 0 && _unSatConstraint.size() == 0) _objectWeight = 1;

        _bestUnSatConsNum   = _unSatConstraint.size();
        _bestAssignment     = _assignment;
        _bestObjectiveValue = _curObjectiveValue;
    }
}

void LsSolver::oldVersion() {
    cout << "This is old Version" << endl;
    Int liftCnt = 0;
    Int feasibleSatCnt = 0, feasibleUnSatCnt = 0;
    Int randomSatCnt = 0, randomUnSatCnt = 0;

    for (_curStep = 0; _curStep < _options._maxStep; _curStep++) {
        if (DEBUG && _options._printStep) {
            cout << endl << "step: " << _curStep << ", ";
            cout << "#unsat: " << getUnSatConsCnt() << ", ";
            cout << "Time: " << util::getSeconds(startTime) << endl;
            cout << "Best Value: " << _bestObjectiveValue << endl;
        }
        if (checkOffFlag()) {
            displayOffInfo();
            displayBestSolution();
            return ;
        }

        unsigned judgeVar;
        Int      judgeValue;
        bool     judgeRandomOp = false;
        if (DEBUG && false) {
            string str;
            using std::cin;
            cout << "wait input: ";
            while (cin >> str) {
                cout << str << " ";
                if (str == "go") break;
                else if (str == "watch") {  // watch
                    unsigned watchedVar;
                    cin >> watchedVar;
                    _debugVar = watchedVar;
                    cout << "watchedVar" << endl;
                } else if (str == "do") {   // set var new val
                    cin >> judgeVar >> judgeValue;
                    cout << judgeVar << " " << judgeValue << endl;
                    cout << "var: " << judgeVar << "  val: " << judgeValue << " score: " << clacScore(Variable(judgeVar), judgeValue) 
                            << " (" << clacHardScore(Variable(judgeVar), judgeValue) << ", " << clacSoftScore(Variable(judgeVar), judgeValue) << ")" << endl;
                    // setVarWithNewVal(Variable(var), val);
                    break;
                } else if (str == "score") {
                    unsigned var;
                    Int val;
                    cin >> var >> val;
                    cout << var << " " << val << endl;
                    cout << "var: " << var << "  val: " << val << " score: " << clacScore(Variable(var), val) 
                        << " (" << clacHardScore(Variable(var), val) << ", " << clacSoftScore(Variable(var), val) << ")" << endl;
                } else if (str == "print_UNSAT") {
                    for (Int consIndex : _unSatConstraint) {
                        cout << "UnSat Cons: [" << consIndex << "] (" << _consWeight[consIndex] << "):  " << _formula->getConsVec()[consIndex] << endl;
                        cout << "Value: " << _consValue[consIndex] << "  limit: " << _formula->getConsVec()[consIndex].getLimit() << endl;
                    }
                }
                cout << endl << "wait input: ";
            }
        }

        // Main Search
        if (getSatState()) {    // SAT
            if (_options._liftFlag && liftObjectiveFunction()) {
                if (DEBUG && _options._printStep) cout << "Did Lift Op" << endl;
                liftCnt++;
                judgeRandomOp = true;
            }
            else if (doFeasibleSatOperator()) {
                if (DEBUG && _options._printStep) cout << "Did feasible Sat Op" << endl;
                feasibleSatCnt++;
            }
            else {
                updateConstraintWeight();
                randomWalkSat();
                randomSatCnt++;
                judgeRandomOp = true;
                if (DEBUG && _options._printStep) cout << "Did Random Walk On Sat" << endl;
            }
        }
        else {                  // UNSAT
            if (doFeasibleUnSatOperator()) {
                if (DEBUG && _options._printStep) cout << "Did feasible UnSat Op" << endl;
                feasibleUnSatCnt++;
            }
            else {
                updateConstraintWeight();
                randomWalkUnSat();
                judgeRandomOp = true;
                if (DEBUG && _options._printStep) cout << "Did Random Walk On UnSat" << endl;
                randomUnSatCnt++;
            }
        }

        if (JUDGE) judgeUnSatConstraint();
        if (JUDGE) judgeCoefFreeValue();
        updateResult();

        // displayBestSolution();
        if (DEBUG && _options._printStep) {
            displayObjectiveAssignment(true);
            displayNoneZeroAssignment();
            cout << "Obj Value: " << calcPolyValue(_formula->getObjectiveFunction()) << "  Best: " << _bestObjectiveValue << endl;
            // if (_curStep % 10 == 0) getchar();
        }
    }
}

void LsSolver::newVersion() {
    cout << "This is new Version" << endl;
    Int liftCnt = 0;

    for (_curStep = 0; _curStep < _options._maxStep; _curStep++) {
        if (DEBUG && _options._printStep) {
            cout << endl << "step: " << _curStep << ", ";
            cout << "#unsat: " << getUnSatConsCnt() << ", ";
            cout << "Time: " << util::getSeconds(startTime) << endl;
            cout << "Best Value: " << _bestObjectiveValue << endl;
        }
        if (checkOffFlag()) {
            displayOffInfo();
            displayBestSolution();
            return ;
        }

        // Main Search
        if (getSatState()) {    // SAT
            if (doObjectiveBoundMove()) {
                if (DEBUG && _options._printStep) cout << "Did Objective Bound Move" << endl;
            }
            else if (doObjectiveTwoLevelMove()) {
                if (DEBUG && _options._printStep) cout << "Did Objective Two Level Move" << endl;
            }
            else if (doSampleSatMove()) {
                if (DEBUG && _options._printStep) cout << "Did Objective Sample Move" << endl;
            }
            else { 
                updateConstraintWeight();
                // randomWalkSat();
                if (!randomWalkSatOnObjVar()) {
                    displayOffInfo();
                    displayBestSolution();
                    return;
                }
                if (DEBUG && _options._printStep) cout << "Did Random Walk On Sat" << endl;
            }
        }
        else {                  // UNSAT
            if (doFeasibleUnSatOperator()) {
                if (DEBUG && _options._printStep) cout << "Did feasible UnSat Op" << endl;
            }
            else {
                updateConstraintWeight();
                randomWalkUnSat();
                if (DEBUG && _options._printStep) cout << "Did Random Walk On UnSat" << endl;
            }
        }

        if (JUDGE) judgeUnSatConstraint();
        if (JUDGE) judgeCoefFreeValue();
        updateResult();

        // displayBestSolution();
        if (DEBUG && _options._printStep) {
            displayObjectiveAssignment(true);
            displayNoneZeroAssignment();
            cout << "Obj Value: " << calcPolyValue(_formula->getObjectiveFunction()) << "  Best: " << _bestObjectiveValue << endl;
            // if (_curStep % 10 == 0) getchar();
        }
    }
}

void LsSolver::solve(bool useNewVersion) {
    initSolver();
    outputInfo(cout);

    if (useNewVersion) newVersion();
    else oldVersion();
}

}
