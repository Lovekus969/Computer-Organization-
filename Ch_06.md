# Chapter 6: Internal Memory

This repository contains summarized notes and explanations for **Chapter 6: Internal Memory** from computer organization and architecture. The chapter covers different types of memory, organization, error correction, and newer memory technologies.

---

## **Table of Contents**
1. [Semiconductor Main Memory](#61-semiconductor-main-memory)  
2. [Error Correction](#62-error-correction)  
3. [DDR DRAM](#63-ddr-dram)  
4. [eDRAM](#64-edram)  
5. [Flash Memory](#65-flash-memory)  
6. [Newer Nonvolatile Solid-State Memory Technologies](#66-newer-nonvolatile-solid-state-memory-technologies)  
7. [Key Terms and Review Questions](#67-key-terms-and-review-questions)

---

## **6.1 Semiconductor Main Memory**

### Organization
- Memory is organized into **cells** storing bits.
- Each cell has a **unique address**.
- Accessed via **read** and **write** operations.

### DRAM vs SRAM
- **DRAM (Dynamic RAM)**:  
  - Stores data as charge in capacitors.  
  - Needs **refreshing**.  
  - High density, slower, cheaper.  
- **SRAM (Static RAM)**:  
  - Uses flip-flops.  
  - **No refreshing needed**.  
  - Faster, lower density, more expensive.

### Types of ROM
- **ROM**: Non-volatile, permanent storage.  
  - **PROM** – Programmable once.  
  - **EPROM** – Erasable via UV light.  
  - **EEPROM** – Electrically erasable, rewritable.

### Chip Logic & Packaging
- Memory chips include **address decoding**, **data I/O**, **control signals**.  
- Packaged as **DIP, SIMM, DIMM**, etc.

### Module Organization
- Combines multiple chips into **modules** (e.g., 4 × 4-bit → 16-bit).  

### Interleaved Memory
- Memory split into **banks**.  
- Consecutive addresses stored in different banks → **parallel access** → improved performance.

---

## **6.2 Error Correction**
- Detect and correct memory errors.  
- Techniques: **Parity Bit**, **Hamming Code**, **ECC (Error-Correcting Code)**.

---

## **6.3 DDR DRAM**
- **SDRAM (Synchronous DRAM)**: Synchronized with system clock.  
- **DDR SDRAM (Double Data Rate)**: Transfers on **rising and falling clock edges**.  
- Variants: DDR2, DDR3, DDR4, DDR5 (higher speed, lower voltage).

---

## **6.4 eDRAM**
- Embedded DRAM inside processor chip → faster than external DRAM.  
- **Cache examples**:  
  - IBM z13 eDRAM cache  
  - Intel Core System Cache (L1, L2, L3)

---

## **6.5 Flash Memory**
- Non-volatile, electrically erasable.  
- **Types**:  
  - **NOR Flash**: Fast random access, code storage.  
  - **NAND Flash**: High density, used in SSDs and USB drives.

---

## **6.6 Newer Nonvolatile Solid-State Memory Technologies**
- **STT-RAM** – Spin-Transfer Torque RAM (magnetic states).  
- **PCRAM** – Phase-Change RAM (amorphous/crystalline states).  
- **ReRAM** – Resistive RAM (resistance-based storage).

---

## **6.7 Key Terms and Review Questions**
- **Key Terms**: DRAM, SRAM, EEPROM, DDR, eDRAM, STT-RAM, PCRAM, ReRAM.  
- **Practice Questions**:  
  - Memory organization and addressing.  
  - Interleaving performance.  
  - Error detection and correction codes.  
  - DDR memory timings and speeds.

---

**Author:** Kush  
**Subject:** Computer Organization and Architecture  
**Chapter:** 6 – Internal Memory
