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

#include <Servo.h>
#include <AFMotor.h>
#include "Configuration.h"
#include "CNCPlotter.h"

/* ---- setup() & loop() - Arduino Methods -----------------------------------*/
void setup() {
    Serial.begin(BAUDRATE);
 
    liftPen();
    setFeedrate(MAX_FEEDRATE);
    printStartup();
    printHelp();
}

void loop() {
    // Build Line char at a time from Serial
    while ( Serial.available()>0 ) {
        char c = Serial.read();

        switch(c) {
            // End of Line
            case '\n':
            case '\r': {
                if ( i > 0 ) {
                    line[ i ] = 0;
                    PDEBUG(String(F("Received : ")) + String(line));
                    processLine(line);
                    i = 0;
                }
                commentLevel = 0;
                break;
            }
            
            // Ignore the following
            case ' ':
            case '@':
            case '/': break;
            
            // If it's a commennt
            case ';':
            case '*':
            case '(': commentLevel++; break;
            case ')': commentLevel--; break;

            // Everything else should be okay
            default: {
                if (!commentLevel) {
                    if ( i >= SZ_SERIAL_BUFFER-1 ) {
                        Serial.println(F("!! ERROR - Command too long"));
                        commentLevel = 0;
                    } else {
                        line[i++] = c;
                    }
                }
                break;
            }
            
        } // END: switch
    } // END: while ( Serial.available() > 0 )
    
    // Keep from saturating void loop()
    delay(LINE_READ_DELAY);
}

/*
 * Interprets the following commands:
 *
 * G0: Rapid linear Move              G0 X25.00 Y15.00
 * G1: Linear Move                    G1 X25.00 Y15.00
 * G4: Dwell (P/ms S/s)               G4 P150 (wait 150ms)
 * G90: Set to Absolute Positioning
 * G91: Set to Relative Positioning
 * G92: Set Position                  G92 X0.00 Y0.00 Z0.00 (you are here)
 * M01: Sleep or Conditional stop
 * M17: Enable/Power all steppers
 * M18: Disable all stepper motors
 * M100: Print Help
 * M114: Get Current Position
 * M300: Play beep sound - pen        M300 S50 (pen up)
 */
