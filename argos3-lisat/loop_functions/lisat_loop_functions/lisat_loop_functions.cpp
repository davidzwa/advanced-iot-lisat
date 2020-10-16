#include "lisat_loop_functions.h"
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/plugins/robots/foot-bot/simulator/footbot_entity.h>
#include <controllers/footbot_lisat/footbot_lisat.h>



#include <math.h> 

#define _USE_MATH_DEFINES

std::default_random_engine generator;
std::normal_distribution<double> dist(NOISE_MEAN, NOISE_STD);  

/****************************************/ 
/****************************************/

CLisatLoopFunctions::CLisatLoopFunctions() :
   // m_unCollectedFood(0),
   // m_nEnergy(0),
   // m_unEnergyPerFoodItem(1),
   // m_unEnergyPerWalkingRobot(1)
   m_distributeBounds(-2.0f, 2.0f),
   m_pcFloor(NULL),
   m_pcRNG(NULL),
   m_finishedRobotsCount(0),
<<<<<<< HEAD
   m_finalLeaderPosition(-1, -1, -1),
   m_finalFirstFinishedRobotPosition(-1, -1, -1),
=======
>>>>>>> parent of b20f4ec... finished implementation to measure distance between a point and a line defined by two robot's location.
   m_robotCount(0) {}

/****************************************/
/****************************************/

