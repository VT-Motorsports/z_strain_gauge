# Strain Gauge Overview
Repo is for the firmware for the strain gauge board

## Minimum Rec
* Passes ADC voltage to the VCU
   *  (decoding to 8bit) 
 
## Extra Scope 
* calculates forces and passes force values to VCU
  * Could be offloaded later by copying functionality since SG MCU is very capable.
 


# TODO
* DTS file
* wrapper/interface for ADC's (functional) to get voltages
* logging for debuging
* Decode to 8bit 
* Send to VCU 
