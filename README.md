# Proto-Synth V1 🎛️

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)

> **Sintetizador DIY educativo basado en ESP32 - Primer kit de GCLab Chile**

El Proto-Synth es un sintetizador educativo desarrollado por GCLab Chile que permite aprender electrónica, programación y síntesis de audio construyendo un instrumento funcional desde cero.

## 🚀 Características

### Hardware
- **Microcontrolador**: ESP32 con DAC integrado
- **Controles**: 4 potenciómetros + 4 botones + 4 LEDs
- **Audio**: Salida mono vía DAC interno (Pin 25)

### Software
- **Secuenciador**: 16 patrones con longitud configurable (4/8/16 pasos)
- **Escalas musicales**: 6 escalas (Mayor, Menor, Armónica, Árabe, Lidia, Frigia)
- **Síntesis**: Ondas diente de sierra con filtro paso-bajo resonante
- **Tempo**: 40-300 BPM
- **Envelope**: Control de decay/release

## 🎛️ Controles

### Potenciómetros
- **POT 1 (Pin 13)**: Frecuencia de corte del filtro paso-bajo
- **POT 2 (Pin 14)**: Resonancia del filtro
- **POT 3 (Pin 12)**: Tempo del secuenciador (40-300 BPM)
- **POT 4 (Pin 27)**: Control de decay/release

### Botones
- **BTN 1 (Pin 32)**: Cambiar escala musical
- **BTN 2 (Pin 33)**: Cambiar patrón de secuencia
- **BTN 3 (Pin 26)**: Play/Stop del secuenciador
- **BTN 4 (Pin 35)**: Cambiar longitud de secuencia

### LEDs (Pines 2, 5, 22, 23)
Indican el paso actual del secuenciador

## 📦 Contenido del Repositorio

```
proto-synth-v1/
├── README.md
├── proto_synth_v1.ino      # Código principal
├── diagrama_circuito.*     # Diagrama del circuito
├── pcb_files/              # Archivos PCB
└── gerbers.zip             # Archivos Gerber para fabricación
```

## 🛠️ Instalación

### Requisitos
- Arduino IDE
- Placa ESP32 instalada en Arduino IDE
- Componentes según esquemático

### Configuración
1. Abrir `proto_synth_v1.ino` en Arduino IDE
2. Seleccionar placa: `ESP32 Dev Module`
3. Conectar ESP32 y cargar el código

## ⚠️ Problemas Conocidos V1

Esta primera versión presenta limitaciones que serán corregidas en versiones futuras:

### Problemas de Hardware
- **Botones sin resistencias pull-up externas**: Usa pull-ups internas del ESP32
- **Potenciómetros sin filtrado**: Faltan diodos para filtrar ruido
- **Salida mono únicamente**: Solo utiliza un DAC, desperdicia capacidad estéreo
- **Pines limitados**: Pocas opciones para conectar sensores externos con cables dupont

### Mejoras Planificadas V2
- Resistencias pull-up externas para botones
- Diodos de filtrado en potenciómetros  
- Salida estéreo utilizando ambos DACs del ESP32
- Más pines disponibles para sensores externos (LDR, etc.)
- Mejores conectores dupont para expansibilidad

## 🎵 Uso Básico

1. **Ajustar sonido**: Potenciómetros modifican filtro y decay
2. **Cambiar escala**: BTN1 cicla entre escalas musicales
3. **Seleccionar patrón**: BTN2 cambia entre 16 patrones
4. **Iniciar secuencia**: BTN3 inicia/detiene reproducción
5. **Ajustar longitud**: BTN4 cambia entre 4, 8 y 16 pasos

## 🎯 Patrones Incluidos

16 patrones de secuencia diferentes:
- Trance básico, Espaciado, Ascendente, Descendente
- Repetitivo, Sincopado, Complejo, Breakbeat
- Dobles, Bajo constante, Off-beat, Arpegio
- Alternado, Saltos, Grupos, Clásico

## 🌟 Historia del Proyecto

- **Diciembre 2024**: Primer taller de construcción de sintetizadores
- **30 unidades fabricadas**: Todas agotadas en 6 meses a través de talleres presenciales

## 📄 Licencia

GPL v3.0 - Proyecto open source

## 🤝 Contacto

**GCLab Chile**
- Instagram: @gc_lab_chile
- Email: gsandoval@gclabchile.com

---

*Desarrollado por GCLab Chile - El primer kit de sintetizadores DIY educativo*
