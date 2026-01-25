# TetraDetector

Repository for the STM32 Tetra Detector software.  
This uses a custom AD8313-based log measurement board with a SAW filter from 380 MHz to 400 MHz.  

The hardware diagrams and schematics are not currently being made public at the time of writing, but this may change in the future.

---

## Disclaimers

This project is a non-commercial, educational hardware experiment.

The devices produced are receive-only and are physically incapable of radio transmission. They contain no transmitting components, no oscillators capable of emission, and no firmware or hardware paths that would permit transmission. No transmission has occurred at any stage of development and none is possible by design.

The devices do not decode, demodulate, decrypt, interpret, or store radio communications. They operate solely as wideband RF signal-strength detectors, producing an analogue voltage proportional to received RF power within a filtered frequency range. No audio, data, identifiers, or protocol-level information can be extracted. No signals are recorded, logged, or retained.

The devices do not target specific users, services, or messages and are unable to distinguish between signal sources. They provide only instantaneous received signal magnitude and nothing more.

A small number of units were produced exclusively for development and testing purposes and were shared privately with individuals assisting in evaluation. No units were offered commercially or were further produced.

This project was undertaken for educational and experimental purposes only, to explore RF front-end design, logarithmic detectors, and embedded signal processing. It is not intended to interfere with, monitor, or gain advantage over emergency services or any other radio users.

---

## Liability

The information, schematics, and descriptions provided in this repository are for educational and informational purposes only. The author accepts no responsibility or liability for any consequences, damages, or legal issues that may arise from anyone building, modifying, or using this information or the devices described. Any use of this information is entirely at the reader’s own risk.

This disclaimer does not override applicable laws; readers remain responsible for ensuring compliance with UK regulations (including the Wireless Telegraphy Act 2006 and other relevant legislation) when experimenting with radio equipment.

For those interested in similar projects, consider applying for a [RSGB UK Amateur Radio licence](https://rsgb.org/main/join-us/) and consulting a local radio group to understand what experiments can be done safely and legally.
