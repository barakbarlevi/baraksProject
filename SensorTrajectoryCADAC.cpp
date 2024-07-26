#include "SensorTrajectoryCADAC.h"
#include "utils.h"

SensorTrajectoryCADAC::SensorTrajectoryCADAC(std::string loadPath, std::string kmlPath) : SensorTrajectory(loadPath, kmlPath) {
    this->FirstLineOfNumericData_ = 1;
    this->currentRowIndex = this->FirstLineOfNumericData_;
       
} // XXXX names, : chain, an so

void SensorTrajectoryCADAC::setBITA_Params()
{ // XXXX names.. make high level..
    
    //std::cout << "currentRowIndex: " << currentRowIndex << std::endl;

    /*
    this->_BITA_Params.tbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 1, 0); // XXXX check on func arguments to match cadac / general(?)
    // massbal: Cannot be detected by the sensor. It's added in he supplier's Update BITA routine, according to the supplier's models.  // XXXX VERIFY for example still calling it massbal ? go over this line.
    this->_BITA_Params.massbal = "0";
    this->_BITA_Params.lat = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 5, 1); // XXXX check on func arguments to match cadac / general(?)
    this->_BITA_Params.lon = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 4, 1); // XXXX check on func arguments to match cadac / general(?)
    this->_BITA_Params.height = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 6, 1);  // XXXX check on func arguments to match cadac / general(?)
    this->_BITA_Params.vbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 7, 1);  // XXXX check on func arguments to match cadac / general(?)
    // xxxx write that assuming alpha = 0 because can't sense it !
    this->_BITA_Params.gamalbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 3, 1);    // XXXX name gamalbal, XXXX to_string more modern way?  
    this->_BITA_Params.azimlbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 2, 1); // XXXX check on func arguments to match cadac / general(?) XXXX name azimlbal change
    */


    this->_BITA_Params.tbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 1, 0, currentRowIndex, "SensorTrajectoryCADAC::setBITA_Params 1"); // XXXX check on func arguments to match cadac / general(?)
    // massbal: Cannot be detected by the sensor. It's added in he supplier's Update BITA routine, according to the supplier's models.  // XXXX VERIFY for example still calling it massbal ? go over this line.
    this->_BITA_Params.massbal = "0";
    this->_BITA_Params.lat = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 5, 1, currentRowIndex, "SensorTrajectoryCADAC::setBITA_Params 2"); // XXXX check on func arguments to match cadac / general(?)
    this->_BITA_Params.lon = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 4, 1, currentRowIndex, "SensorTrajectoryCADAC::setBITA_Params 3"); // XXXX check on func arguments to match cadac / general(?)
    this->_BITA_Params.height = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 6, 1, currentRowIndex, "SensorTrajectoryCADAC::setBITA_Params 4");  // XXXX check on func arguments to match cadac / general(?)
    this->_BITA_Params.vbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 7, 1, currentRowIndex, "SensorTrajectoryCADAC::setBITA_Params 5");  // XXXX check on func arguments to match cadac / general(?)
    // xxxx write that assuming alpha = 0 because can't sense it !
    this->_BITA_Params.gamalbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 3, 1, currentRowIndex, "SensorTrajectoryCADAC::setBITA_Params 6");    // XXXX name gamalbal, XXXX to_string more modern way?  
    this->_BITA_Params.azimlbal = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 2, 1, currentRowIndex, "SensorTrajectoryCADAC::setBITA_Params 7"); // XXXX check on func arguments to match cadac / general(?) XXXX name azimlbal change

}

/*
int SensorTrajectoryCADAC::PlotTrajectoryCoordByCoord(int indexJump)
{                                                      // XXXX indexJump change name ? something more indicative ?
    std::cout << "Starting to plot main trajectory\n"; // XXXX not sure that "Starting to plot main trajectory" is exactly what should be written, if at all.
    for (unsigned int i = 1; i < data.size() / indexJump; i++)
    { // XXXX unsigned int i... something else? decide and be consistent

        // currentRowIndex += indexJump; // XXXX mention that commented on purpose because it's too long for demonstration purposes. even thought that what should have been for constant dt_plot / dt
        currentRowIndex = i * indexJump;
        SingleCoordsLine = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 3, 0); // XXXX

        std::unique_lock<std::mutex> EditKML_ul(KML_editing_mutex); // XXXX explaing on all mutexes why they are here. what's their purpse. what are they protecting. then set names to them and change all throughout. XXXX move to pthreads.

        // this->kmlAppendOneCoord(this->KML_path, std::to_string(0), "Sensor"); // Making a little shortcut and passing 0 for now. Assuming this is the only inspected trajectory as of the moment. // XXXX go over the comment.
        utils::kmlAppendOneCoord(this->KML_path, this->SingleCoordsLine, std::to_string(0), "Sensor"); // Making a little shortcut and passing 0 for now. Assuming this is the only inspected trajectory as of the moment. // XXXX go over the comment. XXXX HERE this->SingleCoordsLine and line above just SingleCoordsLine ?

        std::unique_lock<std::mutex> RunKML_ul(RunKML_mutex); // XXXX explaing on all mutexes why they are here. what's their purpse. what are they protecting. then set names to them and change all throughout. XXXX move to pthreads.

        std::thread t = threadLoadKML_ToGE(this->KML_path); // XXXX change name of t to something indicative. move to pthreads.

        t.join(); // XXXX change name of t to something indicative. move to pthreads.

        RunKML_ul.unlock(); // XXXX explaing on all mutexes why they are here. what's their purpse. what are they protecting. then set names to them and change all throughout. XXXX move to pthreads.

        EditKML_ul.unlock(); // XXXX explaing on all mutexes why they are here. what's their purpse. what are they protecting. then set names to them and change all throughout. XXXX move to pthreads.
    }
    return 0;
}
*/

