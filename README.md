# BME_System_Design

### Project Demonstration


---
### Introduction
<img width="1644" alt="스크린샷 2025-01-24 오전 12 11 48" src="https://github.com/user-attachments/assets/2d6f0f59-52f1-4e64-ac9d-0ff81b5b5245" />



Biosignal: EOG 
- EOG is a signal that measures the potential difference between the retina and the cornea.
- Frequency range: 0.1 ~ 10Hz

Biosignal: EMG
- EMG is a signal that measures the electrical activity within muscles. When nerves within the muscles are activated, their activity manifests as electrical signals.
- Frequency range: 30 ~ 200Hz

### Topic & Objective
Our topic is about "EOG and EMG – Based Image Viewer with Cursor Control".
Previously, there was a study on a keyboard based on eog. And we tried to create a virtual mouse that could be clicked by adding an emg signal. It can be work like an image viewer. We choose the picture by eog movement(4directions: up/down/left/right). it works like mouse cursor. and when we click it by contract the hand, we can see bigger picture than before like album to image. and then we click again, it becomes smaller like image to album.


### Analog Circuit Diagram
<img width="605" alt="image" src="https://github.com/user-attachments/assets/30889c2a-c5b4-4696-b455-ae0a1631437b" />

Operating Frequency: 10 ~ 232Hz(EMG), 0.6 ~ 59Hz(EOG)
- 1st HPF 
  - EOG: 0.6Hz (R=270kΩ, C=1uF)
  - EMG: 10Hz (R=330kΩ, C=47nF)
- 2nd LPF
  - EOG: 59Hz (R=270kΩ * 2, C=10nF * 2)
  - EMG: 232Hz (R=47kΩ, 1kΩ, C=100pF * 2)
 
### Digital Filter/ MSP 430  
- Sampling Frequency: 250Hz
- 3-pole Butterworth low pass filter to remove the power noise
- EOG: fc = 10Hz
- EMG: fc = 100Hz



### Flow Chart
<img width="1610" alt="flowchart" src="https://github.com/user-attachments/assets/db326e0a-1803-48f0-964f-d66028d7fc94" /><img width="1644" alt="스크린샷 2025-01-24 오전 12 11 48" src="https://github.com/user-attachments/assets/9c24d7d1-0cef-4d29-a7aa-9d3c083604aa" />






### GUI Interface
<img width="1061" alt="Interface" src="https://github.com/user-attachments/assets/dc1400e6-1c65-4b16-afdf-15e8f05541a9" />
The following is a flowchart. It moves the cursor by classifying the four directions LEFT, RIGHT, UP, and DOWN through the EOG signal. At the same time, we classify ON and OFF through the EMG signal and open the image when the EMG signal is ON. At this time, the cursor cannot be moved when the image is open. Once again, if the EMG signal is ON, the image is closed.



---
### Sensor


### Software
- LTspice(circuit simulation)
- Matlab(preprocessing)
- C++(MSP 430 embedding system)
- C#(GUI Implementation)



