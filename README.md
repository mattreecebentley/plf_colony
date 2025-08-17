# plf::colony
An unordered data container providing fast iteration/insertion/erasure while maintaining pointer/iterator/reference validity to non-erased elements.
Documentation and function descriptions here: https://plflib.org/colony.htm#functions

 - A conan package for colony is available here: https://conan.io/center/plf_colony
 - A build2 package for colony is available here: https://cppget.org/plf-colony
 - Easy integration with CPM: 
 ```
set(PLF_COLONY_VERSION <INSERT>)
cpmaddpackage(URI "gh:mattreecebentley/plf_colony#${PLF_COLONY_VERSION}" DOWNLOAD_ONLY ON)

if(plf_colony_ADDED)
  add_library(plf_colony INTERFACE)
  target_sources(plf_colony PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${plf_colony_SOURCE_DIR} FILES ${plf_colony_SOURCE_DIR}/plf_colony.h)
  add_library(plf::colony ALIAS plf_colony)
endif()
```
 - Even as a C++20 module:
```
set(PLF_COLONY_VERSION <INSERT>)
cpmaddpackage(URI "gh:mattreecebentley/plf_colony#${PLF_COLONY_VERSION}" DOWNLOAD_ONLY ON)

if(plf_colony_ADDED)
  add_library(plf_colony)
  target_sources(plf_colony PUBLIC FILE_SET CXX_MODULES 
      BASE_DIRS ${plf_colony_SOURCE_DIR} FILES ${plf_colony_SOURCE_DIR}/plf_colony.cpp)
  add_library(plf::colony ALIAS plf_colony)
endif()
```

plf::colony is C++98/03/11/14/17/20/23-compatible.


![Infographic describing the container](https://i.imgur.com/tnRmFBv.png)
