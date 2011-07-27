MODULES = JavaScriptCore ThirdParty/ANGLE WebCore WebKit WebKit2

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/smagnuson/webkit/webkit_3.2/Source

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/smagnuson/webkit/webkit_3.2/Source

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running interactive CMake command-line interface..."
	/usr/bin/cmake -i .
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target install
install: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install

# Special rule for the target install
install/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install/fast

# Special rule for the target install/local
install/local: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local

# Special rule for the target install/local
install/local/fast: install/local
.PHONY : install/local/fast

# Special rule for the target install/strip
install/strip: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip

# Special rule for the target install/strip
install/strip/fast: install/strip
.PHONY : install/strip/fast

# Special rule for the target list_install_components
list_install_components:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Available install components are: \"Unspecified\""
.PHONY : list_install_components

# Special rule for the target list_install_components
list_install_components/fast: list_install_components
.PHONY : list_install_components/fast

# Special rule for the target package
package: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Run CPack packaging tool..."
	/usr/bin/cpack --config ./CPackConfig.cmake
.PHONY : package

# Special rule for the target package
package/fast: package
.PHONY : package/fast

# Special rule for the target package_source
package_source:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Run CPack packaging tool for source..."
	/usr/bin/cpack --config ./CPackSourceConfig.cmake /home/smagnuson/webkit/webkit_3.2/Source/CPackSourceConfig.cmake
.PHONY : package_source

# Special rule for the target package_source
package_source/fast: package_source
.PHONY : package_source/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/smagnuson/webkit/webkit_3.2/Source/CMakeFiles /home/smagnuson/webkit/webkit_3.2/Source/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/smagnuson/webkit/webkit_3.2/Source/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named javascriptcore_netflix

# Build rule for target.
javascriptcore_netflix: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 javascriptcore_netflix
.PHONY : javascriptcore_netflix

# fast build rule for target.
javascriptcore_netflix/fast:
	$(MAKE) -f JavaScriptCore/CMakeFiles/javascriptcore_netflix.dir/build.make JavaScriptCore/CMakeFiles/javascriptcore_netflix.dir/build
.PHONY : javascriptcore_netflix/fast

#=============================================================================
# Target rules for targets named wtf_netflix

# Build rule for target.
wtf_netflix: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 wtf_netflix
.PHONY : wtf_netflix

# fast build rule for target.
wtf_netflix/fast:
	$(MAKE) -f JavaScriptCore/wtf/CMakeFiles/wtf_netflix.dir/build.make JavaScriptCore/wtf/CMakeFiles/wtf_netflix.dir/build
.PHONY : wtf_netflix/fast

#=============================================================================
# Target rules for targets named jsc_netflix

# Build rule for target.
jsc_netflix: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 jsc_netflix
.PHONY : jsc_netflix

# fast build rule for target.
jsc_netflix/fast:
	$(MAKE) -f JavaScriptCore/shell/CMakeFiles/jsc_netflix.dir/build.make JavaScriptCore/shell/CMakeFiles/jsc_netflix.dir/build
.PHONY : jsc_netflix/fast

#=============================================================================
# Target rules for targets named webcore_netflix

# Build rule for target.
webcore_netflix: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 webcore_netflix
.PHONY : webcore_netflix

# fast build rule for target.
webcore_netflix/fast:
	$(MAKE) -f WebCore/CMakeFiles/webcore_netflix.dir/build.make WebCore/CMakeFiles/webcore_netflix.dir/build
.PHONY : webcore_netflix/fast

#=============================================================================
# Target rules for targets named webkit_netflix

# Build rule for target.
webkit_netflix: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 webkit_netflix
.PHONY : webkit_netflix

# fast build rule for target.
webkit_netflix/fast:
	$(MAKE) -f WebKit/CMakeFiles/webkit_netflix.dir/build.make WebKit/CMakeFiles/webkit_netflix.dir/build
.PHONY : webkit_netflix/fast

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... install"
	@echo "... install/local"
	@echo "... install/strip"
	@echo "... list_install_components"
	@echo "... package"
	@echo "... package_source"
	@echo "... rebuild_cache"
	@echo "... javascriptcore_netflix"
	@echo "... wtf_netflix"
	@echo "... jsc_netflix"
	@echo "... webcore_netflix"
	@echo "... webkit_netflix"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