void processLine(char* line) {
    int code = (int) parse_number(line, 'N', NOT_FOUND);
    switch(code) {
        case NOT_FOUND: break;
        default: PDEBUG(String(F("Line ")) + String(code)); break;
    }
    
    code = (int) parse_number(line, 'G', NOT_FOUND);
    switch(code) {
        case NOT_FOUND: break;
        
        // Rapid Linear Move
        case 0: {
            setFeedrate(MAX_FEEDRATE);
            linearMoveTo( parse_number(line, 'X',((positioningMode)?px:0)) + ((positioningMode)?0:px),
                parse_number(line, 'Y',((positioningMode)?py:0)) + ((positioningMode)?0:py));
            ok();
            return;
        }
        // Linear Move
        case 1: {
            setFeedrate(parse_number(line, 'F', fr));
            linearMoveTo( parse_number(line, 'X',((positioningMode)?px:0)) + ((positioningMode)?0:px),
                parse_number(line, 'Y',((positioningMode)?py:0)) + ((positioningMode)?0:py));
            ok();
            return;
        }
        // Dwell
        case 4: {
            int tmsec = parse_number(line, 'P', 0);
            if (tmsec <= 0) tmsec = parse_number(line, 'S', 0)*1000;
            if (tmsec > 0) {
                PDEBUG(F("Pause"));
                delay(tmsec);
                PDEBUG(F("Done"));
            }
            ok();
            return;
        }
        // Set to Absolute Positioning
        case 90: 
          positioningMode = ABSOLUTE; 
          ok(); 
          PDEBUG(F("Positioning Mode set ABSOLUTE")); 
          return;
        // Set to Relative Positioning
        case 91: 
          positioningMode = RELATIVE; 
          ok(); 
          PDEBUG(F("Positioning Mode set RELATIVE")); 
          return;
        // Set Position
        case 92: 
            setPosition( parse_number(line, 'X', 0), 
                parse_number(line, 'Y', 0), 
                parse_number(line, 'Z', 0));
            printCurrentPos();
            return;
        // Not valid 'G' Code
        default: printUnsupported("G",code); return;
    } // END: 'G' Switch

    code = (int) parse_number(line, 'M', NOT_FOUND);
    switch(code) {
        case NOT_FOUND: break;
        // Sleep or Conditional stop
        case 1: {
            // TODO: 12/15/16 - Need to solder a button to the Motor Shield
//          attachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON_PIN), pauseButtonPressed, CHANGE); 
//          while (paused) {
//            delay(LINE_READ_DELAY);
//          }
          ok();
          return;
        }
        // Enable/Power all stepper motors
        case 17: {
            enabled = true;
            ok();
            PDEBUG(F("Motors Enabled"));
            return;
        }
        //  Disable all stepper motors
        case 18: {
            xStepper.release();
            yStepper.release();
            enabled = false;
            ok();
            PDEBUG(F("Motors Disabled"));
            return;
        }
        // Print Help Message
        case 100: printHelp(); ok(); return;
        // Get Current Position
        case 114: printCurrentPos(); return;
        // Set Pen Position
        case 300: {
            int pos = (int) parse_number(line, 'S', NOT_FOUND);
            switch(pos) {
                case 30: lowerPen(); ok(); return;
                case 50: liftPen(); ok(); return;
                case NOT_FOUND:
                    printUnsupported("M300 S", 0);
                    return;
                default:
                    printUnsupported("M300 S", pos);
                    return;
            }
            return;
        }
        // Not valid 'M' Code
        default: printUnsupported("M",code); return;
    } // END: 'M' Switch

    // For Convenience, U & D for pen
    code = (int)parse_number(line, 'U', NOT_FOUND);
    switch(code) {
        case NOT_FOUND: break;
        default: liftPen(); ok(); return;
    }

    code = (int)parse_number(line, 'D', NOT_FOUND);
    switch(code) {
        case NOT_FOUND: break;
        default: lowerPen(); ok(); return;
    }

    printUnsupportedLine(line);
}

/* ---- Command Handlers -----------------------------------------------------*/

/**
 * Look for character /code/ in the buffer and read the float that immediately follows it.
 * Based on Dan Royer's GcodeCNCDemo
 */
float parse_number(char* buffer, char code, float val) {
    char upper = code;
    char lower = code;
    if ( code >= 'a' && code <= 'z' ) {
        upper = code-'a'+'A';
    } else {
        lower = code-'A'+'a';
    }
    
    char *ptr=buffer;
    while(*ptr != 0) {
        if(*ptr==upper || *ptr==lower) {
            return atof(ptr+1);
        }
        ptr++;
    }
    return val;
}

/**
 * Use Bresenham's algorithm to move linearly to (x,y)
 * Based on Dan Royer's GcodeCNCDemo
 */
void linearMoveTo(float x, float y) {
    if (!enabled) return;
    
    PDEBUG(String(F("Try Move To: ")) +String(x) + String(F(", ")) + String(y));
        
    long i;
    long over = 0;
    
    // Bound instructions to printbed
    x = min(max(x, X_MIN), X_MAX);
    y = min(max(y, Y_MIN), Y_MAX);
    
    long dx = (int)(abs(x-px) * STEPS_PER_MM_X);
    long dy = (int)(abs(y-py) * STEPS_PER_MM_Y);
    int dir_x = (px < x) ? STEP_INCREMENT : -STEP_INCREMENT;
    int dir_y = (py < y) ? STEP_INCREMENT : -STEP_INCREMENT;

    PDEBUG(String(F("Moving: ")) +String(dx*dir_x) + String(F(", ")) + String(dy*dir_y));
    
    if (dx > dy) {
        for (i=0; i<dx; ++i) {
            xStepper.step(1,dir_x,STEPSTYLE);
            over+=dy;
            if (over>=dx) {
                over-=dx;
                yStepper.step(1,dir_y,STEPSTYLE);
            }
            delay(POST_STEP_DELAY);
        }
    } else {
        for (i=0; i<dy; ++i) {
            yStepper.step(1,dir_y,STEPSTYLE);
            over+=dx;
            if (over>=dy) {
                over-=dy;
                xStepper.step(1,dir_x,STEPSTYLE);
            }
            delay(POST_STEP_DELAY);
        }
    }
    
    //  Delay before any next lines are submitted
    delay(POST_MOVE_DELAY);
    //  Update the positions
    px += (dir_x * dx/STEPS_PER_MM_X);
    py += (dir_y * dy/STEPS_PER_MM_Y);
}

