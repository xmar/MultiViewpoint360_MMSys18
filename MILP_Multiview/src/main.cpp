/**********************************                                              
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Solve the optimal model for the multi-view scenario                                      
 */

#include <boost/program_options.hpp>                                             
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
  #include <boost/archive/binary_oarchive.hpp>
  #include <boost/archive/binary_iarchive.hpp>
  #define INPUT_ARCHIVE(variableName, inputFile) boost::archive::binary_iarchive variableName(inputFile);
  #define OUTPUT_ARCHIVE(variableName, outputFile) boost::archive::binary_oarchive variableName(outputFile);


#include "OptimalScenario.hpp"
#include "OptimalHorizontalScenario.hpp"
#include "OptimalVerticalScenario.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <chrono>

#define DEBUG 0
#if DEBUG
#define PRINT_DEBUG(x) std::cout << x << std::endl;
#else
#define PRINT_DEBUG(x) {}
#endif // DEBUG

using namespace IMT;

int main( int argc, const char* argv[] )
{
    namespace po = boost::program_options;
    namespace pt = boost::property_tree;
    po::options_description desc("Options");
    desc.add_options()
       ("help,h", "Produce this help message")
       //("inputVideo,i", po::value<std::string>(), "path to the input video")
       ("config,c", po::value<std::string>(),"Path to the configuration file")
       ;
    
    po::variables_map vm;
 
    try {
        po::store(po::parse_command_line(argc, argv, desc),
              vm);
        
        //--help
        if ( vm.count("help") || !vm.count("config"))
        {
           std::cout << "Help: preprocessing -c config"<< std::endl
              <<  desc << std::endl;
           return 0;
        }
        
        po::notify(vm);
        
        //Get the path to the configuration file
        std::string pathToIni = vm["config"].as<std::string>();
        
        std::cout << "Path to the ini file: " <<pathToIni << std::endl;
        
        //read the ini file the feed the GlobalArgsStructure
        pt::ptree ptree;
        pt::ini_parser::read_ini(pathToIni, ptree);
        
        int nbViewpoint = ptree.get<int>("Global.NbViewpoint");
        float optimalGap = ptree.get<float>("Global.optimalGap");
        int nbThread = ptree.get<int>("Global.nbThread");
        int nbScenario = ptree.get<int>("Global.NbScenario");

        SolutionWritter sw("outSol/Test");
        double averageDuration = 0;
        for (int scenarioId = 0; scenarioId < nbScenario; ++scenarioId)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            std::string scenarioName = ptree.get<std::string>("Global.Scenario"+std::to_string(scenarioId));
            std::cout << "Prepare Scenario " << scenarioName << std::endl;
            int nbTile = ptree.get<int>(scenarioName+".NbTile");
            int nbQuality = ptree.get<int>(scenarioName+".NbQuality");
            int nbChunk = ptree.get<int>(scenarioName+".NbChunk");
            int nbProcessedChunk = ptree.get<int>(scenarioName+".NbProcessedChunk");
            int nbLagChunk = ptree.get<int>(scenarioName+".NbLagDownloadChunk");
            std::string adaptationSetConf = ptree.get<std::string>(scenarioName+".AdaptationSetConf");
            std::string bandwidthConf = ptree.get<std::string>(scenarioName+".BandwidthConf");
            std::vector<std::string> bandwidthVect;
            std::istringstream ss(bandwidthConf);
            std::string val;
            while (std::getline(ss, val, ';'))
            {
                bandwidthVect.emplace_back(std::move(val));
            }
            std::string userConf = ptree.get<std::string>(scenarioName+".UserConf");
            std::vector<std::string> userVect;
            ss = std::istringstream(userConf);
            while (std::getline(ss, val, ';'))                                   
            {                                                                    
                userVect.emplace_back(std::move(val));                      
            }
            float avgBandwidth = ptree.get<float>(scenarioName+".avgBandwidth");

            std::string optScenarioName("outSave/optimalScenario_");
            std::string optHorizontalScenarioName("outSave/optimalHorizontalScenario_");
            std::string optVerticalScenarioName("outSave/optimalVerticalScenario_");

            for (SolutionType solType: v_SolutionType)
            {

                    bool scenarioStored = false;
                std::shared_ptr<Scenario> scenario(nullptr);
                std::string scenarioType("");
                std::string scenarioStoredPath("");
                std::string scenarioStoredId("");
                bool runScenario = false;
                switch (solType)
                {
                    case SolutionType::Optimal:
                        scenario = std::make_shared<OptimalScenario>(userVect.size(), bandwidthVect.size(), nbChunk, nbProcessedChunk, nbLagChunk, nbViewpoint, nbTile, nbQuality, optimalGap, nbThread);
                        scenarioStoredId = optScenarioName;
            			scenarioStoredPath = optScenarioName+scenarioName+".save.bin.gz";
                        scenarioType = "Optimal";
                        runScenario = ptree.get<bool>(scenarioName+".optimal");
                        break;
                    case SolutionType::Vertical:
                        scenarioStoredPath = optVerticalScenarioName+scenarioName+".save.bin.gz";
                        scenarioStoredId = optVerticalScenarioName;
                        scenario = std::make_shared<OptimalVerticalScenario>(userVect.size(), bandwidthVect.size(), nbChunk, nbProcessedChunk, nbLagChunk, nbViewpoint, nbTile, nbQuality, optimalGap, nbThread);
                        scenarioType = "Vertical";
                        runScenario = ptree.get<bool>(scenarioName+".verticalOptimal");
                        break;
                    case SolutionType::Horizontal:
                        scenarioStoredPath = optHorizontalScenarioName+scenarioName+".save.bin.gz";
                        scenarioStoredId = optHorizontalScenarioName;
                        scenario = std::make_shared<OptimalHorizontalScenario>(userVect.size(), bandwidthVect.size(), nbChunk, nbProcessedChunk, nbLagChunk, nbViewpoint, nbTile, nbQuality, optimalGap, nbThread);
                        scenarioType = "Horizontal";
                        runScenario = ptree.get<bool>(scenarioName+".horizontalOptimal");
                        break;
                    default:
                        throw std::string("Scenario type error.");
                }
                scenarioStored = boost::filesystem::exists(scenarioStoredPath);
                    
                if (runScenario)
                {
                    std::string namePrintable ("\n\033[1;31m" + scenarioType + " Scenario: " + scenarioName + " " +std::to_string(100*float(scenarioId) / nbScenario) + "%" + "\033[0m\n");
                    std::cout << namePrintable << std::endl;
                    scenario->Init(adaptationSetConf, userVect, bandwidthVect);
                    if (scenarioStored)
                    {
                        std::ifstream ifs(scenarioStoredPath, std::ios_base::in | std::ios_base::binary);
                        boost::iostreams::filtering_istream in;                                
                        in.push(boost::iostreams::gzip_decompressor());                        
                        in.push(ifs);                                                          
                        INPUT_ARCHIVE (ia,in);                                                 
                        ia >> *scenario;
                    }
                    else
                    {
                        scenario->Run(scenarioStoredId+scenarioName,namePrintable, averageDuration/1000.f, scenarioId, nbScenario);
                        std::ofstream ofs(scenarioStoredPath, std::ios_base::out | std::ios_base::binary);
                        boost::iostreams::filtering_ostream out;                               
                        out.push( boost::iostreams::gzip_compressor());                        
                        out.push( ofs );                                                       
                        OUTPUT_ARCHIVE (oa,out);
                        oa << *scenario;
                    }
                    Solution sol = scenario->PrintResults();
                    MetaData meta(solType, nbTile, nbViewpoint, nbChunk, nbLagChunk, nbQuality, optimalGap, avgBandwidth);
                    sw.AddSolution(std::move(meta), std::move(sol));
                    sw.Write();
                }
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
                averageDuration = (averageDuration*scenarioId + duration)/(scenarioId+1);
                std::cout << "Total spent time: " << print_time<long>(averageDuration*(scenarioId+1)/1000.f) << std::endl;
            }

            std::cout << scenarioName <<  " done" << std::endl;
        }

        sw.Write();
 
        std::cout << "DONE" << std::endl;
    }
    catch (std::string s) {std::cerr << "Exception catched: " << s << std::endl;}
    return 0;
}
