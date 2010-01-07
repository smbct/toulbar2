###################################################################################"
#  PACKAGING 
###################################################################################"
# --- Package generation
#########################"

INCLUDE(InstallRequiredSystemLibraries)


IF (APPLE)
	SET(CPACK_GENERATOR "OSXX11")
ENDIF (APPLE)

IF (WIN32)
	SET(CPACK_GENERATOR "NSIS")
	SET(CPACK_SOURCE_GENERATOR "ZIP")
ELSE(WIN32)
	MESSAGE(STATUS "#################################")
	MESSAGE(STATUS " project :${PROJECT_NAME} packaging")
	MESSAGE(STATUS " source:${CMAKE_CURRENT_SOURCE_DIR} FOR UNIX")
	MESSAGE(STATUS "#################################")
	MESSAGE(STATUS "${My_misc_bin}/peo ${My_misc_bin}/narycsp  added in the package")
	MESSAGE(STATUS "---------------------------------")
	INSTALL(FILES ${My_misc_bin}/peo ${My_misc_bin}/narycsp  DESTINATION bin)

	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libgcc1 (>= 1:3.4.2-12), boost (>=1.34.0), xml2 ")
	SET(CPACK_STRIP_FILES "bin/toulbar2;bin/iloglue;bin/mendelsoft")
	SET(CPACK_GENERATOR "STGZ;TGZ;DEB;TBZ2;RPM")
	SET(CPACK_SOURCE_GENERATOR "ZIP;TGZ;TBZ2;DEB")
ENDIF (WIN32)

	SET(PACKAGE_DESCRIPTION "${Toulbar_PACKAGE_DESCRIPTION}")
	SET(PACKAGE_DESCRIPTION_SUMMARY "${Toulbar_PACKAGE_SUMMARY}" )
	SET(CPACK_PACKAGE_NAME "${Toulbar_NAME}")
	SET(CPACK_PACKAGE_VENDOR "${Toulbar_NAME} Development Team INRA Toulouse ")
	SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${Toulbar_PACKAGE_SUMMARY}")
	SET(CPACK_PACKAGE_CONTACT "${CONTACT}")
	SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
	SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
#	SET(CPACK_PACKAGE_FILE_NAME "${Toulbar_NAME_COMPLETE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
	SET(CPACK_PACKAGE_FILE_NAME "${Toulbar_NAME_COMPLETE}")
	SET(CPACK_PACKAGE_VERSION_MAJOR "${Toulbar_VERSION_MAJOR}")
	SET(CPACK_PACKAGE_VERSION_MINOR "${Toulbar_MINOR}")

	SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${Toulbar_NAME_COMPLETE})
	SET(CPACK_PACKAGE_EXECUTABLES "toulbar2" "toulbar2" "iloglue" "ILOGLUE" "mendelsoft" "MENDELSOFTL")

# CPack source configuration
	SET(CPACK_SOURCE_PACKAGE_FILE_NAME ${Toulbar_NAME_COMPLETE}-sources)
	SET(CPACK_SOURCE_IGNORE_FILES "\\\\.swp$;/\\\\.gitignore;/build/;/\\\\.git/;/\\\\.svn/;/bachckup;/_CPack_Packages/;/CMakeFiles/;/old/;\\\\.sh$;\\\\.cmake$;\\\\.zip$;\\\\.gz$;\\\\.bz2$")

# CPack DEB configuration
	SET(CPACK_DEBIAN_PACKAGE_NAME ${Toulbar_NAME})
	SET(CPACK_DEBIAN_PACKAGE_VERSION ${Toulbar_COMPLETE})
	SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
	#SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "")
	SET(CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS "")
	SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "${MAINTAINER}")
	SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${Toulbar_NAME},${Toular_PACKAGE_DESCRIPTION}")

	SET(CPACK_DEBIAN_PACKAGE_SECTION "math")
	SET(CPACK_DEBIAN_PACKAGE_PRIORITY "bin")

# CPack RPM configuration
	SET(CPACK_RPM_PACKAGE_SUMMARY "${Toulbar_PACKAGE_SUMMARY}")
	SET(CPACK_RPM_PACKAGE_NAME ${Toulbar_NAME})
	SET(CPACK_RPM_PACKAGE_VERSION ${Toulbar_COMPLETE})
	SET(CPACK_RPM_PACKAGE_ARCHITECTURE "i386")
	SET(CPACK_RPM_PACKAGE_RELEASE "1")
	SET(CPACK_RPM_PACKAGE_VENDOR "${Toulbar_NAME} Development Team")
	SET(CPACK_RPM_PACKAGE_LICENSE "GNU General Public License GPL")
	SET(CPACK_RPM_PACKAGE_DESCRIPTION "${Toulbar_PACKAGE_DESCRIPTION}")



# CPack NSIS configuration
IF (CPACK_GENERATOR MATCHES "NSIS")
	SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
	FILE(TO_CMAKE_PATH "${MINGW_BASEPATH}" MINGW_BASE)
	#  INSTALL(FILES "${MINGW_BASE}/bin\\\\mingwm10.dll" DESTINATION bin)
	SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/misc/pixmaps\\\\logo.png")
	SET(CPACK_NSIS_MENU_LINKS "${Toulbar2_SHARE_DIRS}/doc/toulbar2.chm" "Toulbar2" "https://mulcyber.toulouse.inra.fr/projects/toulbar2/" "${Toulbar_NAME} Web Site")
	SET(CPACK_CREATE_DESKTOP_LINKS ${Toulbar_NAME})
	SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${Toulbar_NAME}.exe")
	SET(CPACK_NSIS_DISPLAY_NAME "Toulbar2 - solver for weighted CSP")
	SET(CPACK_NSIS_HELP_LINK "https://mulcyber.toulouse.inra.fr/projects/toulbar2/")
	SET(CPACK_NSIS_URL_INFO_ABOUT "https://mulcyber.toulouse.inra.fr/projects/toulbar2/")
	SET(CPACK_NSIS_CONTACT "${CONTACT}")
	SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${Toulbar2_VERSION_SHORT}.0")
	SET(CPACK_NSIS_MODIFY_PATH ON)
ENDIF (CPACK_GENERATOR MATCHES "NSIS")

INCLUDE(CPack)

