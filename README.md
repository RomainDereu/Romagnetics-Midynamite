# Midynamite  
*The Open Source MIDI Multi-Effects Pedal*

Midynamite is a compact, battery-powered MIDI multi-effects pedal with three physical MIDI DIN 5-pin connectors and USB MIDI device support.  
It can function as a standalone pedal or as a USB MIDI interface for a computer.

All effects can be toggled independently and configured via the onboard screen and encoder.

---

## 🎛 Features Overview

---

### 🎵 MIDI Tempo

Send a MIDI clock signal to:

- MIDI OUT 1  
- MIDI OUT 2  
- Both  
- USB MIDI (optional via Settings)

**Adjustable parameters:**

- **Tempo** (BPM)  
- **Output destinations** (including USB)

---

### 🎚 MIDI Modify

Modify incoming MIDI messages in two main ways:

#### 🌀 Channel Mode

- **MIDI Change**  
  Redirect all incoming MIDI messages to a selected MIDI channel.  
  Useful when your controller doesn't support quick channel switching.

- **MIDI Split**  
  Split the keyboard into two zones:  
  - Notes below a split point go to one MIDI channel.  
  - Notes above go to another.  
  Ideal for controlling two synths with one keyboard.

> The split point and both output channels are fully configurable.

---

#### 🎛 Velocity Mode

- **Velocity Change**  
  Add or subtract velocity from the incoming signal.  
  *(Range: -50 to +50)*  
  Preserves dynamic expression.

- **Fixed Velocity**  
  All incoming notes are set to a fixed velocity.  
  Useful for consistent volume or triggering.

---

### 🎼 MIDI Transpose

Apply real-time note transformations to incoming MIDI notes.

#### 📉 Pitch Shift

- Shift all incoming notes up/down by a specified number of semitones.  
- Optionally **mute the original note**.

#### 🎶 Scale Mode (Harmonizer)

- Select a **scale**, **mode**, and **interval**.  
- The pedal generates a harmonized note based on the input.

> Optionally mute the original note to only hear the harmony.

---

## ⚙️ Settings

Accessible in the **Settings** menu:

- **Ch Modify**  
  Toggle between *Change* and *Split* modes in MIDI Modify.

- **Velocity**  
  Select either *Changed* or *Fixed* mode for velocity handling.

- **Send To**  
  Route MIDI Modify and Transpose output to:
  - Out 1  
  - Out 2  
  - Both  
  - **Split**

> In **Split mode**:  
> - If Channel Mode is *Split*, low notes go to Out 1, high notes to Out 2.  
> - If Transpose is enabled, the **original note** goes to Out 1, the **transposed note** to Out 2.

- **Type**  
  Select between *Pitch Shift* and *Scale (Harmonizer)* for MIDI Transpose.

- **Start Menu**  
  Choose default screen on startup:  
  *Tempo*, *MIDI Modify*, *Transpose*, or *Settings*.

- **USB MIDI**  
  Configure USB MIDI behavior:  
  - *Send to USB*  
  - *No USB*

- **Contrast**  
  Adjust screen contrast in 10 levels (from 10% to 100%).

---

## 🆘 Panic Button

To immediately stop all MIDI output:

**Press Select + Value simultaneously**

This will send:

- **All Notes Off**
- **Note Off (for all channels)**

---

