
set_property(GLOBAL APPEND PROPERTY SOURCES
  #push core
  src/push/core/PushContext.cpp
  src/push/core/PushCore.cpp
  src/push/core/PushDisplay.cpp
  src/push/core/PushFaker.cpp
  src/push/core/PushFonts.cpp
  src/push/core/PushLeds.cpp
  src/push/core/PushMidi.cpp
  src/push/core/PushPads.cpp
  src/push/core/PushPainter.cpp
  src/push/core/PushSysex.cpp
  src/push/core/Widget.cpp

  #widgets
  src/push/elements/BrowserWidget.cpp
  src/push/elements/GridWidget.cpp
  src/push/elements/PadLayoutWidget.cpp
  src/push/elements/RootWidget.cpp
  src/push/elements/UnitWidget.cpp

  #primitives
  src/push/primitives/Canvas.cpp
  src/push/primitives/Label.cpp
  src/push/primitives/Pointer.cpp
  src/push/primitives/Rectangle.cpp
  src/push/primitives/StaticLabel.cpp
  src/push/primitives/UnitUIBase.cpp

  #utility
  src/push/utility/PadNoteLayout.cpp
  src/push/utility/PushUIContext.cpp

  src/push/pushThread.cpp
)