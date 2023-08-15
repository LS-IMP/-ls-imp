#include "instacne.hpp"
#include "lsearch.hpp"
#include <cstring>

bool readLpFile = false;
bool useNewVersion = true;

int main(int argc, char** argv) {
    util::setRandom(DEFAULT_RANDOM_SEED);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--lp") == 0) {
            readLpFile = true;
        }
        if (strcmp(argv[i], "--nv") == 0) {
            useNewVersion = true;
        }
        if (strcmp(argv[i], "--ov") == 0) {
            useNewVersion = false;
        }
    }

    if (readLpFile) {
        LS_NIA::NIA_Formula formula = LS_NIA::lpReader::readLpFile(argv[argc - 1]);

        startTime = util::getTimePoint();
        LS_NIA::LsSolver solver(formula);

        solver.solve(useNewVersion);
    } else {
        LS_NIA::Instance instance;
        
        instance.readDemandFile(argv[argc - 2]);
        instance.readSampleFile(argv[argc - 1]);

        LS_NIA::NIA_Formula formula = instance.genFormula();

        startTime = util::getTimePoint();
        LS_NIA::LsSolver solver(formula);

        solver.solve(useNewVersion);
    }

   
}