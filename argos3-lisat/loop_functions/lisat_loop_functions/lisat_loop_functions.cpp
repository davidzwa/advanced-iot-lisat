#include "lisat_loop_functions.h"
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/plugins/robots/foot-bot/simulator/footbot_entity.h>
#include <controllers/footbot_lisat/footbot_lisat.h>



#include <math.h> 

#define _USE_MATH_DEFINES

/****************************************/
/****************************************/

CLisatLoopFunctions::CLisatLoopFunctions() :
   // m_unCollectedFood(0),
   // m_nEnergy(0),
   // m_unEnergyPerFoodItem(1),
   // m_unEnergyPerWalkingRobot(1)
   m_pcFloor(NULL),
   m_pcRNG(NULL),
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
      /* Open the file, erasing its contents */
      m_cOutput.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
      m_cOutput << "# clock\twalking\tresting\tcollected_food\tenergy" << std::endl;

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
   }
   catch(CARGoSException& ex) {
      THROW_ARGOSEXCEPTION_NESTED("Error parsing loop functions!", ex);
   }
}

/****************************************/
/****************************************/

void CLisatLoopFunctions::Reset() {
   /* Zero the counters */
   /* Close the file */
   m_cOutput.close();
   /* Open the file, erasing its contents */
   m_cOutput.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
   m_cOutput << "# col 1\twcol 2" << std::endl;
}

/****************************************/
/****************************************/

void CLisatLoopFunctions::Destroy() {
   /* Close the file */
   m_cOutput.close();
}

void CLisatLoopFunctions::BroadcastRobotFinished() {
   argos::LOG << "Robot finished!" << std::endl;
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

void CLisatLoopFunctions::PreStep() {

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
      } else {
         int id = StringIDtoInt(cController.GetId());
         CVector2 position;
         position.Set(truePosition.GetX(), truePosition.GetY());
         robotPositions[id] = position;
         finishedRobots[id] = cController.IsFinished();
      }

      /* Check if any broadcasts of a robot's finished status are required */
      if (cController.checkBroadcastFinishedStatus()) {
         broadcastFinishedCount++;
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
         if (StringIDtoInt(cController.GetId()) == 1) { // for testing purposes
            argos::LOG << "angle to leader: " << angleRelativeToLeader << std::endl;    
         }
         cController.ReceiveLocationMessage(distanceToLeader, angleRelativeToLeader, 0, true, false);

         /* Let every robot loop over all other robots to forward its location */
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
            //argos::LOG << "angle to sender " << senderId << ": " << angleRelativeToSender << std::endl;         
            cController.ReceiveLocationMessage(distanceToSender, angleRelativeToSender, senderId, false, finishedRobots[senderId]);        
         }


      }
   }   

      //argos::LOG << cController.m_isLeader; << std::endl;

      // /* Count how many foot-bots are in which state */
      // if(! cController.IsResting()) ++unWalkingFBs;
      // else ++unRestingFBs;

      /* Get the position of the foot-bot on the ground as a CVector2 */
      // CVector2 cPos;
      // cPos.Set(cFootBot.GetEmbodiedEntity().GetOriginAnchor().Position.GetX(),
      //          cFootBot.GetEmbodiedEntity().GetOriginAnchor().Position.GetY());
      // argos::LOG << cPos.GetX() << std::endl;




      // /* Get food data */
      // CFootBotLisat::SFoodData& sFoodData = cController.GetFoodData();
      
      // /* The foot-bot has a food item */
      // if(sFoodData.HasFoodItem) {
      //    /* Check whether the foot-bot is in the nest */
      //    if(cPos.GetX() < -1.0f) {
      //       /* Place a new food item on the ground */
      //       m_cFoodPos[sFoodData.FoodItemIdx].Set(m_pcRNG->Uniform(m_cForagingArenaSideX),
      //                                             m_pcRNG->Uniform(m_cForagingArenaSideY));
      //       //Drop the food item 
      //       sFoodData.HasFoodItem = false;
      //       sFoodData.FoodItemIdx = 0;
      //       ++sFoodData.TotalFoodItems;
      //       /* Increase the energy and food count */
      //       m_nEnergy += m_unEnergyPerFoodItem;
      //       ++m_unCollectedFood;
      //       /* The floor texture must be updated */
      //       m_pcFloor->SetChanged();
      //    }
      // }
      // else {
      //    /* The foot-bot has no food item */
      //    /* Check whether the foot-bot is out of the nest */
      //    if(cPos.GetX() > -1.0f) {
      //       /* Check whether the foot-bot is on a food item */
      //       bool bDone = false;
      //       for(size_t i = 0; i < m_cFoodPos.size() && !bDone; ++i) {
      //          if((cPos - m_cFoodPos[i]).SquareLength() < m_fFoodSquareRadius) {
      //              If so, we move that item out of sight 
      //             m_cFoodPos[i].Set(100.0f, 100.f);
      //             /* The foot-bot is now carrying an item */
      //             sFoodData.HasFoodItem = true;
      //             sFoodData.FoodItemIdx = i;
      //             /* The floor texture must be updated */
      //             m_pcFloor->SetChanged();
      //             /* We are done */
      //             bDone = true;
      //          }
      //       }
      //    }
      // }
   /* Update energy expediture due to walking robots */
   // m_nEnergy -= unWalkingFBs * m_unEnergyPerWalkingRobot;
   // /* Output stuff to file */
   // m_cOutput << GetSpace().GetSimulationClock() << "\t"
   //           << unWalkingFBs << "\t"
   //           << unRestingFBs << "\t"
   //           << m_unCollectedFood << "\t"
   //           << m_nEnergy << std::endl;
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(CLisatLoopFunctions, "lisat_loop_functions")
