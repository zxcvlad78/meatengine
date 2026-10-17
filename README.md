# meatengine
## A simple c++ library (or game engine idk) for game development

### Features
- **Lua scripting**
- **Flexible modding**
- **Low level networking (in dev)**

### Linking with your project
1. **Create a vcpkg.json file in the root directory of your project with the following content:**
```json
{
  "name": "project-name",
  "version-string": "1.0.0",
  "builtin-baseline": "80a2d60351a9fbad9f158a6f71a98fd6ee930998", 
  "dependencies": [
    "sfml",
    "entt",
    "termcolor",
    "nlohmann-json",
    "gamenetworkingsockets"
  ]
}
```
2. **run:**
```
git clone <this-repo>
```
3. **move the "/meatengine" folder into your project directory**
4. **add these lines to your CMakeLists.txt file:**
```cmake
add_subdirectory(meatengine)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,meatengine::meatengine>"
)
```

- **Your CMakeLists.txt file should look something like this:**

```cmake
cmake_minimum_required(VERSION 3.24)

set(PROJECT_NAME godlike) # your project name
project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(meatengine)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "include/"
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,meatengine::meatengine>"
)

# copy data folder for mods and resources
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
    COMMENT "Data copied successfully to build directory."
)
```
##

5. **run:**
```
vcpkg install
```
6. **Configure your cmake project**
