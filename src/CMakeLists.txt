add_executable(${PROJECT_NAME} 
  main.cc
  minesweeper.cc
  minesweeper.h
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
