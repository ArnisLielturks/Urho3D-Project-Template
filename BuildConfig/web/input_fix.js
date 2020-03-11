// We must override these Emscripten functions to fix the input when toggling fulscreen mode
__registerFocusEventCallback = function() {
    if (!JSEvents.focusEvent) JSEvents.focusEvent = _malloc(256);
    console.log('Overrided __registerFocusEventCallback');
}
__registerFullscreenChangeEventCallback = function() {
    if (!JSEvents.fullscreenChangeEvent) JSEvents.fullscreenChangeEvent = _malloc(280);
    console.log('Overrided __registerFullscreenChangeEventCallback');
}