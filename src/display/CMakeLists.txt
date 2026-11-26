
set_property(GLOBAL APPEND PROPERTY SOURCES
  #ui
  src/display/elements/BottomPanel.cpp
  src/display/elements/Browser.cpp
  src/display/elements/GridView.cpp
  src/display/elements/RootWindow.cpp
  src/display/elements/ModEngineView.cpp
  src/display/elements/SplashScreen.cpp
  src/display/elements/StepSequencerView.cpp
  src/display/elements/Timeline.cpp
  src/display/elements/TopPanel.cpp
  src/display/elements/UnitView.cpp
  src/display/elements/EditorView.cpp
  
  src/display/popups/DialogWindows.cpp
  src/display/popups/DragViewSelector.cpp
  src/display/popups/NewUnitPopup.cpp
  src/display/popups/PopupManager.cpp
  src/display/popups/RouteManager.cpp
  src/display/popups/ScreenKeyboard.cpp
  src/display/popups/SettingsPopup.cpp
  src/display/popups/TimelinePopup.cpp
  src/display/popups/VirtualMidiKeyboard.cpp

  src/display/primitives/BaseWidget.cpp
  src/display/primitives/Button.cpp
  src/display/primitives/Checkbox.cpp
  src/display/primitives/DropDown.cpp
  src/display/primitives/ClipPreview.cpp
  src/display/primitives/FilePreview.cpp
  src/display/primitives/Label.cpp
  src/display/primitives/Popup.cpp
  src/display/primitives/Slider.cpp
  src/display/primitives/UnitUIBase.cpp

  src/display/utility/FileToCanvas.cpp
  src/display/utility/DefaultColors.cpp
  src/display/utility/DefaultStyles.cpp
  src/display/utility/DragContext.cpp
  src/display/utility/UIContext.cpp

  src/display/guiThread.cpp

)