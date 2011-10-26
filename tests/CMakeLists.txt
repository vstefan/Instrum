# Make sure the compiler can find include files
INCLUDE_DIRECTORIES(${INSTRUM_SOURCE_DIR}/src)

# Add binary called "run_tests" that is built from the source file "test-instrum.cpp".
# The extension is automatically found.
ADD_EXECUTABLE(run_tests test-instrum)

# Link the executable to the Instrum library.
TARGET_LINK_LIBRARIES(run_tests Instrum rt)