/*
int SensorTrajectoryCADAC::kmlInsertEntireTrajectory(std::string KML, int indexJump, int currentNumOfSuppliers, int CollectorSize, float StyleScale) { // XXXX names... edit..
    std::cout << "Inserting entire SensorTrajectoryCADAC to KML: " << KML << " at index jumps of: " << indexJump << std::endl;   // XXXX English. check that this cout is needed. fix what needs to be fixed. XXXX fix cout
    std::string line;
    utils::rgb RGB;
    utils::hsv HSV;
    std::stringstream ss;

    HSV.h = ((((double)currentNumOfSuppliers) / (double)CollectorSize)) * 360.0;    // XXXX THERES ABSOLUTELY no way that so many casts are needed. MINIMIZE.
    HSV.s = 0.9;
    HSV.v = 1;
    RGB = utils::hsv2rgb(HSV);

    int decimalR = static_cast<int>(RGB.r * 255);
    int decimalG = static_cast<int>(RGB.g * 255);
    int decimalB = static_cast<int>(RGB.b * 255);
    int decimalA = (1 * 255);

    ss << std::hex << std::setfill('0') << std::setw(2) << decimalA << std::hex <<std::setfill('0') << std::setw(2) << decimalG << std::hex << std::setfill('0') << std::setw(2) << decimalB << std::hex << std::setfill('0') << std::setw(2) << decimalR << std::endl;
    std::string color = ss.str();   // XXXX ss.str() ? FROm what library this and c_str, to_string .substr() etc are from ?

    std::unique_lock<std::mutex> EditKML_ul(KML_editing_mutex); // XXXX name XXXX explaing on all mutexes why they are here. what's their purpse. what are they protecting. then set names to them and change all throughout. XXXX move to pthreads.

    //this->kmlInsertOneStyle(KML, std::to_string(currentNumOfSuppliers), color.substr(0, color.size() - 1), std::to_string(StyleScale)); // XXXX if unnecessary, delete
    utils::kmlInsertOneStyle(KML, std::to_string(currentNumOfSuppliers), color.substr(0, color.size() - 1), std::to_string(StyleScale));  // XXXX names etc


    this->currentRowIndex = 1;  // XXXX Compensation for output variables title in CADAC's output file.

    for(unsigned int i = 1; i < this->data.size() / indexJump; i++) {   // XXXX unsigned int ? i++ so why had there ++i ? strange ++i
        // currentRowIndex = i * indexJump; // XXXX makes sense to put in comment, add comment about it.
        //line = data[currentRowIndex]; // XXXX CLEAN IF DON"T NEED
        //SingleCoordsLine = line.substr(78,6) + "," + line.substr(66,6) + "," + line.substr(18,2) + line.substr(21,3);   // XXXX change ofc
        std::string lon  = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 4, 1); // XXXX check on func arguments to match cadac / general(?)
        std::string lat  = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 5, 1); // XXXX check on func arguments to match cadac / general(?)
        std::string alt  = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 6, 1); // XXXX check on func arguments to match cadac / general(?)
        SingleCoordsLine = lon + "," + lat +"," + alt;  // XXXX THERE MUST be a much more efficient way... ask chatgpt or think... at the end, in enhancements

        //this->kmlAppendOneCoord(KML, std::to_string(currentNumOfSuppliers), "Supplier");
        utils::kmlAppendOneCoord(KML, this->SingleCoordsLine, std::to_string(currentNumOfSuppliers), "Supplier"); // XXXX here this->singlecoordsline and line about no this-> ?

        currentRowIndex += indexJump;   // XXXX fix name of indexJump (?)
    }

    EditKML_ul.unlock();    // XXXX challenge the fact that mutexes are really where they should be. all throughout the code. XXXX explaing on all mutexes why they are here. what's their purpse. what are they protecting. then set names to them and change all throughout. XXXX move to pthreads.

    return 0;
}
*/

void SensorTrajectoryCADAC::setSingleCoordsLine()
{
    /*
    std::string lon = utils::SubStringStartTillReaching(this->data[this->currentRowIndex], ',', 4, 1); // XXXX check on func arguments to match cadac / general(?)  XXXX here i put this-> and below not. do both work? why? which to choose?
    std::string lat = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 5, 1); // XXXX check on func arguments to match cadac / general(?)
    std::string alt = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 6, 1); // XXXX check on func arguments to match cadac / general(?)
    */

    std::string lon = utils::SubStringStartTillReaching(this->data[this->currentRowIndex], ',', 4, 1, currentRowIndex, "SensorTrajectoryCADAC::setSingleCoordsLine 1"); // XXXX check on func arguments to match cadac / general(?)  XXXX here i put this-> and below not. do both work? why? which to choose?
    std::string lat = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 5, 1, currentRowIndex, "SensorTrajectoryCADAC::setSingleCoordsLine 2"); // XXXX check on func arguments to match cadac / general(?)
    std::string alt = utils::SubStringStartTillReaching(data[currentRowIndex], ',', 6, 1, currentRowIndex, "SensorTrajectoryCADAC::setSingleCoordsLine 3"); // XXXX check on func arguments to match cadac / general(?)


    this->SingleCoordsLine = lon + "," + lat + "," + alt;   // XXXX THERE MUST be a much more efficient way... ask chatgpt or think... at the end, in enhancements

} // XXXX names etc

//int SensorTrajectoryCADAC::PlotTrajectoryAtOnce(std::string KML, int indexJump, int currentNumbebrOfSuppliers, int CollectorSize, float StyleScale) {};
// XXXX in order to fill an entire sensor and an entire supplier, write for instance CADAC_As_Sensor and CADAC_As_Supplier. mention it in the paper and github. create a template for both.