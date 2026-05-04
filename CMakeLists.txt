build_exec(
  EXECNAME uav_secure
  SOURCE_FILES
    main.cc
    common.cc
    security.cc
    anti_replay.cc
    gui.cc
    metrics.cc
    app.cc
  LIBRARIES_TO_LINK
    ${ns3-libs}
    ${ns3-contrib-libs}
    ssl
    crypto
  EXECUTABLE_DIRECTORY_PATH
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scratch/uav_secure/
)
