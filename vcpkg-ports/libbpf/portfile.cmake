#    any-port-in-a-storm - port files for vcpkg
#    Copyright (C) 2024 Michael Przybylski
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO libbpf/libbpf
        REF v1.4.1
        SHA512 3073806d9fc2aafb6620f41bf7d73f17ebe92cff577821a309d0fb2192a88b8bb5a984d11590fe1050035dc4dc154d50372f78c2e87a1000ddb795e30120ddf5
        HEAD_REF master
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

# Configure libbpf build with CMake
vcpkg_configure_cmake(SOURCE_PATH ${SOURCE_PATH})

# build and install libbpf
vcpkg_build_cmake()
vcpkg_install_cmake()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
    # Remove static library archive files
    set(_lib_suffix "${VCPKG_TARGET_STATIC_LIBRARY_SUFFIX}")
    file(GLOB_RECURSE
            TO_REMOVE
            "${CURRENT_PACKAGES_DIR}/lib/*${_lib_suffix}"
            "${CURRENT_PACKAGES_DIR}/debug/lib/*${_lib_suffix}"
            "${CURRENT_PACKAGES_DIR}/lib/*${_lib_suffix}.*"
            "${CURRENT_PACKAGES_DIR}/debug/lib/*${_lib_suffix}.*")
    file(REMOVE ${TO_REMOVE})
endif()

vcpkg_fixup_pkgconfig()
vcpkg_install_copyright(FILE_LIST
        "${SOURCE_PATH}/LICENSE"
        "${SOURCE_PATH}/LICENSE.BSD-2-Clause"
        "${SOURCE_PATH}/LICENSE.LGPL-2.1")
