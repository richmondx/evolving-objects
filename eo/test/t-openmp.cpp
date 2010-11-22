//-----------------------------------------------------------------------------
// t-openmp.cpp
//-----------------------------------------------------------------------------

#include <fstream>
#include <sstream>
#include <climits>

#include <utils/eoLogger.h>
#include <utils/eoParserLogger.h>

#include <eo>
#include <es/make_real.h>

#include <apply.h>
#include <omp_apply.h>

#include <omp.h>

#include "real_value.h"

//-----------------------------------------------------------------------------

typedef eoReal< eoMinimizingFitness > EOT;

//-----------------------------------------------------------------------------

int main(int ac, char** av)
{
    eoParserLogger parser(ac, av);

    unsigned int popMin = parser.getORcreateParam((unsigned int)1, "popMin", "Population Min", 'p', "Evolution Engine").value();
    unsigned int popStep = parser.getORcreateParam((unsigned int)1, "popStep", "Population Step", 0, "Evolution Engine").value();
    unsigned int popMax = parser.getORcreateParam((unsigned int)100, "popMax", "Population Max", 'P', "Evolution Engine").value();

    unsigned int dimMin = parser.getORcreateParam((unsigned int)1, "dimMin", "Dimension Min", 'd', "Evolution Engine").value();
    unsigned int dimStep = parser.getORcreateParam((unsigned int)1, "dimStep", "Dimension Step", 0, "Evolution Engine").value();
    unsigned int dimMax = parser.getORcreateParam((unsigned int)100, "dimMax", "Dimension Max", 'D', "Evolution Engine").value();

    unsigned int nRun = parser.getORcreateParam((unsigned int)100, "nRun", "Number of runs", 'r', "Evolution Engine").value();

    double threshold = parser.getORcreateParam((double)3.0, "threshold", "Threshold of max speedup", 0, "Evolution Engine").value();

    std::string speedupFileName = parser.getORcreateParam(std::string("speedup"), "speedupFileName", "Speedup file name", 0, "Results").value();
    std::string efficiencyFileName = parser.getORcreateParam(std::string("efficiency"), "efficiencyFileName", "Efficiency file name", 0, "Results").value();

    uint32_t seedParam = parser.getORcreateParam((uint32_t)0, "seed", "Random number seed", 0).value();
    if (seedParam == 0) { seedParam = time(0); }

    if (parser.userNeedsHelp())
	{
	    parser.printHelp(std::cout);
	    exit(1);
	}

    make_help(parser);
    make_verbose(parser);

    rng.reseed( seedParam );

    eoEvalFuncPtr< EOT, double, const std::vector< double >& > mainEval( real_value );
    eoEvalFuncCounter<EOT> eval( mainEval );

    eoUniformGenerator< double > gen(-5, 5);

    std::ostringstream params;
    params << "-p" << popMin << "-P" << popMax << "-d" << dimMin << "-D" << dimMax << "-r" << nRun << "-t" << threshold << "-s" << seedParam;
    std::ofstream speedupFile( std::string( speedupFileName + params.str() ).c_str() );
    std::ofstream efficiencyFile( std::string( efficiencyFileName + params.str() ).c_str() );

    size_t nbtask = 1;
#pragma omp parallel
    {
	nbtask = omp_get_num_threads();
    }

    eo::log << eo::logging << "Nb task: " << nbtask << std::endl;

    for ( size_t p = popMin; p <= popMax; p += popStep )
	{
	    for ( size_t d = dimMin; d <= dimMax; d += dimStep )
	    	{
		    eo::log << eo::logging << p << 'x' << d << std::endl;

		    for ( size_t r = 0; r < nRun; ++r )
			{
			    eoInitFixedLength< EOT > init( d, gen );

			    double Ts;
			    double Tp;

			    // sequential scope
			    {
				eoPop< EOT > pop( p, init );
				double t1 = omp_get_wtime();
				apply< EOT >(eval, pop);
				double t2 = omp_get_wtime();
				Ts = t2 - t1;
			    }

			    // parallel scope
			    {
				eoPop< EOT > pop( p, init );
				double t1 = omp_get_wtime();
				omp_apply< EOT >(eval, pop);
				double t2 = omp_get_wtime();
				Tp = t2 - t1;
			    }

			    if ( ( Ts / Tp ) > threshold ) { continue; }

			    speedupFile << Ts / Tp << ' ';
			    efficiencyFile << Ts / ( nbtask * Tp );

			    eo::log << eo::debug;
			    eo::log << "Ts = " << Ts << std::endl;
			    eo::log << "Tp = " << Tp << std::endl;
			    eo::log << "S_p = " << Ts / Tp << std::endl;
			    eo::log << "E_p = " << Ts / ( nbtask * Tp ) << std::endl;
			} // end of runs

		    speedupFile << std::endl;
		    efficiencyFile << std::endl;

		} // end of dimension

	} // end of population

    return 0;
}

//-----------------------------------------------------------------------------