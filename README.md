# keyMapper
A POC tool to capture/intercept key inputs to a game process

# How it works
Subscribe to both raw input events and keyboard evnets, use raw input events to identify device while using keyboard events for getting actual keystroke value.

## Known issues
The project is put on hold upon finding (significant!) windows-only problems in I/O device behaviors. 
Possible solution is to do a full mock via device driver.
You can explore yourself with a real joystick or https://github.com/martinpitt/umockdev

1. When plugging in device while pressing buttons: usb plug and unplug events (WM_DEVICECHANGE) can be later/earlier than keystroke events coming in
2. Latency concerns: there's no gurantee that both raw_input queues and keyboard events come together on time within a reasonable latency, spikes for few ms is big in fast games.
