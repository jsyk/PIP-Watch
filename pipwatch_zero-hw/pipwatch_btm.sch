EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:74xgxx
LIBS:ac-dc
LIBS:brooktre
LIBS:cmos_ieee
LIBS:dc-dc
LIBS:elec-unifil
LIBS:Epcos-MKT
LIBS:ftdi
LIBS:gennum
LIBS:graphic
LIBS:hc11
LIBS:logo
LIBS:microchip1
LIBS:microchip_pic10mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic16mcu
LIBS:msp430
LIBS:nxp_armmcu
LIBS:powerint
LIBS:pspice
LIBS:references
LIBS:relays
LIBS:rfcom
LIBS:sensors
LIBS:stm8
LIBS:stm32
LIBS:supertex
LIBS:transf
LIBS:ttl_ieee
LIBS:video
LIBS:w_analog
LIBS:w_connectors
LIBS:w_device
LIBS:w_logic
LIBS:w_memory
LIBS:w_microcontrollers
LIBS:w_opto
LIBS:w_power
LIBS:w_relay
LIBS:w_rtx
LIBS:w_transistor
LIBS:w_vacuum
LIBS:stm32-extra
LIBS:open-project
LIBS:misc_pip
LIBS:misc_epd
LIBS:pipwatch_zero-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 5
Title ""
Date "14 sep 2014"
Rev "HW02"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L BTM431 U501
U 1 1 53EF6896
P 5400 4500
F 0 "U501" H 6000 4300 60  0000 C CNN
F 1 "BTM431" H 4850 4300 60  0000 C CNN
F 2 "BTM431" H 5400 4500 60  0000 C CNN
F 3 "" H 5400 4500 60  0000 C CNN
	1    5400 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 2400 4200 2400
Wire Wire Line
	4200 2400 4200 3900
Wire Wire Line
	3850 3900 4400 3900
Wire Wire Line
	4050 2900 4400 2900
Connection ~ 4200 2900
Wire Wire Line
	6400 2600 6550 2600
Wire Wire Line
	6550 2600 6550 3600
Wire Wire Line
	6550 3200 6400 3200
Wire Wire Line
	6550 3600 6400 3600
Connection ~ 6550 3200
$Comp
L GND #PWR502
U 1 1 53EF6996
P 4050 2900
F 0 "#PWR502" H 4050 2900 30  0001 C CNN
F 1 "GND" H 4050 2830 30  0001 C CNN
F 2 "" H 4050 2900 60  0000 C CNN
F 3 "" H 4050 2900 60  0000 C CNN
	1    4050 2900
	0    1    1    0   
$EndComp
$Comp
L GND #PWR503
U 1 1 53EF69AA
P 6700 3000
F 0 "#PWR503" H 6700 3000 30  0001 C CNN
F 1 "GND" H 6700 2930 30  0001 C CNN
F 2 "" H 6700 3000 60  0000 C CNN
F 3 "" H 6700 3000 60  0000 C CNN
	1    6700 3000
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4300 3800 4400 3800
Wire Wire Line
	4300 3600 4300 3800
Wire Wire Line
	3400 3700 4400 3700
Wire Wire Line
	4400 3600 4300 3600
Connection ~ 4300 3700
$Comp
L 3V3 #PWR501
U 1 1 53EF6A0E
P 3900 3650
F 0 "#PWR501" H 3900 3750 40  0001 C CNN
F 1 "3V3" H 3900 3775 40  0000 C CNN
F 2 "" H 3900 3650 60  0000 C CNN
F 3 "" H 3900 3650 60  0000 C CNN
	1    3900 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 3700 3900 3650
Wire Wire Line
	4400 2500 3550 2500
Wire Wire Line
	4400 2600 3550 2600
Wire Wire Line
	4400 2700 3550 2700
Wire Wire Line
	4400 2800 3550 2800
Wire Wire Line
	6400 2400 7000 2400
Wire Wire Line
	6400 3400 7000 3400
Wire Wire Line
	6400 3500 7000 3500
Text HLabel 3550 2500 0    60   Input ~ 0
BTM_CTS
Text HLabel 3550 2600 0    60   Input ~ 0
BTM_RXD
Text HLabel 3550 2700 0    60   Output ~ 0
BTM_RTS
Text HLabel 3550 2800 0    60   Output ~ 0
BTM_TXD
Text HLabel 7000 2400 2    60   Output ~ 0
BTM_ACTIVE
Text HLabel 7000 3400 2    60   Output ~ 0
BTM_DTR
Text HLabel 7000 3500 2    60   Input ~ 0
BTM_DSR
Wire Wire Line
	4400 4500 3550 4500
Text HLabel 3550 4500 0    60   Input ~ 0
BTM_RESET
Wire Wire Line
	6400 3300 7000 3300
Text HLabel 7000 3300 2    60   Output ~ 0
BTM_RI
$Comp
L C C501
U 1 1 53EF7E88
P 3650 3900
F 0 "C501" H 3650 4000 40  0000 L CNN
F 1 "100n" H 3656 3815 40  0000 L CNN
F 2 "SM0805" H 3688 3750 30  0000 C CNN
F 3 "" H 3650 3900 60  0000 C CNN
	1    3650 3900
	0    1    1    0   
$EndComp
Connection ~ 4200 3900
Wire Wire Line
	3400 3700 3400 3900
Wire Wire Line
	3400 3900 3450 3900
Connection ~ 3900 3700
Wire Wire Line
	6700 3000 6550 3000
Connection ~ 6550 3000
$EndSCHEMATC
