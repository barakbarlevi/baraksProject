#include "Header.h"
// XXXX organize all code in most appriopriate manner
// XXXX go though all public / protected / private sections and decide on the best declarations.
//  XXXX    A FIGURE EXPLAINING THE LAYOUT: MAIN TRAJECTORY SENSOR, SUPPLIER, COLLOCTOR, ETC. can be done with screenshop from ge.    XXXX delete if not needed here

#include <iostream>
#include <fstream>
#include <string>
#include <sstream> // XXXX why so many streams......
#include <chrono>
#include <thread> // XXXX pthread !
#include <functional>
// #include <process.h>    // XXXX change?
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <mutex>
#include <memory>
#include <filesystem> // XXXX for the execution permissions. if moving elsewhere, move this too. in general, make order in headers cuz currently giant mess.
#include <pthread.h>

#include "PredictionSupplierCADAC.h"
#include "SensorTrajectoryCADAC.h"
#include "PredSuppTrajectoryCADAC.h" // XXXX see if can bring the two together, or must have the seperation as sensor and supplier.
#include "SuppliersCollector.h"
#include "utils.h"
#include <stdlib.h>
#include <string>
#include <sstream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

// XXXX from the stackoverflow most accepted answer on naming conventions:
//  class separated_by_underscores
/*
protected:
        // the more public it is, the more important it is,
        // so order: public on top, then protected then private


https://geosoft.no/development/cppstyle.html:
  void setDepth (int depth)
  {
    depth_ = depth;
  }

An issue is whether the underscore should be added as a prefix or as a suffix. Both practices are commonly used, but the latter is recommended because it seem to best preserve the readability of the name.
*/

std::mutex RunKML_mutex;      // XXXX name
std::mutex KML_editing_mutex; // IMPORTANT: Understand why the hell is this mutex needed? it was tough to say on the windows system and requires check here. XXXX
std::mutex ChangeFiles_mutex; // IMPORTANT: Understand why the hell is this mutex needed? it was tough to say on the windows system and requires check here. XXXX

std::mutex nameThisMutex;   // xxxx name this mutex

std::mutex gSyncOutputReading_mutex;
std::condition_variable gSyncOutputReading_cv;
bool gSyncOutputReading_ready = false;
bool loopMaintainer = true;  // xxxx needed?
bool input_read = false;
int readOnce = 0;

std::mutex mux;
std::condition_variable cv;  
bool completed[3]{ false, false, false };

void signalCompletionOfT1() {
        std::lock_guard<std::mutex> ul(mux);
        completed[0] = true;
        cv.notify_all();
    }

    void signalCompletionOfT2() {
        std::lock_guard<std::mutex> ul(mux);
        completed[0] = false;
        completed[1] = true;
        cv.notify_all();
    }

    void signalCompletionOfT3() {
        std::lock_guard<std::mutex> ul(mux);
        completed[0] = false;
        completed[1] = false;
        completed[2] = true;
        cv.notify_all();
    }

    void waitForCompletionOfT1() {
        std::unique_lock<std::mutex> ul(mux);             
        cv.wait(ul, [&]() {return !completed[2]; });         
    }

    void waitForCompletionOfT2() {
        std::unique_lock<std::mutex> ul(mux);             
        cv.wait(ul, [&]() {return !completed[0]; });         
    }

    void waitForCompletionOfT3() {
        std::unique_lock<std::mutex> ul(mux);         
        cv.wait(ul, [&]() {return !completed[1]; });           
    }      

// int g_data = 0;
// xxxx remove all unnecessary sleep()s
// xxxx check cadac for sleeps too
// xxxx "int" i ? ++i or i++ ? xxxx always size_t? maybe unsigned?
// xxxx what abouth factory design pattern? how can it be implemented in mojo?

