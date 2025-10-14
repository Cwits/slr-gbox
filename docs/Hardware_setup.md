# Hardware Setup Guide

This would be, probably, most difficult and time consuming part for someone.  
This document describes how I assembled the prototype hardware used for this project.  
It’s not a strict manual - more like notes and references from a working build.  


## Reference build  
Most of the components are easily available at market.  

### Required Tools (minimal):  
- Soldering iron  
- Multimeter  
- STLink  
- Hot glue gun  

### Bill of Materials (BOM):  
- Raspberry Pi 4 with heatsink  
- STM32F411 "Black Pill"  
- PCM5102A DAC module  
- PCM1808 ADC module  
- Generic 7-inch hdmi usb touchscreen  
- Several Prototyping PCB  
- USB3.0 to SATA converter(with external power)  
- HDD or SSD  
- Dupont female connectors(salvaged from cheap rainbow colored chinese wires), pin headers  
- 24V 2.5A power supply(got mine from e-wastes)  
- 2x XL6009 DC-DC step down converters  
- Broken usb audio interface(with working power rails and analogue part)  
- Plastic enclosure (I've used one from Arduino Starter Kit)
- Other small, but not necessarily required, things.  
- Wires  
- A lot of enthusiasm  

I'm assuming that who is reading this have basic electronics and programming knowledge, otherwise some research and practice must be done before proceeding.  

# Safety Warning & Disclaimer of Liability  
> This is an experimental DIY project involving electronics, soldering, and potentially dangerous voltages.  
> All information and code are provided “AS IS” without warranty.  
> All hardware modifications described in this repository are intended only for experienced users working on devices they own.  
> Any modification may void warranties, damage equipment, or create electrical hazards.  
> The author assumes no responsibility for loss, damage, or personal injury, or any other consequences resulting from use of this material.  
> By proceeding, you acknowledge the risks and assume full responsibility for your actions.  

# ⚠️ **WARNING: HIGH VOLTAGE AND SAFETY NOTICE** ⚠️
> This project involves working with voltages (24V DC) that can be dangerous and potentially cause fire if not handled correctly.  
> The use of a plastic enclosure for a device containing power converters requires careful planning of ventilation and insulation.  
> You are building this device at your own risk.  
> I’m not responsible for any damage to your components, property, or health. Please double-check all your connections before powering on.  

All wiring details see in [wiring_diagram](docs/pics/wiring_diagram.png)  

## Power unit assembly:  
Solder both DC-DC modules on prototyping pcb board as well as DC 2.1mm jack. Connect them together according to wiring_diagram.jpg.  
*(Todo: can add several caps for filtering.)*  
Using multimeter set up outputs to next voltages:  
- **+12V** - for audio interface and for HDD sata to usb3.0 converter  
- **+5V** - for Raspberry Pi, motherboard and display(under load this unit can become hot, so consider using any kind of heatsink).  

Make sure that you took wires at least AWG24-AWG22 for power rails(to put it simply - not the thinnest ones).  
**Warning**: double check voltage levels before switching on! Wrong voltages might burn your devices!  

![Power Unit](docs/pics/power_supply.jpg)

## Assembling “motherboard”:
Assembly STM32 together with ADC and DAC on Prototyping PCB according to the wiring diagram

To have certain modularity at prototyping stage I’ve soldered several pin headers on the one side of prototyping pcb to easily connect/disconnect audio interface, as well as short as possible wires with dupont female connectors on the other side for same reasons connecting to Raspberry Pi.  
Yes, that can affect the quality of sound, but right now I'm ok with that.  

### Uploading the firmware:  
Open STM32 ST-LINK Utility(can be downloaded from stm32 webpage).  
Load the firmware from docs/stm/ folder(called clocker.hex) to program.  
Connect STM32 Black Pill through STLink to PC.  
Do "Target -> Connect" and than "Target -> Program & Verify".  
If firmware works correctly then built-in led must blink each second.  
If any problems occurred - search in google for more detailed information and how-to  

![Motherboard Top View](docs/pics/motherboard_top.jpg)
![Motherboard Bottom View](docs/pics/motherboard_bottom.jpg)

## Preparing audio interface board:  
For the microphone inputs I used a salvaged audio-interface board (pre-ADC stage only). I removed custom DSP/ADC/DAC components and wired the preamp outputs into our ADC.  
Due to potential intellectual-property and safety issues, I cannot publish photos, step-by-step modification instructions, or exact soldering points for the original commercial board. Use only devices you own and perform hardware work at your own risk.

## Case Assembly:  
Before assembling make sure you flashed hdd with the Raspberry Pi firmware so you won’t need to disassemble it later!

Drill or cut openings for:
- XLR's and 6.3mm jacks
- Pots and switches
- DC jack
- Display
- Raspberry Pi ports.

Some components I’ve mounted on distancers(like usb audio soundcard), others on hot glue(like Raspberry Pi). 
First I've installed audio interface on plastic distancers and mounted Raspberry Pi on hot glue to make it solid.
I've put little bit of foam around the HDD because it is vibrating and can’t be mounted on hot glue, as Raspberry Pi.
Connect everything together.

![Photo](docs/pics/assembled.jpg)
![Photo](docs/pics/assembled_inside.jpg)

Unfortunately I didn't managed to squeeze all components inside the box, so some of them are sticking out of it, but that is fine for prototype, right?  

This setup is just one working example — you can freely modify to adapt wiring, enclosure, and power layout to whatever components you have.    

# Notes and Variations:
Most of the parts explained here and not strictly necessary to get the job done, you can freely modify, extend or adapt hardware to your own needs.  
Here is some ideas:  

- ADC and DAC modules doesn’t have to be exactly the same, basically every other ic’s will do the job, but may require some software and hardware modification(e.g. device tree overlay to fit bit depth if you use 16bit ic’s). Only one requirement is that it must be I2S protocol.  
- If you don’t have broken usb audio interface board than you can build without it at all, or only using DAC module.  
- If you want audio input as well(including xlr) than there are plenty of open-source builds for mic pre amps, but you may do that by yourself(e.g. ThatMicPre, Balanced Mic Preamp from Elektor/EEWeb, Ultra-Simple Microphone Preamplifier from EEWeb or naufalboys Mic-Preamp).  
- Exactly this power supply and DC-DC modules isn’t mandatory, you can use any other power supply with or without DC-DC converters while they will supply necessary voltages and amps(e.g. 5V 3A for Raspberry Pi).  
- HDD not mandatory as well, sd can can be used but consider that sd card can be slower than HDD/SSD via USB3.0  
- I guess that one of HiFiBerries hat can be used as alternative  
(here https://github.com/rsta2/circle/discussions/453 is written that Raspberry Pi 5 can handle up to 4 stereo outputs! 0_o)  
- STM32 not mandatory as well, Raspberry Pi can act as a master by providing all necessary clocks(but requires some software modifications and some clock jitter might be involved)  

Stay creative! Find your own way to build the device and feel free to share your story - it would be awesome to read!

## A Note on Noise
Please be aware that this prototype is built on a perfboard. This is great for flexibility and experimentation, but it is not optimal for high-fidelity audio. The long wires and lack of a proper ground plane make the analog circuits susceptible to noise from the Raspberry Pi and power converters.  
Consider the noise in this version a "feature" of the prototype. The main goal of this hardware release is to validate the software and the overall concept. The battle for a clean signal will be won on the next iteration — with a custom designed PCB.  

## A Note on STM32
I've used this code in another project, so it is mostly a mess. I have this on my future checklist.  
For building firmware I'm using TrueStudio.  