add_executable(lisat_main main.cpp)
target_link_libraries(lisat_main
  footbot_lisat
  lisat_loop_functions
  argos3core_simulator)
