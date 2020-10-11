#ifndef FORAGING_LOOP_FUNCTIONS_H
#define FORAGING_LOOP_FUNCTIONS_H

#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/core/utility/math/range.h>
#include <argos3/core/utility/math/rng.h>
#include <chrono>

using namespace argos;

class CLisatLoopFunctions : public CLoopFunctions {

public:

   CLisatLoopFunctions();
   virtual ~CLisatLoopFunctions() {}

   virtual void Init(TConfigurationNode& t_tree);
   virtual void Reset();
   virtual void Destroy();
   //virtual CColor GetFloorColor(const CVector2& c_position_on_plane);
   virtual void PreStep();

   virtual void BroadcastRobotFinished();

private:

   //std::vector<CVector2> m_cFoodPos;
   CFloorEntity* m_pcFloor;
   CRandom::CRNG* m_pcRNG;

   std::string m_strOutput;
   std::ofstream m_cOutput;


   int m_robotCount;
   //UInt32 m_unCollectedFood;
};

#endif
