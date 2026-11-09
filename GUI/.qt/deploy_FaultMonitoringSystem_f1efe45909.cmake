include("D:/Qt/repos/FaultMonitoringSystem/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FaultMonitoringSystem-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "D:/Qt/repos/FaultMonitoringSystem/FaultMonitoringSystem.exe"
    GENERATE_QT_CONF
)
