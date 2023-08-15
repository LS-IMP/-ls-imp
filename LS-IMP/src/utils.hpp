#pragma once

/* inclusions *****************************************************************/

#include <assert.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

/* uses ***********************************************************************/

using std::ostream;
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;

/* types **********************************************************************/

using Float = long double;   // std::stold
using Int   = int_fast32_t;  // std::stoi
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

template <typename K, typename V>
using Map = std::unordered_map<K, V>;
template <typename T>
using Set = std::unordered_set<T>;
template <typename T1, typename T2>
using Pair = std::pair<T1, T2>;

/* global variables ***********************************************************/

extern Int randomSeed;
extern TimePoint startTime;

extern std::mt19937 genRandom;

/* debug flags ****************************************************************/
extern const bool DEBUG;
extern const bool JUDGE;

/* constants ******************************************************************/
extern const string COMMENT_WORD;

extern const Int DEFAULT_RANDOM_SEED;

extern const Float NEGATIVE_INFINITY;

extern const Int DUMMY_MIN_INT;
extern const Int DUMMY_MAX_INT;

extern const string DUMMY_STR;

extern const Int READ_RATE;         // READ_RATE %
extern const Int DEMAND_DIVISOR;    // DEMAND / DIVISOR
extern const Int NIA_NUM;           // NIA constraint num;


/* namespaces *****************************************************************/

namespace util {
bool isInt(Float d);

void setRandom(Int seed);
void testRandom();

/* functions: printing ******************************************************/

void printComment(const string& message, Int preceedingNewLines = 0, Int followingNewLines = 1, bool commented = true);
void printBoldLine(bool commented);
void printThickLine(bool commented = true);
void printThinLine();

void printHelpOption();

/* functions: argument parsing **********************************************/
vector<string> getArgV(int argc, char* argv[]);
vector<string> splitStr(const string str, char splitFlag = ' ');

/* functions: timing ********************************************************/

TimePoint getTimePoint();
Float getSeconds(TimePoint startTime);
void printDuration(TimePoint startTime);

/* functions: error handling ************************************************/

void showWarning(const string& message, bool commented = true);
void showError(const string& message, bool commented = true);

/* functions: templates implemented in headers to avoid linker errors *******/

template <typename T>
void printRow(const string& name, const T& value) {
    cout << std::left << std::setw(30) << name;
    cout << std::left << std::setw(15) << value << "\n";
}

template <typename T>
void printContainer(const T& container) {
    cout << "printContainer:\n";
    for (const auto& member : container) {
        cout << "\t" << member;
    }
    cout << "\n";
}

template <typename K, typename V>
void printMap(const Map<K, V>& m) {
    cout << "printMap:\n";
    for (const auto& kv : m) {
        cout << "\t" << kv.first << "\t:\t" << kv.second << "\n";
    }
    cout << "\n";
}

template <typename Key, typename Value>
bool isLessValued(std::pair<Key, Value> a, std::pair<Key, Value> b) {
    return a.second < b.second;
}

template <typename T>
T getSoleMember(const vector<T>& v) {
    if (v.size() != 1) showError("vector is not singleton");
    return v.front();
}

template <typename T>
void popBack(T& element, vector<T>& v) {
    if (v.empty()) {
        showError("vector is empty");
    }
    element = v.back();
    v.pop_back();
}

template <typename T>
void invert(T& t) {
    std::reverse(t.begin(), t.end());
}

template <typename T, typename U>
bool isFound(const T& element, const U& container) {
    return std::find(std::begin(container), std::end(container), element) != std::end(container);
}

template <typename T, typename U1, typename U2>
void differ(Set<T>& diff, const U1& members, const U2& nonmembers) {
    for (const auto& member : members) {
        if (!isFound(member, nonmembers)) {
            diff.insert(member);
        }
    }
}

template <typename T, typename U>
void unionize(Set<T>& unionSet, const U& container) {
    for (const auto& member : container)
        unionSet.insert(member);
}

template <typename T, typename U>
bool isDisjoint(const T& container, const U& container2) {
    for (const auto& member : container) {
        for (const auto& member2 : container2) {
            if (member == member2) {
                return false;
            }
        }
    }
    return true;
}

template <typename T>
void shuffleRandomly(T& container) {
    std::mt19937 generator;
    generator.seed(randomSeed);
    std::shuffle(container.begin(), container.end(), generator);
}

}  // namespace util

/* classes ********************************************************************/

class MyError {
   public:
    MyError(const string& message, bool commented);
};