int main(int argc, char *argv[])
{ 
    (void)argc;
    (void)argv;

    char* homeENV = getenv("HOME");
    std::stringstream ss;
    ss.str(homeENV);
    std::string home = ss.str();


    std::array<std::string, 2> detectedTrajectories = {home + "/Source_Files/CADAC/Custom/Version7/Vandenberg_az180",
                                                       home + "/Source_Files/CADAC/Custom/Version7/Vandenberg_az90"}; // Absolute / relative paths of detection plots/tracks/simulation data. xxxx absolute path looks like crap FIX change to relative path    

    std::vector<predictionSuppliers> predictionSuppliers = {CADAC, CADAC, CADAC};

    std::vector<std::string> currentCollectorPriamryInputFiles = {home + "/Source_Files/CADAC/Custom/Version7/inputOriginal.asc",
                                                                  home + "/Source_Files/CADAC/Custom/Version7/input_Drag0p7.asc",
                                                                  home + "/Source_Files/CADAC/Custom/Version7/input_Drag1p3.asc"}; // Absolute / relative paths of detection plots/tracks/simulation data. xxxx absolute path looks like crap FIX change to relative path. xxxx should be a vector in order to allow predictors modification in flight xxxx english

    std::vector<std::string> currentCollectorExecutables = {home + "/Source_Files/CADAC/Custom/Version7/SIX_DOF",
                                                            home + "/Source_Files/CADAC/Custom/Version7/SIX_DOF",
                                                            home + "/Source_Files/CADAC/Custom/Version7/SIX_DOF"}; // Absolute / relative paths of detection plots/tracks/simulation data. xxxx absolute path looks like crap FIX change to relative path. xxxx should be a vector in order to allow predictors modification in flight xxxx english

    std::vector<std::string> currentCollectorLoadPaths = {home + "/Source_Files/CADAC/Custom/Version7/cadacOutput.asc",
                                                          home + "/Source_Files/CADAC/Custom/Version7/cadacOutput.asc",
                                                          home + "/Source_Files/CADAC/Custom/Version7/cadacOutput.asc"}; // Absolute / relative paths of detection plots/tracks/simulation data. xxxx absolute path looks like crap FIX change to relative path. xxxx should be a vector in order to allow predictors modification in flight xxxx english xxxx explain that this also is a vector for the same reasons

    utils::kmlInitPrimaryController();
    utils::kmlInitSecondaryController();

#if 0
    for (unsigned int i = 0; i < detectedTrajectories.size(); i++)
    { // XXXX what is the best decleration for i ? auto? so many others i've seen?

        std::string detectionKML = "Detection" + std::to_string(i) + ".kml";    // XXXX names... to_string... XXXX THIS IS A TRICKY LINE BECAUSE i should mention that anybody has their own path. plus i'm showing here mine... FIX
        SensorTrajectoryCADAC Sensor(detectedTrajectories.at(i), detectionKML); // XXXX 3xNotspecified? what are these, deleteable? xxxx explain why 1 etc.
        Sensor.readInput();                                                     // XXXX Names.. loadPath the best ? how about dataLoadPath?

        /* there's noise here. basically, sensor trajectories from the bank shouldn't have different colors.
           when trying to properly write the pthreads implementation, it's important because
           class methods may beed parameters passed and if u can store everything u need for the plotting inside
           the class as members, then u probably can use this-> and don't need an assistant struct.
           so continuing with the second part of main, then will pass everything to pthreads.
        */

        std::thread THREAD = Sensor.threadpPlotTrajectoryAtOnce(5, 0, 0, 0); // XXXX THREAD name differently
        THREAD.join();                                                        // XXXX THREAD name differently and move to POSIX

        // pthread_t t;
        // pthread_create(&t, NULL, &Trajectory::hello_helper , &Sensor);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // xxxx why is this needed..?
    }

#endif

#if 1

    // XXXX move ALL declerations up
    std::string detectionKML = "Detection.kml";        // XXXX
    std::vector<std::string> collectorsKMLs_List = {}; // xxxx not sure about this line. CHECK WITH WORK.

    SensorTrajectoryCADAC SensorTrajectoryCADAC(detectedTrajectories.at(0), detectionKML); // XXXX Make sure about position units.. km / m... write in nice comment..
    SensorTrajectoryCADAC.readInput(true);
    //loopMaintainer = true; // xxxx
    std::thread pltThread = SensorTrajectoryCADAC.threadplotTrajectoryCoordByCoord(5, 200); // XXXX give better name. XXXX parameterize

#endif

#if 1

    std::cout << "Started plotting inspected trajectory\n" << std::endl; // XXXX better english xxxx google earth has to be opened with primarykml loaded

    /* // XXXX Operate / do something when reaching a specific height on ascent:  */

    float H_detection(8000); // XXXX best way to init ints / floats? they have a name... with dobule, long etc.. these types xxxx units - meter ?

    // xxxx in this case we are operating when reaching a certain height. on its face, we could have passed h_detection to threadplotTrajectoryCoordByCoord(5, 200, H),
    // assign bita_params.height there, and wait for reaching it. but what if we want to operate based on any other feature of bita_params? pass that too? so i decided
    // that i do want to update bita params every time i can until reaching that condition. or maybe, put a condition in there that is changeable.
    // and in main, it will only look like conditionIsMet, waiting for a notification on it. but then i would be forced to check on it every time even after
    // it's met! and it's a waste. so maybe i do want a seperate thread updating BITA but in a way that currentrowindex is safe xxxx write all of these considerations
    // in order to explain to the reader why i did what i did.
    // just a mutex doesn't let setbitaparams run. it doesn't get released in plottrahcoordbycoord.
    
    // xxxx could have used a semaphore instead?
    while ((std::stof(SensorTrajectoryCADAC._BITA_Params.height) < H_detection) && (SensorTrajectoryCADAC.get_vVertical() <= 0)) // XXXX height not a good name. nor vVertical. explain on coordinate system. XXXX nor H_detection. specify it's FIRST detect. be clear
    {
        // https://stackoverflow.com/questions/12551341/when-is-a-condition-variable-needed-isnt-a-mutex-enough
        // let's make this the consumer 1 xxxx

        std::unique_lock<std::mutex> ul(SensorTrajectoryCADAC.syncDetectSetBITA_mutex);
        SensorTrajectoryCADAC.syncDetectSetBITA_cv.wait(ul, [&](){ return SensorTrajectoryCADAC.syncDetectSetBITA_ready; }); // xxxx here i learned in the tough way the imprtance of capturing by reference and not by value ENGLISH. put it?

        SensorTrajectoryCADAC.setBITA_Params();                                            // xxxx i generally don't wanna wait till setting bita params is done when plotting. it's very important to plot in relevant time. what if setting up bita params will take a very long time? the target doesn't wait for me.
        std::cout << "height: " << SensorTrajectoryCADAC._BITA_Params.height << std::endl; // XXXX can't keep "height". change.

        SensorTrajectoryCADAC.syncDetectSetBITA_ready = false;
        ul.unlock();

        SensorTrajectoryCADAC.syncDetectSetBITA_cv.notify_one();
        ul.lock();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::cout << "Reached " << H_detection << "[m], at currentRowIndex: " << SensorTrajectoryCADAC.currentRowIndex << std::endl; // XXXX choose what to cout, only necessary.
    SensorTrajectoryCADAC.reachedHdetection = true;

#endif

#if 1
    int suppliersCollectorsVectorIndex(0);                                      // XXXX best init way ? bring up.
    std::vector<std::shared_ptr<SuppliersCollector>> suppliersCollectorsVector; // XXXX SuppliersCollectorsVector is this the right name? plural / singular wise. XXXX bring up. xxxx should it be a vector or is it constant size? go through all vector definitions in the code and decide.

    while (SensorTrajectoryCADAC.currentRowIndex <= SensorTrajectoryCADAC.data.size()) // as long as ballistic motion still takes place. loop on collectors xxxx english
    {
        std::shared_ptr<SuppliersCollector> currentCollector = std::make_shared<SuppliersCollector>(suppliersCollectorsVectorIndex);
        suppliersCollectorsVector.push_back(currentCollector);

        currentCollector->collectorKML_ = "Collector" + std::to_string(suppliersCollectorsVectorIndex) + ".kml";

        for (size_t i = 0; i < currentCollectorPriamryInputFiles.size(); ++i) // loop on number of current suppliers for the current collector xxxx english
        {                                                                     

            switch (predictionSuppliers.at(i))
            {
                case CADAC:
                {
                    std::shared_ptr<PredictionSupplierCADAC> predictionSupplierCADAC = std::make_shared<PredictionSupplierCADAC>(currentCollectorExecutables.at(i), currentCollectorPriamryInputFiles.at(i)); // xxxx this is the point where polymorphism has to be reduced to monomorphism ENGLISH. need to understand what kind of derived class dealing with, because can't use the pure virtual functions of PredictionSupplier.h. should mention how to create a distinguishment between different types of suppliers, cause now it's not really given. maybe have seperate intervals of i for each simulation. for instance: i=0 to 3 is CADAC. i = 4 to 7 is something else. i = 8 to 10 is third etc.

                    // xxxx Leave this here in comment so ppl will know that it exists and what's its purpose
                    // std::thread T0 = unq_ptr_to_PredictionSupplierCADAC->threadPrepareInputFiles();   // XXXX wrinte as a comment: this would have been the place to any supplementary input files editing. for example: change simulation dt or any other type of simulation-depended necessary editing. xxxx english. i don't need it right now so i'm not implementing it. // XXXX name T0 to indicative. POSIX
                    
                    

                    // Threaded version, delete if not needed xxxx
                    //std::thread T1 = predictionSupplierCADAC->threadupdateBITA_ParamsInSupplierInput(SensorTrajectoryCADAC.getBITA_Params()); // XXXX names XXXX POSIX xxxx mention: way of updating bita params in input files may vary.
                    //T1.join();                                                                                                            // XXXX name XXXX POSIX                

                    

                    /*******************************************
                    int first, second, third;

                    first = predictionSupplierCADAC->updateBITA_ParamsInSupplierInput(SensorTrajectoryCADAC.getBITA_Params()); // XXXX names XXXX POSIX xxxx mention: way of updating bita params in input files may vary.
                    first ++;
                                                                                                                  // XXXX name XXXX POSIX                
                    second = predictionSupplierCADAC->runSupplierOnce(); // XXXX POSIX XXXX names xxxx no matter what, this will have to include process sych. cuz on the one hand displaying radar data and on the other hand running simulation and plotting it. i think it's a good first step to let it run on a seperate thread. xxxx mention: way of running once may vary.
                    second = first + 1;

                    predictionSupplierCADAC->trajectoryCADAC = std::make_shared<PredSuppTrajectoryCADAC>(currentCollectorLoadPaths.at(i), currentCollector->collectorKML_);
                    
                    third = second;
                    third = predictionSupplierCADAC->trajectoryCADAC->readInput(false);
                    third = second + 1;                    

                    // Had the lines above not been synchronized in some way, a situation where the same input file is read would occur from time to time.
                    // A visualization of this is given in the document, where one could see trajectories completely overlap, since they're identicle.
                    // They could be two subsequent trajectories inside the same supplier, or within two suppliers, one being the last of the preceeding supplier
                    // and the other one being the first of the succeeding supplier.
                    // I had a hard time trying to understand why this synchronization is needed. Presumably, the method running the supplier once should block the current program
                    // from executing the line for input reading, since the method uses std::system() which is blocking. I tweeked the code a bit and witness it does block.
                    // I first assumed that if the call to read input only happens after the trajectory simulation had ended and generated a new output file, there should be no problem.
                    // When debugging, I saw that the overlapping trajectories contain the exact same data, which means my assumption above wasn't right.
                    // I also tried to rule out sequential inconsistency due to debugger optimization by using the functions return values.
                    // I assumed sequential consistency that didn't exist.
                    // My estimation of this situation is based on this post about sequential consistency https://stackoverflow.com/questions/38425920/memory-model-in-c-sequential-consistency-and-atomicity 
                    // and specifically the statement that: "If you just use plain non-atomic operations, or relaxed atomics, and no mutexes, then sequential consistency is not guaranteed."                
                    // At first I tried to run the lines as threads and use join() on the in a sequential order. That performed exactly like not threadding at all !
                    // It's in my assumption that the structure of joinning a thread right after instantiating it is meanningless and interperted as if it was written
                    // in exactly in the above form. 
                    // I then tried to overcome the overlapping trajectories issue by synchronizing the simulation run, which ends in an output file being written,
                    // and its reading. I think i got it right using a conditional variable, see https://stackoverflow.com/questions/78794995/synchronizing-simulation-output-generation-and-reading-using-producer-and-consum?noredirect=1#comment138922877_78794995:
                    
                    std::thread T1 = predictionSupplierCADAC->threadupdateBITA_ParamsInSupplierInput(SensorTrajectoryCADAC.getBITA_Params()); // XXXX names XXXX POSIX xxxx mention: way of updating bita params in input files may vary.
                    T1.join();

                    std::thread T2 = predictionSupplierCADAC->threadRunSupplierOnce();
                    
                    predictionSupplierCADAC->trajectoryCADAC = std::make_shared<PredSuppTrajectoryCADAC>(currentCollectorLoadPaths.at(i), currentCollector->collectorKML_);

                    std::thread T3 = predictionSupplierCADAC->trajectoryCADAC->threadReadInput(false);
       
                    T2.join();

                    T3.join();

                    // After this, there were still overlapping trajectories, though they seemed less likely to occur. I assumed there's still an issue
                    // with synching the BITA parameters writing to the input file. So I tried to synchronize all three threads to basically run in
                    // concurrency. This didn't seem logical to me to have threads that I force to run one after the other, because it misses the whole
                    // idea of threading, letting them run in parallel. On the other hand though, no other solution seemed to work as I still got
                    // overlaps.


                    // If 3 way synchronizing works: I'm not sure synchronization overhead in this solution is preferred over a sleep() between threads. xxxx if sleeps even solves it.
                    *******************************************/
                    
                    /*
                    std::thread T2 = predictionSupplierCADAC->threadRunSupplierOnce(); // XXXX POSIX XXXX names xxxx no matter what, this will have to include process sych. cuz on the one hand displaying radar data and on the other hand running simulation and plotting it. i think it's a good first step to let it run on a seperate thread. xxxx mention: way of running once may vary.
                    predictionSupplierCADAC->trajectoryCADAC = std::make_shared<PredSuppTrajectoryCADAC>(currentCollectorLoadPaths.at(i), currentCollector->collectorKML_);
                    std::thread T3 = predictionSupplierCADAC->trajectoryCADAC->threadReadInput(false);                        
                    T2.join();
                    T3.join();
                    */

                    std::thread t1 = predictionSupplierCADAC->threadupdateBITA_ParamsInSupplierInput(SensorTrajectoryCADAC.getBITA_Params());
                    std::thread t2 = predictionSupplierCADAC->threadRunSupplierOnce();
                    predictionSupplierCADAC->trajectoryCADAC = std::make_shared<PredSuppTrajectoryCADAC>(currentCollectorLoadPaths.at(i), currentCollector->collectorKML_);
                    std::thread t3 = predictionSupplierCADAC->trajectoryCADAC->threadReadInput(false);
                    t1.join();
                    t2.join();
                    t3.join();
                        


                    suppliersCollectorsVector.at(suppliersCollectorsVectorIndex)->suppliersVector.push_back(predictionSupplierCADAC->trajectoryCADAC); // XXXX
                }
            }

            suppliersCollectorsVector.at(suppliersCollectorsVectorIndex)->currentNumOfSuppliers++;
        }

        suppliersCollectorsVector.at(suppliersCollectorsVectorIndex)->plotCollectorAtOnce(2); // xxxx parameterize xxxx try lock_guard with this and the two above in the loop to prevent from plotting 2 at the same time
        suppliersCollectorsVectorIndex++; // XXXX leave in comment? do something else?
        SensorTrajectoryCADAC.setBITA_Params();
    }

    pltThread.join(); // XXXX posix XXXX name XXXX explain why here, explain also for all pthreads.













    std::this_thread::sleep_for(std::chrono::seconds(1)); // XXXX why is this here?

    //  XXXX Operate / do something when reaching a specific detect time value

    // while(std::stof(SensorTrajectoryCADAC.getBITA_Params().tbal < 5)) {   // XXXX fix 5 XXXX fix names
    //     std::cout << "tbal: " << SensorTrajectoryCADAC.getBITA_Params().tbal << std::endl;    // XXXX fix tbal XXXX maybe omit entirely the cout
    //     SensorTrajectoryCADAC.setBITA_Params();
    // }

    // std::cout << "Reached detect time t = 5 [sec]" << std::endl;    // XXXX fix 5, fix english if needed.

    //return 0;

#endif

    return 0;
}