# Copyright 2018 Max Harmathy
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.1)

project(harfbuzz-qml LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick Widgets REQUIRED)
find_package(KF5Declarative REQUIRED)

find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)


set(harfbuzz-qml_SRCS
  main.cpp
  qml.qrc
  fontsettingsmodel.cpp
  freetype-renderer.cpp
  kxftconfig.cpp
  menupreviewimageprovider.cpp
  menupreview.cpp
)

include_directories( . )

add_executable(${PROJECT_NAME} ${harfbuzz-qml_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC
  "${FREETYPE_INCLUDE_DIRS}"
  "${FONTCONFIG_INCLUDE_DIRS}"
  "${HARFBUZZ_INCLUDE_DIRS}"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt5::Core
  Qt5::Quick
  Qt5::Widgets
  KF5::Declarative
  "${FREETYPE_LIBRARIES}"
  "${FONTCONFIG_LIBRARIES}"
  "${HARFBUZZ_LIBRARIES}"
)
