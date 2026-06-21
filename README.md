# Strain Gauge Overview
Repo is for the firmware for the strain gauge board

## Minimum Rec
* Passes ADC voltage to the VCU
   *  (decoding to 8bit) 
 
## Extra Scope 
* calculates forces and passes force values to VCU
  * Could be offloaded later by copying functionality since SG MCU is very capable.
* implement ADC DMA streaming and increase the sampling/avg rate to increase effective accuracy.
* Get actual reference voltage from ADC pin to increase ADC accuracy.
 

### Scope Architecture  
- [ ] Sampling Thread
  * sample x amount of data and calculate running avg and pass to message Queue 
- [ ] CAN Thread
  * take queued data and send CAN 
    * need to wait for 3 messages worth and in the correct order 



# TODO
- [x] DTS file
- [x] Sampling function
- [x] averaging function 
- [x] Can message function


