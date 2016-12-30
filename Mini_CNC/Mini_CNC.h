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
 
#ifndef CNCPlotter_h
#define CNCPlotter_h

/* ---- Utility Constants ----------------------------------------------------*/
#undef PDEBUG /* undef it, just in case */
#ifdef DEBUG
#  define PDEBUG(str) Serial.println(str)
#else
#  define PDEBUG(str) /* not debugging: nothing */
#endif

// Make sure if serial is empty we don't loop(){ nil }
#define LINE_READ_DELAY (100)

// Positioning Modes
#define RELATIVE (0)
#define ABSOLUTE (1)

// Code Positions
#define NOT_FOUND (-1)

/* ---- Structs & Globals ----------------------------------------------------*/
float px = X_MIN;
float py = Y_MIN;
float pz = Z_MAX;
float fr = MAX_FEEDRATE;

AF_Stepper xStepper(STEPS_PER_REVOLUTION_X, STEPPER_PIN_X);
AF_Stepper yStepper(STEPS_PER_REVOLUTION_Y, STEPPER_PIN_Y);
Servo penServo;

char line[SZ_SERIAL_BUFFER];
int i = 0;

int commentLevel = 0; // think semaphore
int positioningMode = ABSOLUTE;
bool enabled = START_ENABLED;
bool paused = false;

#endif
