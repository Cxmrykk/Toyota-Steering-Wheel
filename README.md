### Development Progress

#### Models:

- Spline adapter: **Complete**
- Clockspring: **Complete**

#### PCB:

- Steering wheel transceiver: **Complete**
- Car transceiver: **Complete**

#### Firmware:

- Steering wheel transceiver: **_In Progress_**
- Clockspring transceiver: **_In Progress_**

### Material List

**Crimps**

- FDD1.25-250 (Horn Tab)
- RV1.25-4 (Ground)
- MDD1.25-187 and FDD1.25-187 (Power Connection)

**Screws**

- M5x12mm Flat Head Nut & Bolt

**Splicing**

- 2.5 - 3mm Diameter Heat Shrink
- Solder, and Soldering Iron
- Wire Strippers and/or Wire Cutters

**Connectors**

- 24 pin ATX cable
- 16 pin IDC cable

**Airbag Connector**

- Typically the airbag inflator on the new steering wheel will use 1 or 2 connectors based on the 3 variations of the **AK2 Squib Connector**.
  - In my case, the Toyota EV steering wheel airbag inflator used a single **Code 2 (Blue/Green) AK2 Squib Connector**.
  - You can find each individual connector here: [Link to Molex PDF](https://www.content.molex.com/dxdam/literature/987650-4993.pdf)
- You can source these connectors either by:
  - Buying the same clockspring used with the new steering wheel (Toyota)
  - Buying a third party clockspring from older cars that happen to use the same connector (Kia, Hyundai, GM in some cases).

**Microcontrollers**

- **Xiao ESP32-C3** - Mounted inside the steering wheel
- **Xiao ESP32-S3** - Mounted in the car, below the clockspring

**Custom PCBs**

- Ordered on JLCPCB
- Please take a look in the [hardware](/hardware) directory:
  - Steering Wheel Transciever
  - Car Transciever

**Spline Adapter**

- Required to fit the steering wheel on older cars (tested working with 2003 Toyota Corolla).
- Ordered on IN3DTEC (Maraging Steel, Bead blasted)

**Clockspring**

- Provides wiring for the airbag and repurposed horn cable (12V Power)
- Printed locally using Bambu P1S (White ABS, 0.2mm nozzle)
