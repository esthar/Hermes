Hermes
======

This program is the firmware for the Hermes Computer, one of the flight computers on board the payload of the Amherst College Electronics Club's high altitude balloon project, codename Daedalus. Hermes is connected to the other flight computer, Athena, through the I2C bus as a slave, and to a serial radio transceiver module. The main purpose of the program is to receive data from Athena and relay it to the radio, transmit its own status to Athena through the I2C and initiate emergency cut-down if command is received on the radio or if a given set of requirements are met.
