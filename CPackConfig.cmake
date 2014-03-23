include(InstallRequiredSystemLibraries)

# For help take a look at:
# http://www.cmake.org/Wiki/CMake:CPackConfiguration

### general settings
set(CPACK_PACKAGE_NAME "StoRM")
set(CPACK_PACKAGE_VENDOR "i2 RWTH Aachen University")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Stochastic Reward Model Checker - An extensible model checker written in C++.")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

### versions
set(CPACK_PACKAGE_VERSION_MAJOR "${STORM_CPP_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${STORM_CPP_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${STORM_CPP_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${STORM_CPP_VERSION_HASH}")

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

### source package settings
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES "~$;[.]swp$;/[.]svn/;/[.]git/;.gitignore;/build/;tags;cscope.*")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src")

include(CPack)