void setPosition(float x, float y, float z) {
    px = min(max(x, X_MIN), X_MAX);
    py = min(max(y, Y_MIN), Y_MAX);
    pz = min(max(y, Z_MIN), Z_MAX);
}

void setFeedrate(float f) {
    if (f > MAX_FEEDRATE || f < MIN_FEEDRATE) {
        printUnsupported("F", f);
    } 
    fr = min(max(f, MIN_FEEDRATE), MAX_FEEDRATE);
    xStepper.setSpeed(fr);
    yStepper.setSpeed(fr);
    PDEBUG(String(F("Feedrate set: ")) + String(fr));   
}


/* ---- Pen Movement ---------------------------------------------------------*/
void liftPen() {
    if (!enabled) return;

    penServo.attach(PEN_SERVO_PIN);
    penServo.write(PEN_Z_UP_ANGLE);
    delay(PEN_MOVE_DELAY);
    penServo.detach();
    pz=Z_MAX;
    PDEBUG(F("Raised Pen"));
}

void lowerPen() {
    if (!enabled) return;
   
    penServo.attach(PEN_SERVO_PIN);
    penServo.write(PEN_Z_DOWN_ANGLE);
    delay(PEN_MOVE_DELAY);
    penServo.detach();
    pz=Z_MIN;
    PDEBUG(F("Lowered Pen"));
}

/* ---- Button ISR -----------------------------------------------------------*/
void pauseButtonPressed() {
    detachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON_PIN));
    paused = !paused;
}

/* ---- Print Helpers --------------------------------------------------------*/
void printStartup() {
    Serial.print(F("CNC Plotter: "));
    Serial.println(VERSION);
    
    Serial.print(F("Connected at baudrate "));
    Serial.println(BAUDRATE);
    
    Serial.print(F("X-Axis: "));
    Serial.print(X_MIN);
    Serial.print(F(" - "));
    Serial.print(X_MAX);
    Serial.println(F(" mm."));
    Serial.print(F("Y-Axis: "));
    Serial.print(Y_MIN);
    Serial.print(F(" - "));
    Serial.print(Y_MAX);
    Serial.println(F(" mm."));
}

void printHelp() {
    Serial.println(F("Commands:"));
    Serial.println(F("G0 [X(steps)] [Y(steps)] - Rapid Linear Move"));
    Serial.println(F("G1 [X(steps)] [Y(steps)] [F(feedrate)] - Linear Move"));
    Serial.println(F("G4 P(milliseconds)/S(seconds) - Dwell"));
    Serial.println(F("G90 - Set to Absolute Positioning"));
    Serial.println(F("G91 - Set to Relative Positioning"));
    Serial.println(F("G92 [X(steps)] [Y(steps)] - Set Position"));
    Serial.println(F("M01 - Sleep or Conditional stop"));
    Serial.println(F("M17 - Enable/Power all stepper motors"));
    Serial.println(F("M18 - Disable all stepper motors"));
    Serial.println(F("M100 - Print Help Message"));
    Serial.println(F("M114 - Get Current Position"));
    Serial.println(F("M300 [S(angle)] - Set Pen Position"));
}

void printCurrentPos() {
    Serial.print(F("ok C: X:"));
    Serial.print( px );
    Serial.print(F(" Y:"));
    Serial.print( py );
    Serial.print(F(" Z:"));
    Serial.print(pz);
    Serial.print(F(" F:"));
    Serial.println(fr);
}

void printUnsupported(String command, float code) {
    Serial.print(F("ok - Unsupported Command: "));
    Serial.print(command);
    Serial.println(code);
}

void printUnsupportedLine(String line) {
    Serial.print(F("ok - Unsupported Command: "));
    Serial.println(line);
}

void ok() {
    Serial.println(F("ok"));
}