void CLisatLoopFunctions::Init(TConfigurationNode& t_node) {
   try {
      TConfigurationNode& tLisat = GetNode(t_node, "lisat_parameters");
      /* Get a pointer to the floor entity */
      m_pcFloor = &GetSpace().GetFloorEntity();
      /* Get the number of food items we want to be scattered from XML */
      // get robot count from .argos configuration file
      GetNodeAttribute(tLisat, "number_of_robots", m_robotCount);

      // /* Create a new RNG */
      // m_pcRNG = CRandom::CreateRNG("argos");
      // /* Distribute uniformly the items in the environment */
      // for(UInt32 i = 0; i < unFoodItems; ++i) {
      //    m_cFoodPos.push_back(
      //       CVector2(m_pcRNG->Uniform(m_cForagingArenaSideX),
      //                m_pcRNG->Uniform(m_cForagingArenaSideY)));
      // }
      
      /* Get the output file name from XML */
      GetNodeAttribute(tLisat, "output", m_strOutput);
      GetNodeAttribute(tLisat, "output_sum", m_strErrorSumOutput);

      /* Open the file, erasing its contents */
      m_cOutput.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
      m_cOutput << "# counter \t distance" << std::endl;

      m_errorSumOutput.open(m_strErrorSumOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
      m_errorSumOutput << "# counter \t error sum " << std::endl;

      /* Iterate over all robots to pick a leader */
      CSpace::TMapPerType& m_cFootbots = GetSpace().GetEntitiesByType("foot-bot");
      for(CSpace::TMapPerType::iterator it = m_cFootbots.begin(); it != m_cFootbots.end(); ++it) {
         /* Get handle to foot-bot entity and controller */
         CFootBotEntity& cFootBot = *any_cast<CFootBotEntity*>(it->second);
         CFootBotLisat& cController = dynamic_cast<CFootBotLisat&>(cFootBot.GetControllableEntity().GetController());
         // Make first robot leader
         if (it == m_cFootbots.begin()) {
            cController.GiveLeaderStatus();
            argos::LOG << "Leader chosen" << std::endl;
         }
      
      }

      /* calculate planes */
      if (NUMBER_OF_MICS == 2)  {
         m_number_of_planes = 2;
      } else {
          m_number_of_planes = NUMBER_OF_MICS * 2;
      }
      float max_degrees = 360.0;
      float degrees = 0;
      m_plane_degree_step = max_degrees / m_number_of_planes;
      for (int i = 0; i < m_number_of_planes + 1; i++) {
         m_plane_angles[i] = degrees + (m_plane_degree_step * i);
      }

      m_pcRNG = CRandom::CreateRNG("argos");

   }
   catch(CARGoSException& ex) {
      THROW_ARGOSEXCEPTION_NESTED("Error parsing loop functions!", ex);
   }
}


/****************************************/
/****************************************/

void CLisatLoopFunctions::Destroy() {
   /* Close the file */
   m_cOutput.close();
   m_errorSumOutput.close();   
}

/****************************************/
/****************************************/

// CColor CLisatLoopFunctions::GetFloorColor(const CVector2& c_position_on_plane) {
//    if(c_position_on_plane.GetX() < -1.0f) {
//       return CColor::GRAY50;
//    }
//    for(UInt32 i = 0; i < m_cFoodPos.size(); ++i) {
//       if((c_position_on_plane - m_cFoodPos[i]).SquareLength() < m_fFoodSquareRadius) {
//          return CColor::BLACK;
//       }
//    }
//    return CColor::WHITE;
// }

/****************************************/
/****************************************/

float CalculateDistanceTwoRobots(CVector2 point1, CVector2 point2) {
   return sqrt( pow(point2.GetX() - point1.GetX(), 2) + pow(point2.GetY() - point1.GetY(), 2) );
}

float CalculateAngleTwoRobots(CRadians rob1Heading, CVector2 rob1, CVector2 rob2) {
   float atanResult = atan( abs(rob1.GetX() - rob2.GetX()) / abs(rob1.GetY() - rob2.GetY()) ) * 180 / M_PI; //degrees, should always be between 0 and 90
   float northToHeading = rob1Heading.GetValue() *180 / M_PI; //ranges from 0 to 180 (clockwise) and 0 to -180 (counterclockwise)
   //normalize north to heading to range: 0-360 (increasing clockwise)
   if (northToHeading < 0) {
      northToHeading = 360 + northToHeading;
   }

   /* Reverse heading angle direction and account for ARgos3 x-axis being reversed (hence + 90 degrees) */
   northToHeading = fmod(360 - northToHeading + 90, 360);

   //argos::LOG << "atan: " << atanResult << std::endl;  
   //argos::LOG << "heading: " << northToHeading << std::endl;  

   /* 
    * IMPORTANT: this value indicates how many
    * degrees robot has to turn clockwise
    * to make his heading point straight
    * to the target
    */
   float totalAngle;

   /* Calculate base angle depending on orientation of robot in relation to target */
   float base_formula;
   if (rob1.GetX() < rob2.GetX()) { // target to the right of robot
      if(rob1.GetY() < rob2.GetY()) { // target to the north of robot
         base_formula = atanResult;   
      }
      else {                          // target to the south of robot
         base_formula = 180 - atanResult;
      }
   } else {                           // target to the left of robot
      if(rob1.GetY() < rob2.GetY()) { // target to the north of robot
         base_formula = 360 - atanResult;
      }
      else {                          // target to the south of robot
         base_formula = 180 + atanResult;
      }     
   }
      
   //argos::LOG << "base_formula: " << base_formula << std::endl;  

   /* Incorporate heading of robot into total angle calculation */
   if (northToHeading < base_formula) {
      totalAngle = base_formula - northToHeading;
   } else {
      totalAngle = base_formula + (360 - northToHeading);
   }

   //argos::LOG << "heading: " << northToHeading << std::endl;  
   return (totalAngle);

}

int StringIDtoInt(std::string strid) {
   strid.std::string::erase(0, 2);
   return std::stoi(strid);
}

float CLisatLoopFunctions::TranslateAngleToPlane(float angle) {
   if (angle >= 360) { // quick fix, otherwise 360 gives out of bounds
      angle = 359;
   }
   int plane_id = (int) angle / m_plane_degree_step;
   float mid_plane_angle = (m_plane_angles[plane_id] + m_plane_angles[plane_id+1]) / 2;
   return mid_plane_angle; 
}

float addNormalNoiseToAngle(float angle) {
      //argos::LOG << "angle before noise: " << angle << std::endl;         
      angle = angle + dist(generator);
      //argos::LOG << "angle after noise: " << angle << std::endl;         
      return angle;
}



void CLisatLoopFunctions::PreStep() {
   argos::LOG << "finished robot count: " << m_finishedRobotsCount << std::endl;         

   // Get all footbots
   CSpace::TMapPerType& m_cFootbots = GetSpace().GetEntitiesByType("foot-bot");

   CVector2 leaderPosition;

   CVector2 robotPositions[m_robotCount];

   bool finishedRobots[m_robotCount];

   int broadcastFinishedCount = 0;

   // Iterate over all robots 
   for(CSpace::TMapPerType::iterator it = m_cFootbots.begin(); it != m_cFootbots.end(); ++it) {
      
      /* Get handle to foot-bot entity and controller */
      CFootBotEntity& cFootBot = *any_cast<CFootBotEntity*>(it->second);
      CFootBotLisat& cController = dynamic_cast<CFootBotLisat&>(cFootBot.GetControllableEntity().GetController());

      CVector3 truePosition = cFootBot.GetEmbodiedEntity().GetOriginAnchor().Position;

      /* find leader position */
      if (cController.hasLeaderStatus()) {
         leaderPosition.Set(truePosition.GetX(), truePosition.GetY());  
         finishedRobots[0] = 1;         
      } 
      else {
         int id = StringIDtoInt(cController.GetId());
         CVector2 position;
         position.Set(truePosition.GetX(), truePosition.GetY());
         robotPositions[id] = position;
         finishedRobots[id] = cController.IsFinished();
      }

      /* Check if any broadcasts of a robot's finished status are required */
      if (cController.checkBroadcastFinishedStatus()) {
         broadcastFinishedCount++;
         m_finishedRobotsCount++; // add to total number of robots
<<<<<<< HEAD

         if (m_finishedRobotsCount == 1) {
            m_finalFirstFinishedRobotPosition.x = truePosition.GetX();
            m_finalFirstFinishedRobotPosition.y = truePosition.GetY();
            m_finalFirstFinishedRobotPosition.z = truePosition.GetZ();
            cController.setFinishedFirst();
         }
=======
>>>>>>> parent of b20f4ec... finished implementation to measure distance between a point and a line defined by two robot's location.
         cController.confirmBroadcastFinishedStatus();
      }
      
      /* Update if another robot recently finished */

   }


   /* Iterate over all robots to broadcast leader position and any finished robots */ //TODO: leader position actually only needs to be broadcasted once?
   for(CSpace::TMapPerType::iterator it = m_cFootbots.begin(); it != m_cFootbots.end(); ++it) {
      
      /* Get handle to foot-bot entity and controller */
      CFootBotEntity& cFootBot = *any_cast<CFootBotEntity*>(it->second);
      CFootBotLisat& cController = dynamic_cast<CFootBotLisat&>(cFootBot.GetControllableEntity().GetController());
      CVector2 selfPosition;
      if (!cController.hasLeaderStatus()) {

         if (broadcastFinishedCount > 0) {
            cController.updateRobotsFinishedCount(broadcastFinishedCount);            
         }

         //argos::LOG << "robot id: " << StringIDtoInt(cController.GetId()) << std::endl;    
         CVector3 truePosition = cFootBot.GetEmbodiedEntity().GetOriginAnchor().Position;
         selfPosition.Set(truePosition.GetX(), truePosition.GetY());   
         float distanceToLeader = CalculateDistanceTwoRobots(leaderPosition, selfPosition);

         CRadians xAngle; CRadians yAngle; CRadians zAngle;
         cFootBot.GetEmbodiedEntity().GetOriginAnchor().Orientation.ToEulerAngles(zAngle, yAngle, xAngle);
      
         float angleRelativeToLeader = CalculateAngleTwoRobots(zAngle, selfPosition, leaderPosition);
         // if (StringIDtoInt(cController.GetId()) == 1) { // for testing purposes
         //    argos::LOG << "angle to leader: " << angleRelativeToLeader << std::endl;    
         // }
         float adjustedAngle = addNormalNoiseToAngle(angleRelativeToLeader);
         cController.ReceiveLocationMessage(distanceToLeader, adjustedAngle, 0, true);
         //argos::LOG << "adjust angle to leader: " << adjustedAngle << std::endl;    


         /* Let every robot loop over all senders to receive their location */
         for (int senderId = 1; senderId < m_robotCount; senderId++) {
            if (senderId == StringIDtoInt(cController.GetId())) { //skip itself
               continue;
            }
            CVector2 senderPosition;
            senderPosition = robotPositions[senderId];

            float distanceToSender = CalculateDistanceTwoRobots(selfPosition, senderPosition);
            CRadians xAngle; CRadians yAngle; CRadians zAngle;
            cFootBot.GetEmbodiedEntity().GetOriginAnchor().Orientation.ToEulerAngles(zAngle, yAngle, xAngle);
            float angleRelativeToSender = CalculateAngleTwoRobots(zAngle, selfPosition, senderPosition);

            //argos::LOG << "distance to sender " << senderId << ": " << distanceToSender << std::endl;    
            //argos::LOG << "angle to sender " << senderId << ": " << angleRelativeToSender <<
            float adjustedAngle2 = addNormalNoiseToAngle(angleRelativeToSender);
            cController.ReceiveLocationMessage(distanceToSender, adjustedAngle2, senderId, finishedRobots[senderId]);        
         }


      }
   }   
}

<<<<<<< HEAD

void CLisatLoopFunctions::Reset() {

}

void CLisatLoopFunctions::PostStep() {
   // if (m_finishedRobotsCount == m_robotCount - 1) { //exclude leader
   //    CSimulator& cSimulator = this->GetSimulator();
   //    //cSimulator.Terminate();
   //    //cSimulator.Reset();
   //    exit(0);
   // }
}

bool CLisatLoopFunctions::IsExperimentFinished() {
   if (m_finishedRobotsCount == m_robotCount - 1) { //exclude leader


   argos::CSimulator& cSimulator = argos::CSimulator::GetInstance();

   cSimulator.Terminate();
      int results[m_robotCount];
      int counter = 1;
      float error_sum = 0;
      Vector leader_position = m_finalLeaderPosition;
      Vector finished_robot_position = m_finalFirstFinishedRobotPosition;

      argos::LOG <<"Leader X: " << m_finalLeaderPosition.x << std::endl;    
      argos::LOG <<"Leader Y: " << m_finalLeaderPosition.y << std::endl;    
      argos::LOG <<"Leader Z: " << m_finalLeaderPosition.z << std::endl;    
      argos::LOG <<"finished X: " << m_finalFirstFinishedRobotPosition.x << std::endl;    
      argos::LOG <<"finished Y: " << m_finalFirstFinishedRobotPosition.y << std::endl;    
      argos::LOG <<"finished Z: " << m_finalFirstFinishedRobotPosition.z << std::endl;    

      /* Loop over all robots and calculate distance to line formed by leader and first finished robots */
      CSpace::TMapPerType& m_cFootbots = GetSpace().GetEntitiesByType("foot-bot");
      for(CSpace::TMapPerType::iterator it = m_cFootbots.begin(); it != m_cFootbots.end(); ++it) {

         CFootBotEntity& cFootBot = *any_cast<CFootBotEntity*>(it->second);
         CFootBotLisat& cController = dynamic_cast<CFootBotLisat&>(cFootBot.GetControllableEntity().GetController());

         if (!cController.hasLeaderStatus() && !cController.checkFinishedFirst()) {
            CVector3 robPos = cFootBot.GetEmbodiedEntity().GetOriginAnchor().Position;
            Vector robotFinalPosition(robPos.GetX(), robPos.GetY(), robPos.GetZ());
            float error = shortDistance(leader_position, finished_robot_position, robotFinalPosition);
            argos::LOG << "Distance Error for robot " << cController.GetId() << "is: " << error << std::endl;
            m_cOutput << "#" << counter << "\te: " << error << std::endl;           
            results[counter] = error;
            error_sum += error;
            counter++;
         }
      }
      float error_avg = error_sum / (m_robotCount - 2);
         /* Close the file */
      m_errorSumOutput << "#" << counter << "\t avg error: " << error_avg << std::endl; 
      m_cOutput.close();
      m_errorSumOutput.close();
=======
bool CLisatLoopFunctions::IsExperimentFinished() {
   if (m_finishedRobotsCount == m_robotCount-1) {
>>>>>>> parent of b20f4ec... finished implementation to measure distance between a point and a line defined by two robot's location.
      return true;
   }
   return false;
}

void CLisatLoopFunctions::PostExperiment() {
   exit(0);
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(CLisatLoopFunctions, "lisat_loop_functions")
