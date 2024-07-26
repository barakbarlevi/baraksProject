#pragma once

#include "Trajectory.h"
#include "utils.h"

class SensorTrajectory : public Trajectory {
    protected:
    
    float vVertical = 0;    // XXXX name sucks. XXXX explain on assumed coordinate system. up is down etc
    
    public:

    SensorTrajectory(std::string loadPath, std::string kmlPath);   // ADD CONSTS? WHEN TO ADD CONSTS? XXXX
    ~SensorTrajectory() = default; // COMPLETE IF THERE's memory allocation needed to be deleted. XXXX what's default? why this?
    virtual void setBITA_Params() = 0;  // XXXX names...
    BITA_params getBITA_Params() { return this->_BITA_Params; } // XXXX names...
    
    //float get_vVertical() { return std::stof(utils::SubStringStartTillReaching(this->data[currentRowIndex], ',', 10, 1)); } // XXXX the ',', 15, and 1 ARE NOT GENERAL BUT DEPENDENT ON SIMULATION so need to change or atleast comment that this is not the focus of the project and this line can be manipulated by any one who chooses so and i made simplification to myself that arent the interesting or important case.
    float get_vVertical() { return std::stof(utils::SubStringStartTillReaching(this->data[currentRowIndex], ',', 10, 1, currentRowIndex, "get_vVertical")); } // XXXX the ',', 15, and 1 ARE NOT GENERAL BUT DEPENDENT ON SIMULATION so need to change or atleast comment that this is not the focus of the project and this line can be manipulated by any one who chooses so and i made simplification to myself that arent the interesting or important case.
};