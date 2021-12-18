# FRDM K64F Logic Analyzer

A [SUMP](URL "https://www.sump.org/projects/analyzer/protocol/") logic analyzer based on an  FRDM-K64F board.
This project was motivated by a blog post on [MCU on Eclipse](URL "https://mcuoneclipse.com/2014/06/19/updated-freedom-board-logic-analyzer-with-dma/").

## Features
* 5MHz sampling rate
* 4 channels
* 32k samples buffer

## Operation description
This project makes use of the highly configurable peripherals on the MK64FN1M0VLL12 microcontroller.
It uses DMA to collect the samples directly from the GPIO data register. The periodic interrupt timer (PIT) controls the sampling frequency and triggers the DMA transfers at regular intervals.

The reference manual for the chip has some interesting tables about the DMA's performance. According to the RM, when operating at 120MHz system clock, the eDMA should be able to service more than 10 million requests. The current firmware however doesn't seem to work at 10MHz.

## Client
This project has only been tested with Sigrok and Pulseview. There may be incompatibilities with other clients.
You can visit the [Sigrok Website](URL "https://www.sigrok.org/") to download the client.
