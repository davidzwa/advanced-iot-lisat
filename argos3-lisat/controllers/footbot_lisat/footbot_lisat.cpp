/* Include the controller definition */
#include "footbot_lisat.h"
/* Function definitions for XML parsing */
#include <argos3/core/utility/configuration/argos_configuration.h>
/* 2D vector definition */
#include <argos3/core/utility/math/vector2.h>

/****************************************/
/****************************************/

CFootBotLisat::CFootBotLisat() :
   m_pcWheels(NULL),
   m_pcProximity(NULL),
   m_cAlpha(10.0f),
   m_fDelta(0.5f),
   m_fWheelVelocity(2.5f),
   m_cGoStraightAngleRange(-ToRadians(m_cAlpha),
                           ToRadians(m_cAlpha)) {}

/****************************************/
/****************************************/

void CFootBotLisat::Init(TConfigurationNode& t_node) {
   /*
    * Get sensor/actuator handles
    *
    * The passed string (ex. "differential_steering") corresponds to the
    * XML tag of the device whose handle we want to have. For a list of
    * allowed values, type at the command prompt:
    *
    * $ argos3 -q actuators
    *
    * to have a list of all the possible actuators, or
    *
    * $ argos3 -q sensors
    *
    * to have a list of all the possible sensors.
    *
    * NOTE: ARGoS creates and initializes actuators and sensors
    * internally, on the basis of the lists provided the configuration
    * file at the <controllers><footbot_diffusion><actuators> and
    * <controllers><footbot_diffusion><sensors> sections. If you forgot to
    * list a device in the XML and then you request it here, an error
    * occurs.
    */
   m_pcWheels    = GetActuator<CCI_DifferentialSteeringActuator>("differential_steering");
   m_pcProximity = GetSensor  <CCI_FootBotProximitySensor      >("footbot_proximity"    );
   /*
    * Parse the configuration file
    *
    * The user defines this part. Here, the algorithm accepts three
    * parameters and it's nice to put them in the config file so we don't
    * have to recompile if we want to try other settings.
    */
   GetNodeAttributeOrDefault(t_node, "alpha", m_cAlpha, m_cAlpha);
   m_cGoStraightAngleRange.Set(-ToRadians(m_cAlpha), ToRadians(m_cAlpha));
   GetNodeAttributeOrDefault(t_node, "delta", m_fDelta, m_fDelta);
   GetNodeAttributeOrDefault(t_node, "velocity", m_fWheelVelocity, m_fWheelVelocity);

   /* Store number of robots through parameters in .argos file */
   GetNodeAttributeOrDefault(t_node, "number_of_robots", m_number_of_robots, m_number_of_robots);

   /* Contains location information about other robots */
   RobotRelativeLocation m_otherRobotLocations[m_number_of_robots-1];

}

/****************************************/
/****************************************/

/* The line correction algorithm */
WheelVelocities LineCorrectionAlgorithm() {

    WheelVelocities wheelVelocities = {5,5};

    // robotsCloserToStartpoint = CalculateRobotsCloserToStartpoint();

    // // If close to startpoint
    // if (disanceToStartpoint < interRobotDistanceValue) {
    //   if (robotsCloserToStartpoint.length() == 0) {
    //     //self.at_final_position = True;
    //     wheelVelocities = {0, 0};
    //     return wheelVelocities;
    //   }
    //   else {
    //     potential_positions = calculate_desired_positions(line.startpointAdjacentPositions, robotsCloserToStartpoint)
    //     distanceToPotentialPositions = calculate_distances(potentialPositions);
    //     return min(distanceToPotentialPositions) // return closest potential position 
    //   }
    // }
    // else {
    //   return startpoint;
    // }


  //WheelVelocities wheelVelocities = {5, 5};
  return wheelVelocities;
}

void CFootBotLisat::ControlStep() {

  // If the robot is a leader it remains stationary
  if (m_isLeader) return;

   /* Get readings from proximity sensor */
   const CCI_FootBotProximitySensor::TReadings& tProxReads = m_pcProximity->GetReadings();
   /* Sum them together */
   CVector2 cAccumulator;
   for(size_t i = 0; i < tProxReads.size(); ++i) {
      cAccumulator += CVector2(tProxReads[i].Value, tProxReads[i].Angle);
   }
   cAccumulator /= tProxReads.size();
   /* If the angle of the vector is small enough and the closest obstacle
    * is far enough, continue going straight, otherwise curve a little
    */
   CRadians cAngle = cAccumulator.Angle();
   if(m_cGoStraightAngleRange.WithinMinBoundIncludedMaxBoundIncluded(cAngle) &&
      cAccumulator.Length() < m_fDelta ) {


      /* Set wheel velocity for target location */
      WheelVelocities targetWheelVelocities = LineCorrectionAlgorithm();
      m_pcWheels->SetLinearVelocity(targetWheelVelocities.leftWheelVelocity, targetWheelVelocities.rightWheelVelocity);

   }
   else {
      /* Turn, depending on the sign of the angle */
      if(cAngle.GetValue() > 0.0f) {
         m_pcWheels->SetLinearVelocity(m_fWheelVelocity, 0.0f);
      }
      else {
         m_pcWheels->SetLinearVelocity(0.0f, m_fWheelVelocity);
      }
   }
}

void ReceiveLocationMessage(RobotRelativeLocation relativeLocation) { //todo: add robot id

};

void CFootBotLisat::GiveLeaderStatus() {
  m_isLeader = true;
}

void CFootBotLisat::RemoveLeaderStatus() {
  m_isLeader = false;
}

bool CFootBotLisat::hasLeaderStatus() {
  return m_isLeader;
}


/****************************************/
/****************************************/

/*
 * This statement notifies ARGoS of the existence of the controller.
 * It binds the class passed as first argument to the string passed as
 * second argument.
 * The string is then usable in the configuration file to refer to this
 * controller.
 * When ARGoS reads that string in the configuration file, it knows which
 * controller class to instantiate.
 * See also the configuration files for an example of how this is used.
 */
REGISTER_CONTROLLER(CFootBotLisat, "footbot_lisat_controller")
