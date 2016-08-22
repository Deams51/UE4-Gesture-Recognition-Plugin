# UE4-VR-Gesture-Recognition-Plugin

A Gesture recognition plugin for Unreal Engine 4 allowing to record, detect and follow the progression of gestures from motion controllers in real-time with a high precision. 

![alt tag](https://cloud.githubusercontent.com/assets/6062062/17849267/468ecbe8-6858-11e6-96b9-3bc9c74185ac.gif)


As a developer/designer, you only need to record one template by gestures. This can be done live in the game. 
Once the template has been recorded, the listener will be able to detect when a gesture is started and follow its progression in real time.
You can freely add events based on the progression such as triggering a spell once the gesture is 95% done, or tick a spell for each 10% of gesture completed.

The plugin is currently under development but the following core features are already functional:
- Recording a gesture from BP or C++
- Detecting when a gesture has been started in realtime from BP or C++
- Detecting when a gesture has been completed in realtime from BP or C++
- Accessing the current gesture speed and progress percentage from BP or C++

## Todo
- Tutorial & Documentation

## Contact
Post issues to this github or [the unreal thread] (https://forums.unrealengine.com/showthread.php?108316-Custom-Gesture-Recognition).




## Credits
This plugin would not have been possible without the previous work published by B. Caramiaux et al. in: 

B. Caramiaux, N. Montecchio, A. Tanaka, F. Bevilacqua. Adaptive Gesture Recognition with Variation Estimation for Interactive Systems. ACM Transactions on Interactive Intelligent Systems (TiiS), 4(4), 18-51. December 2014 
