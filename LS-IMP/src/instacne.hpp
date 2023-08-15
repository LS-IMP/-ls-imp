#include "utils.hpp"
#include "formula.hpp"

namespace LS_NIA {
    
class Instance  {
protected:
    Int _usrCnt, _supplyCnt, _demandCnt;
    Map<string, Int> _usrMap;            // discretization usrID -> usrIndex
    Map<string, Int> _supplyMap;         // supplyID -> supplyIndex
    Map<string, Int> _demandMap;         // demandID -> demandIndex

    vector<Int> _demandValue;            // .at(demandIndex) = demandValue
    
    vector<vector<Int> > _usrSupply;     // .at(usrIndex).at(i) = ith supplyment of usr
    vector<vector<Int> > _usrDemand;     // .at(usrIndex).at(i) = index linked to demandTable by user with ith supplyment 
    // ??? Maybe stored a pair here
    vector<vector<Pair<Int, Int> > > _demand2US;     // .at(demandIndex).at(i) = <usrIndex, supplyIndex>
    
    Int _tableCnt;
    vector<vector<Int> > _demandTable;   // read from sample file, at(i) linked with usrDemand
                                        // demandTable.at(i).at(0) is the limit on .at(i).at(j) where j > 0

    Variable _varCnt;                    // used for genFormula
    Map<string, Variable> _varMap;       // map str to varID;
    // Map<Variable, string> strMap;       // map varID to str; for DEBUG
 
    void addUsr(string usrID);
    void addSupply(string supplyID);
    void addDemand(string demandID);

    inline bool haveUsr(string usrID)       const { return _usrMap.count(usrID) != 0; }
    inline bool haveSupply(string supplyID) const { return _supplyMap.count(supplyID) != 0;}
    inline bool haveDemand(string demandID) const { return _demandMap.count(demandID) != 0;}
    inline bool haveVar(string queryStr)    const { return _varMap.count(queryStr) != 0;}

    Int getUsrIndex(string usrID) const;
    Int getSupplyIndex(string supplyID) const;
    Int getDemandIndex(string demandID) const;
    // ??? Do wo need index -> id ?

    void     addVar(string queryStr);
    Variable getVar(Int usrID, Int supplyID, Int demandID);
    bool     haveVar(Int usrID, Int supplyID, Int demandID) const;

    void genUsrSupplyFormula(NIA_Formula& formula);
    void genDemandLimitFormula(NIA_Formula& formula, Int queryIndex);
    void genMutliLinearFormula(NIA_Formula& formula);
    void genObjectiveFunction(NIA_Formula& formula, Int queryDemandIndex);

    void displayDemand2US() const;      // for DEBUG
public:

    void readDemandFile(string filePath);
    void readSampleFile(string filePath);

    Instance() : _usrCnt(0), _supplyCnt(0), _demandCnt(0), _tableCnt(0) {}

    NIA_Formula genFormula();

    const static Int undef;
};



} // namespace LS_NIA
