#include "instacne.hpp"

namespace LS_NIA {

const Int Instance::undef = -1;

void Instance::addUsr(string usrID) {
    assert(_usrMap.count(usrID) == 0);
    _usrMap.insert(std::make_pair(usrID, _usrCnt++));
    if (DEBUG && false) {
        cout << "Add Usr: " << usrID << " -> " << getUsrIndex(usrID) << endl; 
    }
    
    _usrSupply.push_back(vector<Int>());
    assert(_usrSupply.size() == _usrCnt);

    _usrDemand.push_back(vector<Int>());
    assert(_usrDemand.size() == _usrCnt);
}

void Instance::addSupply(string supplyID) {
    assert(_supplyMap.count(supplyID) == 0);
    _supplyMap.insert(std::make_pair(supplyID, _supplyCnt++));
    if (DEBUG && false) {
        cout << "Add Supply: " << supplyID << " -> " << getSupplyIndex(supplyID) << endl; 
    }
}

void Instance::addDemand(string demandID) {
    assert(_demandMap.count(demandID) == 0);
    _demandMap.insert(std::make_pair(demandID, _demandCnt++));
    if (DEBUG && false) {
        cout << "Add Demand: " << demandID << " -> " << getDemandIndex(demandID) << endl;
    }
}

Int Instance::getUsrIndex(string usrID) const {
    if (haveUsr(usrID)) return _usrMap.at(usrID);
    else return undef;
}

Int Instance::getSupplyIndex(string supplyID) const {
    if (haveSupply(supplyID)) return _supplyMap.at(supplyID);
    else return undef;
}

Int Instance::getDemandIndex(string demandID) const {
    if (haveDemand(demandID)) return _demandMap.at(demandID);
    else return undef;
}

void Instance::readDemandFile(string filePath) {
    std::ifstream fileStream(filePath);
    string line;
    assert(_demandCnt == 0);
    while (getline(fileStream, line)) {
        // cout << "line: " << line << endl;
        vector<string> demandData = util::splitStr(line, '`');
        assert(demandData.size() == 2);
        string demandID = demandData.at(0);
        Int demandV = std::stoi(demandData.at(1));

        if (!haveDemand(demandID)) addDemand(demandID);
        else util::showError("Duplicate demandID " + demandData.at(0));

        _demandValue.push_back(demandV / DEMAND_DIVISOR);    // 5w original
        assert(_demandValue.size() == _demandCnt);
    }
    fileStream.close();
}

void Instance::readSampleFile(string filePath) {
    std::ifstream fileStream(filePath);
    string line;
    assert(_usrCnt == 0 && _supplyCnt == 0);
    assert(_demandCnt > 0);
    _demand2US.resize(_demandCnt);    // afeter readDemandFile

    char splitChar = '-';

    while (getline(fileStream, line)) {
        if (genRandom() % 100 > READ_RATE) { // READ_RATE = 10  only read 10% original
            continue;
        }

        if (splitChar == '-') {
            if (line.find("`") != string::npos) splitChar = '`';
            if (line.find(";") != string::npos) splitChar = ';';
            assert(splitChar != '-');
        }


        vector<string> sampleData = util::splitStr(line, splitChar);
        assert(sampleData.size() == 4);
        string usrID = sampleData.at(0), supplyID = sampleData.at(1);
        
        if (!haveUsr(usrID))       addUsr(usrID);
        if (!haveSupply(supplyID)) addSupply(supplyID);

        Int usrIndex    = getUsrIndex(usrID);
        Int supplyIndex = getSupplyIndex(supplyID);

        if (util::isFound(supplyIndex, _usrSupply.at(usrIndex))) {
            util::showWarning("Duplicate UsrID: " + sampleData.at(0) + " and supplyID: " + sampleData.at(1)); 
        } 
        else {

            // <usrIndex, supplyIndex> -> _tableCnt
            _usrSupply.at(usrIndex).push_back(supplyIndex);
            _usrDemand.at(usrIndex).push_back(_tableCnt);

            assert(_usrSupply.at(usrIndex).size() == _usrDemand.at(usrIndex).size());
            
            Int supplyLimit = std::stoll(sampleData.at(2));
            vector<string> demandList = util::splitStr(sampleData.at(3), ',');
            vector<Int>    curDemand;

            curDemand.push_back(supplyLimit);
            Set<Int> visitedDemandIndex;
            for (string demandID : demandList) {
                if (!haveDemand(demandID)) {
                    // util::showWarning("DemandID: " + demandID + " haven't appear before !!!");
                    continue;
                }
                Int demandIndex = getDemandIndex(demandID);
                if (visitedDemandIndex.count(demandIndex) > 0) continue;
                visitedDemandIndex.insert(demandIndex);

                curDemand.push_back(demandIndex);
                _demand2US.at(demandIndex).push_back(std::make_pair(usrIndex, supplyIndex));        // ??? multi add
            }

            _demandTable.push_back(curDemand);
            _tableCnt++;
            assert(_tableCnt == _demandTable.size());
        }
    }

    if (DEBUG && false) displayDemand2US();
}

bool Instance::haveVar(Int usrIndex, Int supplyIndex, Int demandIndex) const {
    assert(usrIndex < _usrCnt);
    assert(supplyIndex < _supplyCnt);
    assert(demandIndex < _demandCnt);
    string queryStr = "U" + to_string(usrIndex) + "S" + to_string(supplyIndex) + "D" + to_string(demandIndex);
    return haveVar(queryStr);
}

void Instance::addVar(string queryStr) {
    if (JUDGE) assert(!haveVar(queryStr));
    // if (DEBUG) strMap.insert(std::make_pair(varCnt, queryStr)); 
    _varMap.insert(std::make_pair(queryStr, _varCnt));
    _varCnt++;
}

Variable Instance::getVar(Int usrIndex, Int supplyIndex, Int demandIndex) {
    assert(usrIndex < _usrCnt);
    assert(supplyIndex < _supplyCnt);
    assert(demandIndex < _demandCnt);
    string queryStr = "U" + to_string(usrIndex) + "S" + to_string(supplyIndex) + "D" + to_string(demandIndex);
    if (!haveVar(queryStr)) addVar(queryStr);
    return _varMap.at(queryStr);
}

/**
 * @brief User-Supply: sum of vars <= cast count
 * Note that: _usrSupply.at(usrIndex) stored supplyIndex
 *            _usrDemand.at(usrIndex) stored demandAddress in _demandTable
 * By use usrIndex and index number i can visit supplyIndex _usrSupply.at(usrIndex).at(i)
 *                                            demandAddress _usrDemand.at(usrIndex).at(i)
 *                                         and demandVector _demandTable.at(demandAddress)
 * In demandVector.at(0) stored cast number
 */
void Instance::genUsrSupplyFormula(NIA_Formula& formula) {
    for (Int usrIndex = 0; usrIndex < _usrCnt; usrIndex++) {  // usrIndex
        const vector<Int>& supplyVec = _usrSupply.at(usrIndex);
        for (Int supplySize = supplyVec.size(), i = 0; i < supplySize; i++) {
            Int supplyIndex = supplyVec.at(i);
            const vector<Int>& demandVec = _demandTable.at(_usrDemand.at(usrIndex).at(i));

            assert(demandVec.size() > 0);
            if (demandVec.size() == 1) continue;

            Int castNum = demandVec.at(0);

            // \sum coef * var <= carNum
            Polynomial poly;
            for (Int demandSize = demandVec.size(), j = 1; j < demandSize; j++) {   // from 1,  at(0) is castNum
                Int demandIndex = demandVec.at(j);

                Variable var = getVar(usrIndex, supplyIndex, demandIndex);
                poly += Monomial(var, 1);
            }
            formula.addConstraint(poly, Op::LEQUAL, castNum);

            if (DEBUG && true) cout << "UsrSupply:  " << poly << " <= " << castNum << endl;
        }

        formula.setVarCnt(_varCnt);
    }
}

/**
 * @brief Demand Limit: sum of demand supplied by usr >= demand need
 * 
 */
void Instance::genDemandLimitFormula(NIA_Formula& formula, Int queryDemandIndex) {
    for (Int demandIndex = 0; demandIndex < _demandCnt; demandIndex++) {
        if (demandIndex == queryDemandIndex) continue;

        const vector<Pair<Int, Int> >& demandVec = _demand2US.at(demandIndex);
        Polynomial poly;
        Int polySize = 0;
        for (Int demandSize = demandVec.size(), i = 0; i < demandSize; i++) {
            Int usrIndex    = demandVec.at(i).first;
            Int supplyIndex = demandVec.at(i).second;
            if (JUDGE) assert(haveVar(usrIndex, supplyIndex, demandIndex));
            poly += Monomial(getVar(usrIndex, supplyIndex, demandIndex), 1);
            polySize++;
        }
        // sum var >= _demandValue.at(demandIndex);
        Int limit = _demandValue.at(demandIndex);

        if (polySize < limit) {
            util::showWarning("DemandIndex " + to_string(demandIndex) + " can not sat dmeand ");
            continue;
        }

        formula.addConstraint(poly, Op::GEQUAL, limit);
        if (DEBUG && true) cout << "DemandLimit:  " << demandIndex << " " << poly << " >= " << limit << endl;
    }
    // ??? Need a faster visit, demandSupplied ?
}

/**
 * @brief 
 * 
 */
void Instance::genMutliLinearFormula(NIA_Formula& formula) {
    Int needConstraintNum = NIA_NUM;            // 100 3.7 constraint
    Int niaConsCnt = 0;
    
    while (needConstraintNum-- > 0) {
        Int querySupplyIndex  = genRandom() % _supplyCnt;
        Int queryDemandIndex1 = genRandom() % _demandCnt;
        Int queryDemandIndex2 = genRandom() % _demandCnt;
        while(queryDemandIndex1 == queryDemandIndex2) {
            queryDemandIndex2 = genRandom() % _demandCnt;
        }

        // poly[0]: sum {usrIndex},  supplyIndex,  demandIndex1
        // poly[1]: sum {usrIndex},  supplyIndex,  demandIndex2
        // poly[2]: sum {usrIndex}, {supplyIndex}, demandIndex1
        // poly[3]: sum {usrIndex}, {supplyIndex}, demandIndex2
        // poly[0] * poly[3] - poly[1] * poly[2] >= 0
        Polynomial poly[4];


        const vector<Pair<Int, Int> >& demandVec1 = _demand2US.at(queryDemandIndex1);
        const vector<Pair<Int, Int> >& demandVec2 = _demand2US.at(queryDemandIndex2);

        for (Int vecSize = demandVec1.size(), i = 0; i < vecSize; i++) {
            Int usrIndex    = demandVec1.at(i).first;
            Int supplyIndex = demandVec1.at(i).second;
            if (JUDGE) assert(haveVar(usrIndex, supplyIndex, queryDemandIndex1));

            poly[2] += Monomial(getVar(usrIndex, supplyIndex, queryDemandIndex1), 1);
            if (supplyIndex == querySupplyIndex)
                poly[0] += Monomial(getVar(usrIndex, supplyIndex, queryDemandIndex1), 1);
        }

        for (Int vecSize = demandVec2.size(), i = 0; i < vecSize; i++) {
            Int usrIndex    = demandVec2.at(i).first;
            Int supplyIndex = demandVec2.at(i).second;
            if (JUDGE) assert(haveVar(usrIndex, supplyIndex, queryDemandIndex2));

            poly[3] += Monomial(getVar(usrIndex, supplyIndex, queryDemandIndex2), 1);
            if (supplyIndex == querySupplyIndex)
                poly[1] += Monomial(getVar(usrIndex, supplyIndex, queryDemandIndex2), 1);
        }

        if (DEBUG && false) {
            cout << "p1: " << poly[0] << "\np2: " << poly[1] << "\np3: " << poly[2] << "\np4: " << poly[3] << endl;
            cout << "p1 * p4: " << (poly[0] * poly[3]) << endl;
            cout << "p2 * p3: " << (poly[1] * poly[2]) << endl;
            cout << "p1 * p4 - p2 * p3: " << (poly[0] * poly[3]) - (poly[1] * poly[2]) << endl;
        }

        Polynomial res = (poly[0] * poly[3]) - (poly[1] * poly[2]);

        formula.addConstraint(res, Op::GEQUAL, 0);
        niaConsCnt++;
        if (DEBUG && false) cout << "NIA:  (" << querySupplyIndex << ", " << queryDemandIndex1 << ", " << queryDemandIndex2 << ") "  
                                << res << " >= 0" << endl;
    }
    if (DEBUG && true) cout << "NIA constraint cnt: " << niaConsCnt << endl;
}

void Instance::genObjectiveFunction(NIA_Formula& formula, Int queryDemandIndex) {
    assert(queryDemandIndex < _demandCnt);

    Polynomial poly;
    const vector<Pair<Int, Int> >& demandVec = _demand2US.at(queryDemandIndex);

    for(Int vecSize = demandVec.size(), i = 0; i < vecSize; i++) {
        Int usrIndex    = demandVec.at(i).first;
        Int supplyIndex = demandVec.at(i).second;

        if (JUDGE) assert(haveVar(usrIndex, supplyIndex, queryDemandIndex));

        poly += Monomial(getVar(usrIndex, supplyIndex, queryDemandIndex), 1);
    }

    formula.addObjectiveFunction(-poly);

    if (DEBUG && true) cout << "Objective funciton: " << -poly << endl;
}

void Instance::displayDemand2US() const {
    for (Int demandIndex = 0; demandIndex < _demandCnt; demandIndex++) {
        cout << "DemandIndex: " << demandIndex;
        for (Pair<Int, Int> p : _demand2US.at(demandIndex)) {
            cout << " (" << p.first << ", " << p.second << ")  ";
        }
        cout << endl;
    }
}

NIA_Formula Instance::genFormula() {
    NIA_Formula res;
    cout << "Model problem\n";
    cout << "usr num | supply num | demand num" << endl;
    cout << _usrMap.size() << " | " << _supplyMap.size() << " | " << _demandMap.size() << endl;

    Int queryDemandIndex = genRandom() % _demandCnt;

    // Part I. Basic Constraints: Linear
    // 2.1 user-supply:     sum vars <= cast count
    genUsrSupplyFormula(res);

    // 2.2 demand:          sum vars >= demand value
    genDemandLimitFormula(res, queryDemandIndex);

    // 3.7 NIA
    genMutliLinearFormula(res);

    // objective function
    genObjectiveFunction(res, queryDemandIndex);

    // return std::move(res);
    return res;
}

} // namespace LS_NIA