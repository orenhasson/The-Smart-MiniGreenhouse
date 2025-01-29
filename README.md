# The-Smart-MiniGreenhouse

We aimed to create a miniature greenhouse that could serve as a model for a larger one, designed to independently optimize internal conditions for specific plants (optimal conditions inserted by user). Our focus was on soil humidity, temperature, and light.

## How the system works?
The system integrates a soil humidity sensor, a temperature sensor, and WiFi to optimize the greenhouse's operation.  

- 💧 Soil Humidity Sensor:  
  The sensor is calibrated to 100% for a fully saturated soil benchmark. For a generic plant, 35% humidity is set as optimal. The sensor checks the soil every 20 seconds. If the humidity drops below 35%, irrigation activates for 20 seconds. Afterward, the sensor checks again and repeats the irrigation cycle as needed until optimal humidity is restored.  
- 🌡️ Temperature Sensor:  
  Mounted on the greenhouse roof, the temperature sensor triggers a fan when the temperature reaches 30°C or higher.  
- ⛅ Lighting Control:  
  The system connects to WiFi to access real-time data and manage lighting. While primarily for convenience rather than plant health, the lights turn on at 17:00 (5 PM) and off at 23:00 (11 PM) daily.  

## How it's made?
We used a Large Ikea plastic box to portarait the grenhouse. we drilled 3 square holes in the size of 3 fans that are activated according to temperature, as shown in the following image: 

<img src="Assets/IMG_3485.HEIC" alt="FANS" width="500">

In addition there was a hole drilled for the temperature and soil humidity sensors:



The wiring was made according to the following sketch:

<img src="Assets/Wiring_Sketch.jpeg" alt="Sketch" width="500">




## Suggestions For Recreating This Project
A few insights that we recomended to consider doing, and we would have done if we were to continue working on this project.
- Water Irrigation Control: We suggest to change the irrigation system in a way that the controller activates the irrigation as long as humidity is under 35% and to deactivate it once humidity is 35% or more (and not for 20 seconds each time). The reason we didnt do it that way was to prevent a case where if the soil humidity sensor breakes, the irrigation wont operate for long hours. In order to also achieve that goal, we suggest to limit irrigation for 1 hour or less.
- Backup for WiFi and MQTT:


## Contents

- 📁 [Code](/Code/) This folder contains the scripts for operating the system.
- 📁 [Assets](/Assets/) This folder contains images that depict the mechanical components of the system.


 


