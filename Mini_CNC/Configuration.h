/**
 * CNC Plotter Firmware / Minimal GCODE Interpreter
 * Ian McLinden 2016
 *
 * Largely based on Dan Royer's GcodeCNCDemo (https://github.com/MarginallyClever/GcodeCNCDemo)
 * Also based on MakerBlock's TinyCNC (https://github.com/MakerBlock/TinyCNC-Sketches)
 * 
 * Image to Vector with Inkscape
 * OR with Dan Royer's Makelangelo (https://github.com/MarginallyClever/Makelangelo-software)
 * OR with Evil mad's Stipplegen (https://github.com/evil-mad/stipplegen)
 * 
 * Create GCODE with Marty McGuire's Unicorn Plugin for Inkscape (https://github.com/martymcguire/inkscape-unicorn)
 *
 * Pretty: Send GCODE with PrintRun (http://www.pronterface.com/)
 * Lightweight: Send GCODE with gcodesender.py (https://github.com/bborncr/gcodesender.py)
 *
 * Required Libraries:
 * Adafruit AccelStepper (https://github.com/adafruit/AccelStepper)
 * Adafruit AFMotor (https://github.com/adafruit/Adafruit-Motor-Shield-library)
 */

#ifndef Configuration_h
#define Configuration_h

#define VERSION (1.1)

// Define to enable debug printing
// #define DEBUG

// Max command history & command length
#define SZ_SERIAL_BUFFER (256)

// 9600 Works fine
#define BAUDRATE (9600)


// ---- Drawing & Bed Settings -------------------------------------------------

// Bed Size Limits (mm): ~40mm - 50mm for most stepper motors,
// Measure max travel after calibrating axes
#define X_MIN (0)
#define X_MAX (35)
#define Y_MIN (0)
#define Y_MAX (35)
#define Z_MIN (0)
#define Z_MAX (1)

// Delay after every line, can also be written into GCODE with G4: Dwell,
// Or left as 0
#define POST_MOVE_DELAY (0)

// Should motors be enabled at startup, default true
#define START_ENABLED (true)


// ---- Stepper Motors ---------------------------------------------------------

// From AFMotor: SINGLE | DOUBLE | INTERLEAVE | MICROSTEP
#define STEPSTYLE (INTERLEAVE)

// Step multiplier, 1 should be fine with INTERLEAVE or MICROSTEP
#define STEP_INCREMENT (1)

// Delay after each step
#define POST_STEP_DELAY (0)

// Which Stepper interface on the shield,
// 1 = M1 & M1, 2 = M3 & M4
#define STEPPER_PIN_X (2)
#define STEPPER_PIN_Y (1)

// Steps per revolution: motor-dependant, see spec
#define STEPS_PER_REVOLUTION_X (20)
#define STEPS_PER_REVOLUTION_Y (24)

// Steps per 1 MM, calibrate using Triffid Hunter's Calibration Guide:
// http://reprap.org/wiki/Triffid_Hunter's_Calibration_Guide
#define STEPS_PER_MM_X (13.4328358)
#define STEPS_PER_MM_Y (8.94187777)

// Feedrates
#define MAX_FEEDRATE (800)
#define MIN_FEEDRATE (50)


// ---- Servo ------------------------------------------------------------------

// Servo on PWM Pin
#define PEN_SERVO_PIN (9)

// Pen Positions for raised lowered
#define PEN_Z_UP_ANGLE (90)
#define PEN_Z_DOWN_ANGLE  (0)

// Hold while raising or lowering Pen
// Depends on servo and load
#define PEN_MOVE_DELAY (175)


// ---- Interface --------------------------------------------------------------

// Press to Pause / Resume - Place between GND & PIN with 10k Ohm Resistor
#define PAUSE_BUTTON_PIN (19)


#endif
