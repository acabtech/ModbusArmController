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
#define NUM_SERVOS 3
const int SERVO_PINS[NUM_SERVOS] = {6, 7, 8};

// Relay Configuration
#define NUM_RELAYS 4
const int RELAY_PINS[NUM_RELAYS] = {11, 12, 13, 14};

// Application Configuration
#define SENSOR_READ_INTERVAL_MS 5000  // Read sensor every 5 seconds
#define SERVO_DEFAULT_POSITION 90     // Neutral position in degrees
#define RELAY_SAFE_STATE LOW          // Off state for relays

// Modbus Register and Coil Addresses (0-based indexing for internal use)
#define RELAY_COIL_START_ADDR 0       // Relay coils starting address
#define SERVO_REG_START_ADDR 0        // Holding registers 0-4 for servo positions (0-180 degrees)
#define TEMP_REG_ADDR 10              // Holding register 10: Temperature (int16_t, x10 for integer; signed, two's complement)
#define HUMIDITY_REG_ADDR 11          // Holding register 11: Humidity (x10)
#define PRESSURE_REG_ADDR 12          // Holding register 12: Pressure (hPa, integer)
#define ALTITUDE_REG_ADDR 13          // Holding register 13: Altitude (int16_t, m x10; signed, two's complement)

#endif // CONFIG_H