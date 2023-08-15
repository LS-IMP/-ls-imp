#include "utils.hpp"
#include "formula.hpp"
#include "assignment.hpp"


namespace LS_NIA {

struct Options {
    bool    _greedyInit;
    Int     _bmsThreshold;
    Int     _maxStep;
    Float   _maxTime;                      // in second
    bool    _timeOff;
    bool    _stepOff;
    bool    _liftFlag;

    bool    _tabuFlag;
    Int     _tabuConst;
    Int     _tabuRand;

    Int     _randomStep;                   // used for random walk

    bool    _printStep;                     // print for Debug

    Options() {
        _greedyInit   = false;
        // _bmsThreshold = 1000; 
        _bmsThreshold = 100; 
        _maxStep      = DUMMY_MAX_INT;
        // _maxTime      = 300;               // in second
        _maxTime      = 60;               // in second
        _timeOff      = true;
        _stepOff      = true;
        _liftFlag     = true;

        _tabuFlag     = true;
        _tabuConst    = 3;
        _tabuRand     = 10;

        _randomStep   = 10;

        _printStep    = false;
    }
};

class LsSolver {
protected:
    const NIA_Formula* _formula;

    Options         _options;
    Variable        _varCnt;
    Int             _consCnt;
    Assignment      _assignment;

    vector<Variable> _objectiveVars;

    Float           _curObjectiveValue;
    vector<Float>   _consValue;                 // cons.poly value on current assignment ??? Float ?
    vector<Float>   _consWeight;                // cons weight used for hard score

    OperatorPool    _operatorPool;

    Set<Int>        _unSatConstraint;           // store unsat Constraint Index
    // Set<Int>        _unBndConstraint;           // store unbounded Constraint Index

    Int             _curStep;
    vector<Int>     _tabuStepOnVar;             // store tabu step on var;

    vector<vector<Int> > _var2ConsIndex;        // .at(var).at(i) is constraint Index

    Assignment      _bestAssignment;            // store best assignment
    Float           _bestObjectiveValue;        // store best objective value
    Int             _bestUnSatConsNum;          // store best UNSAT constraint num 

    Float           _objectWeight;              // used for soft score

    Variable        _debugVar;                  // for debug
    // vector<Float>   _constraintWeight;          // used for hard score

    // bool                    _useCoef = true;       // use coef or freeTerm
    vector<Set<unsigned> >  _consVarSet;           // .at(consIndex) = variables appear in consIndex
    vector<Map<unsigned, Float> > _consCoefOnVar;  // .at(consIndex).at(var) = coefficient of var in cons.at(consIndex)
    // vector<Map<unsigned, Float> > _consFreeOnVar;  // .at(consIndex).at(var) = free term of var in cons.at(consIndex)

    void outputInfo(ostream &out) const;
    bool checkOffFlag() const;

    void displayOffInfo() const;
    void displayBestSolution() const;
    void displayObjectiveAssignment(bool onlyUnBounded = false) const;
    void displayNoneZeroAssignment() const;
    void displayUsefulBestAssignment() const;

    void initSolver();
    void initObjectiveVars();
    void initAssignment();
    void initBestAssignment();
    void initAssignmentBound();
    void initConsValue();
    void initConsWeight();
    void initTabuStep();
    void initConsVarInfo();         // init _var2ConsIndex _consVarSet
    void initUnSatConstraint();
    void initConsCoef();
    void initDebugVar() {_debugVar = Variable::undef;}      // for DEBUG
    // void initDebugVar() {_debugVar = 5180u;}      // for DEBUG

    Float calcConsValue(const Constraint& cons) const;
    Float calcPolyValue(const Polynomial& poly) const;
    Float calcConsValueExVar(const Constraint& cons, const Variable& exVar) const;
    Float calcMonomialValue(const Monomial& mono) const;
    Float calcMonomialValueExVar(const Monomial& mono, const Variable& exVar) const;
    Float calcVarCoefOnPoly(const Polynomial& poly, const Variable& exVar) const;
    Float calcVarCoefOnMono(const Monomial& mono, const Variable& exVar) const;

    Pair<Float, Float> clacVarInfoInCons(const Int& consIndex, const Variable& exVar) const;

    void addUnSatConstraint(Int consIndex);
    void delUnSatConstraint(Int consIndex);
    inline bool isUnSatConstraint(Int consIndex) const { return _unSatConstraint.count(consIndex) > 0; }
    inline Int  getUnSatConsCnt() const { return _unSatConstraint.size(); }
    
    bool judgeUnSatConstraint() const;
    bool judgeCoefFreeValue() const;
    bool judgeLimitOpVal(Int limit, Op op, Int val) const;
    
    inline bool getSatState() const { return _unSatConstraint.size() == 0; }

    inline Int  getVarAssign(const Variable& var) const { return _assignment.getVal(var); }
    inline void setVarAssign(const Variable& var, Int val) { _assignment.valAt(var) = val; }
    inline Int  getVarLB(const Variable& var) const { return _assignment.getLB(var); }
    inline Int  getVarUB(const Variable& var) const { return _assignment.getUB(var); }

    bool liftObjectiveFunction();
    Int  liftObjectiveFunctionOnVar(const Variable& var, bool findMax);

    Int  findFeasibleVarValue(const Variable& var, bool findMax);
    Int  findFeasibleVarValueOnCons(const Variable& var, const Int consIndex, bool findMax);     // Bounded Value
    // Int  findBoundedVarValueOnCons(const Variable& var, const Constraint& cons, bool findMax);

    inline bool couldCalc(Op op, Int coef, bool findMax) const;
    inline bool checkOperator(Variable var, Int val) const;
    
    void insertOperatorOnCons(const Int consIndex);

    bool doFeasibleSatOperator();
    void insertSatOperatorOnVar(const Variable& var);
    void insertSatBoundOperatorOnVar(const Variable& var, bool findMax);
    void insertSatOperatorOnVarUnBoundCons(const Variable& var, bool findMax);

    bool doFeasibleUnSatOperator();
    // void insertUnSatOperatorOnVar( const Variable& var);

    // new SAT moves
    bool doObjectiveBoundMove();
    bool doObjectiveTwoLevelMove();
    bool doSampleSatMove();

    bool isFactor(const Polynomial& poly, unsigned var1, unsigned var2) const;

    bool selectOperatorAndMove(Float minScore);

    Float clacScore( Variable var, Int val) const { return clacHardScore(var, val) + clacSoftScore(var, val); }  
    Float clacHardScore( Variable var, Int val) const;
    Float clacSoftScore( Variable var, Int val) const;

    void randomWalkSat();
    bool randomWalkSatOnObjVar();
    void randomWalkUnSat();
    void randomWalkOnCons(const Constraint& cons);

    void setVarWithNewVal(const Variable& var, Int val);       // move
    void updateConsInfo(const Int& consIndex, const Variable& var, const Int& val);       // update _consFactorOnVar

    void updateConstraintWeight();
    bool updateResultJudge();
    void updateResult();

    void oldVersion();
    void newVersion();

public:
    LsSolver(const NIA_Formula& formula) { _formula = &formula; };
    void solve(bool useNewVersion = true);
};



} // namespace LS_NIA 