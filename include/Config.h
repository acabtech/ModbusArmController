#ifndef CONFIG_H
#define CONFIG_H

// Modbus UART Configuration
#define MODBUS_UART_TX_PIN 0
#define MODBUS_UART_RX_PIN 1
#define MODBUS_BAUD_RATE 9600
#define MODBUS_SLAVE_ID 1
#define MODBUS_DE_PIN 2

// I2C Configuration for BME280
#define BME280_SDA_PIN 4
#define BME280_SCL_PIN 5
#define BME280_I2C_ADDRESS 0x76  // Default address, change to 0x77 if SDO pin is high

// Servo Configuration
#define NUM_SERVOS 5
const int SERVO_PINS[NUM_SERVOS] = {6, 7, 8, 9, 10};

// Relay Configuration
#define NUM_RELAYS 16
const int RELAY_PINS[NUM_RELAYS] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};

// Application Configuration
#define SENSOR_READ_INTERVAL_MS 5000  // Read sensor every 5 seconds
#define SERVO_DEFAULT_POSITION 90     // Neutral position in degrees
#define RELAY_SAFE_STATE LOW          // Off state for relays

// Modbus Register and Coil Addresses (0-based indexing for internal use)
#define RELAY_COIL_START_ADDR 0       // Coils 0-15 for relays
#define SERVO_REG_START_ADDR 0        // Holding registers 0-4 for servo positions (0-180 degrees)
#define TEMP_REG_ADDR 10              // Holding register 10: Temperature (x10 for integer)
#define HUMIDITY_REG_ADDR 11          // Holding register 11: Humidity (x10)
#define PRESSURE_REG_ADDR 12          // Holding register 12: Pressure (Pa)
#define ALTITUDE_REG_ADDR 13          // Holding register 13: Altitude (m x 100)

#endif // CONFIG_H