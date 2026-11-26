#message("Sources from sampler")

set_property(GLOBAL APPEND PROPERTY SOURCES
  units/Sampler/Sampler.cpp
  units/Sampler/SamplerUnit.cpp
  units/Sampler/SamplerView.cpp
  units/Sampler/SamplerUI.cpp
  units/Sampler/SamplerPushUI.cpp
  units/Sampler/Actions/LoadAsset.cpp
  units/Sampler/Actions/SetMode.cpp
)

#get_property(SOURCES GLOBAL PROPERTY SOURCES)
#message(${SOURCES})

