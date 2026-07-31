# 8f_clip

**8f_clip** is a modern, high-performance audio clipping plugin developed by **Oktafonika**. Built using the JUCE framework, it features a clean vector interface, real-time waveform history, and an interactive transfer function graph designed for precision audio processing.

---

## 🎨 Themes / Interfaces
8f_clip comes with multiple built-in UI themes that can be switched instantly via the right-click context menu. Your preference is automatically saved with your project state.

| Bright Blue Theme | Dark Red Theme |
| :---: | :---: |
| ![Bright Blue](screenshots/bright0.png) | ![Dark Red](screenshots/dark0.png) |

---

## ✨ Key Features

* **Interactive Transfer Function Graph:** Visually draw and manipulate your clipping threshold and softness directly on the curve (supports drag & drop, double-click reset, and direct mouse interaction).
* **Real-Time Output Waveform:** Monitor your processed audio with a live scrolling history buffer, zoom controls, and a freeze/pause mode (double-click to pause).
* **Adjustable Oversampling:** Up to 16X oversampling options to effectively minimize unwanted aliasing artifacts during heavy clipping.
* **Flexible Shaping Controls:**
  * **Gain:** Input drive control ranging from -12 dB to +12 dB.
  * **Clip:** Threshold adjustment to dial in subtle saturation or hard digital clipping.
  * **Softness:** Smooth tanh-based saturation blending for musical, warm tone-shaping.
* **Precise Output Metering:** Accurate peak hold and peak-level metering scaled down to -inf dB.
* **Custom Context Menu:** Easily reset window sizes, toggle themes, or access plugin information via right-click.
* **Resizable GUI:** Fully vector-based, responsive interface with size-persistence.

---

## 🛠️ Controls Overview

1. **GAIN:** Adjusts the input signal level before clipping.
2. **CLIP:** Sets the threshold level for clipping activation.
3. **SOFTNESS:** Introduces smooth transitional saturation past the threshold point.
4. **OVERSAMPLING:** Selectable internal processing rate (OFF, 2X, 4X, 8X, 16X) for pristine fidelity.
5. **ZOOM Slider (Waveform view):** Fine-tunes the time-window resolution of the live waveform history.

---

## 💻 System Requirements & Formats

* **Formats:** VST3 (Cross-platform support)
* **Framework:** C++ / JUCE

---

## 📄 License & Usage

This project is source-available under a custom non-commercial-sale license. 

* **Permitted:** You are free to use, modify, and study the source code for both personal and commercial music production.
* **Restricted:** Selling this software, its compiled binaries, or repackaging it as a paid product is **strictly prohibited**.
* **Branding:** The names **"8f"** and **"Oktafonika"** are protected artistic properties.

See the [LICENSE](LICENSE.txt) file for full details.

---

## ☕ Support the Creator

If you enjoy using **8f_clip** in your productions, consider supporting future development and indie audio plugin design by purchasing music from the creator:

👉 **[Oktafonika Bandcamp](https://oktafonika.bandcamp.com/)**
