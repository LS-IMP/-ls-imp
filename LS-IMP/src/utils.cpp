/* inclusions *****************************************************************/

#include "utils.hpp"

/* debug flags ****************************************************************/
const bool DEBUG = true;
const bool JUDGE = false;

/* constants ******************************************************************/
const string COMMENT_WORD = "c";

const Int DEFAULT_RANDOM_SEED = 9;

const Float NEGATIVE_INFINITY = -std::numeric_limits<Float>::infinity();

const Int DUMMY_MIN_INT = std::numeric_limits<Int>::min();
const Int DUMMY_MAX_INT = std::numeric_limits<Int>::max();

const string DUMMY_STR = "";

const Int READ_RATE = 10;           // 10%
const Int DEMAND_DIVISOR = 50000;   // demandValue / 5w
const Int NIA_NUM = 10;            // 100 3.7 constraint

/* global variables ***********************************************************/

Int randomSeed = DEFAULT_RANDOM_SEED;
TimePoint startTime;

std::mt19937 genRandom;

/* namespaces *****************************************************************/

/* namespace util *************************************************************/

// bool util::isInt(Float d) {
//     Float intPart;
//     Float fractionalPart = modf(d, &intPart);
//     return fractionalPart == 0.0;
// }

void util::setRandom(Int seed) {
    genRandom = std::mt19937(seed);
}

void util::testRandom() {
    Int times = 10;
    while (times --) {
        cout << genRandom() << endl;
    }
}

/* functions: printing ********************************************************/

void util::printComment(const string& message, Int preceedingNewLines, Int followingNewLines, bool commented) {
    for (Int i = 0; i < preceedingNewLines; i++)
        cout << "\n";
    cout << (commented ? COMMENT_WORD + " " : "") << message;
    for (Int i = 0; i < followingNewLines; i++)
        cout << "\n";
}

void util::printBoldLine(bool commented) {
    printComment("******************************************************************", 0, 1, commented);
}

void util::printThickLine(bool commented) {
    printComment("==================================================================", 0, 1, commented);
}

void util::printThinLine() {
    printComment("------------------------------------------------------------------");
}

void util::printHelpOption() {
    cout << "  -h, --help    help information\n";
}
/* functions: argument parsing ************************************************/

vector<string> util::getArgV(int argc, char* argv[]) {
    vector<string> argV;
    for (Int i = 0; i < argc; i++)
        argV.push_back(string(argv[i]));
    return argV;
}

vector<string> util::splitStr(const string str, char splitFlag) {
    vector<string> res;
    Int st = 0;
    for (Int i = 0, size = str.size(); i < size; i++) {
        if (str.at(i) == splitFlag) {
            res.push_back(str.substr(st, i - st));
            st = i + 1;
        }
    }
    if (str.size() - st > 0) {
        res.push_back(str.substr(st, str.size() - st));
    }
    return res;
}

/* functions: timing **********************************************************/

TimePoint util::getTimePoint() {
    return std::chrono::steady_clock::now();
}

Float util::getSeconds(TimePoint startTime) {
    TimePoint endTime = getTimePoint();
    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
}

void util::printDuration(TimePoint startTime) {
    printThickLine();
    printRow("seconds", getSeconds(startTime));
    printThickLine();
}

/* functions: error handling **************************************************/

void util::showWarning(const string& message, bool commented) {
    printBoldLine(commented);
    printComment("MY_WARNING: " + message, 0, 1, commented);
    printBoldLine(commented);
}

void util::showError(const string& message, bool commented) {
    throw MyError(message, commented);
}

/* classes ********************************************************************/

/* class MyError **************************************************************/

MyError::MyError(const string& message, bool commented) {
    util::printBoldLine(commented);
    util::printComment("MY_ERROR: " + message, 0, 1, commented);
    util::printBoldLine(commented);
}
